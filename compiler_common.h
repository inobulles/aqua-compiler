#if !defined(__AQUA_COMPILER__COMPILER_COMMON_H)
#define __AQUA_COMPILER__COMPILER_COMMON_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <zed.h>

// section variables and macros

static uint64_t rom_bytes;
static uint8_t* rom_data;

static uint64_t position_label_count;
static zed_token_t* position_labels;

static uint64_t data_label_count;
static zed_token_t* data_labels;

static uint64_t logic_section_words;
static uint64_t* logic_section_data;

// zed token helper functions

static inline int64_t zed_token_find(zed_token_t* list, uint64_t count, const char* name) { // search through list of tokens and return index of matching token or -1 if nothing found 
	for (uint64_t i = 0; i < count; i++) {
		if (list[i].name && strcmp(list[i].name, name) == 0) {
			return i;
		}
	}

	return -1;
}

static inline void zed_token_add_data(zed_token_t* self, uint8_t byte) {
	self->data = (uint8_t*) realloc(self->data, self->data_bytes + 1);
	self->data[self->data_bytes++] = byte;
}

static inline void zed_token_free(zed_token_t* self) {
	if (self->name) free(self->name);
	if (self->data) free(self->data);

	memset(self, 0, sizeof(*self));
}

// rom helper functions

static inline void rom_add_data(uint64_t data) {
	logic_section_data = (uint64_t*) realloc(logic_section_data, (logic_section_words + 1) * sizeof(uint64_t));
	logic_section_data[logic_section_words++] = data;
}

static uint8_t instruction_validated = 1;
static uint8_t current_operand;

static uint64_t current_instruction_position;
static zed_instruction_t current_instruction;

static inline void rom_validate_instruction(void) {
	if (!instruction_validated) {
		(void) sizeof(char[1 - 2 * (sizeof(current_instruction) > sizeof(uint64_t))]); // throw error when sizeof(zed_instruction_t) is greater than 8 bytes (this should never happen in theory)
		logic_section_data[current_instruction_position] = *((uint64_t*) &current_instruction);
	}
}

#define ZED_TYPE_OPCODE 255

static inline void rom_add(uint8_t type, uint64_t data) {
	if (type == ZED_TYPE_OPCODE) {
		rom_validate_instruction();
		instruction_validated = 0;

		current_instruction_position = logic_section_words;
		rom_add_data(0); // leave space for the instruction
		
		memset(&current_instruction, 0, sizeof(current_instruction));
		current_instruction.operation = data & 0xF;
		current_operand = 1;

	} else {
		uint8_t is_64_bit_operand = data > 0xFFFF; // does the data need more than 16 bits to be represented?
		
		// no way to make this more elegant :(
		// you unfortunately can't create lists of elements that are < 8 bits in C99

		if (current_operand == 1) {
			current_instruction.operand1_64_bit = is_64_bit_operand;
			current_instruction.operand1_type = type & 0b111;

			if (!is_64_bit_operand) current_instruction.operand1_data = data & 0xFFFF;

		} else if (current_operand == 2) {
			current_instruction.operand2_64_bit = is_64_bit_operand;
			current_instruction.operand2_type = type & 0b111;

			if (!is_64_bit_operand) current_instruction.operand2_data = data & 0xFFFF;

		} else if (current_operand == 3) {
			current_instruction.operand3_64_bit = is_64_bit_operand;
			current_instruction.operand3_type = type & 0b111;

			if (!is_64_bit_operand) current_instruction.operand3_data = data & 0xFFFF;
		}

		current_operand++;

		if (is_64_bit_operand) {
			rom_add_data(data);
		}
	}
}

static inline void rom_create(void) {
	rom_bytes = 0;
	rom_data = (uint8_t*) 0;

	position_label_count = 1; // first position is the main position, so allocate that
	position_labels = (zed_token_t*) malloc(position_label_count * sizeof(zed_token_t));
	
	data_label_count = 0;
	data_labels = (zed_token_t*) 0;

	logic_section_words = 0;
	logic_section_data = (uint64_t*) 0;
}

static inline void rom_free(void) {
	// no need to free anything really, user isn't expected to call rom_create multiple times during the process' lifespan
}

static inline void rom_build(void) {
	// meta section

	zed_meta_section_t meta_section = {
		.magic = ZED_MAGIC,
		.version = ZED_VERSION,

		.data_section_element_count = data_label_count,
		.position_section_element_count = position_label_count,
	};

	rom_bytes = sizeof(meta_section);
	rom_data = (uint8_t*) malloc(rom_bytes);

	// data section
	
	zed_data_section_element_t* data_section_elements = (zed_data_section_element_t*) malloc(data_label_count * sizeof(zed_data_section_element_t));

	for (uint64_t i = 0; i < data_label_count; i++) { // write each data element and record a pointer to it
		data_section_elements[i].bytes = data_labels[i].data_bytes;
		data_section_elements[i].data_offset = rom_bytes;

		rom_data = (uint8_t*) realloc(rom_data, rom_bytes + data_labels[i].data_bytes);
		memcpy(rom_data + rom_bytes, data_labels[i].data, data_labels[i].data_bytes);

		rom_bytes += data_labels[i].data_bytes;
	}

	rom_bytes = ((rom_bytes - 1) / sizeof(zed_data_section_element_t) + 1) * sizeof(zed_data_section_element_t); // snap to nearest 128 bit boundary (sizeof(zed_data_section_element_t))
	meta_section.data_section_offset = rom_bytes;

	for (uint64_t i = 0; i < data_label_count; i++) {
		rom_data = (uint8_t*) realloc(rom_data, rom_bytes + sizeof(zed_data_section_element_t));
		memcpy(rom_data + rom_bytes, &data_section_elements[i], sizeof(zed_data_section_element_t));

		rom_bytes += sizeof(zed_data_section_element_t);
	}

	// position section

	meta_section.position_section_offset = rom_bytes;

	for (uint64_t i = 0; i < position_label_count; i++) {
		rom_data = (uint8_t*) realloc(rom_data, rom_bytes + sizeof(uint64_t));
		*((uint64_t*) (rom_data + rom_bytes)) = position_labels[i].position;
		rom_bytes += sizeof(uint64_t);
	}

	// write logic section
	
	meta_section.logic_section_offset = rom_bytes;
	meta_section.logic_section_words = logic_section_words;

	uint64_t logic_section_bytes = logic_section_words * sizeof(uint64_t);

	rom_data = (uint8_t*) realloc(rom_data, rom_bytes + logic_section_bytes);
	memcpy(rom_data + rom_bytes, logic_section_data, logic_section_bytes);
	
	rom_bytes += logic_section_bytes;
	memcpy(rom_data, &meta_section, sizeof(meta_section)); // write the meta section to the rom
}

#endif