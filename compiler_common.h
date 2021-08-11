#if !defined(__AQUA_COMPILER__COMPILER_COMMON_H)
#define __AQUA_COMPILER__COMPILER_COMMON_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <zed.h>

// section variables and macros

static uint64_t zed_rom_bytes;
static uint8_t* zed_rom_data;

static uint64_t zed_position_label_count;
static zed_token_t* zed_position_labels;

static uint64_t zed_data_label_count;
static zed_token_t* zed_data_labels;

static uint64_t zed_logic_section_words;
static uint64_t* zed_logic_section_data;

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

static inline void zed_token_set_data(zed_token_t* self, uint8_t* data, uint64_t bytes) {
	self->data_bytes = bytes;
	printf("%s: self->data: %p\n", __func__, self->data);
	self->data = (uint8_t*) realloc(self->data, self->data_bytes);
	memcpy(self->data, data, bytes);
}

static inline void zed_token_free(zed_token_t* self) {
	if (self->name) free(self->name);
	if (self->data) free(self->data);

	memset(self, 0, sizeof(*self));
}

static inline zed_token_t* zed_add_token(zed_token_t** list, uint64_t* count, const char* name) {
	*list = (zed_token_t*) realloc(*list, (*count + 1) * sizeof(zed_token_t));
	zed_token_t* token = &(*list)[(*count)++]; // *lol* :sushi:
	memset(token, 0, sizeof(*token));
	
	size_t bytes = strlen(name) + 1;
	token->name = malloc(bytes);
	memcpy(token->name, name, bytes);

	return token;
}

// rom helper functions

static inline void zed_rom_add_data(uint64_t data) {
	zed_logic_section_data = (uint64_t*) realloc(zed_logic_section_data, (zed_logic_section_words + 1) * sizeof(uint64_t));
	zed_logic_section_data[zed_logic_section_words++] = data;
}

static uint8_t zed_instruction_validated = 1;
static uint8_t zed_current_operand;

static uint64_t zed_current_instruction_position;
static zed_instruction_t zed_current_instruction;

static inline void zed_rom_validate_instruction(void) {
	if (!zed_instruction_validated) {
		// throw error when sizeof(zed_instruction_t) is greater than 8 bytes (this should never happen in theory)
		(void) sizeof(char[1 - 2 * (sizeof(zed_current_instruction) > sizeof(uint64_t))]);

		zed_logic_section_data[zed_current_instruction_position] = *((uint64_t*) &zed_current_instruction);
	}
}

#define ZED_TYPE_OPCODE 255

static inline void zed_rom_add(uint8_t type, uint64_t data) {
	if (type == ZED_TYPE_OPCODE) {
		zed_rom_validate_instruction();
		zed_instruction_validated = 0;

		zed_current_instruction_position = zed_logic_section_words;
		zed_rom_add_data(0); // leave space for the instruction
		
		memset(&zed_current_instruction, 0, sizeof(zed_current_instruction));
		zed_current_instruction.operation = data & 0xF;
		zed_current_operand = 1;

	} else {
		uint8_t is_64_bit_operand = data > 0xFFFF; // does the data need more than 16 bits to be represented?
		
		// no way to make this more elegant :cry:
		// you unfortunately can't create lists of elements that are < 8 bits in C99

		if (zed_current_operand == 1) {
			zed_current_instruction.operand1_64_bit = is_64_bit_operand;
			zed_current_instruction.operand1_type = type & 0b111;

			if (!is_64_bit_operand) zed_current_instruction.operand1_data = data & 0xFFFF;

		} else if (zed_current_operand == 2) {
			zed_current_instruction.operand2_64_bit = is_64_bit_operand;
			zed_current_instruction.operand2_type = type & 0b111;

			if (!is_64_bit_operand) zed_current_instruction.operand2_data = data & 0xFFFF;

		} else if (zed_current_operand == 3) {
			zed_current_instruction.operand3_64_bit = is_64_bit_operand;
			zed_current_instruction.operand3_type = type & 0b111;

			if (!is_64_bit_operand) zed_current_instruction.operand3_data = data & 0xFFFF;
		}

		zed_current_operand++;

		if (is_64_bit_operand) {
			zed_rom_add_data(data);
		}
	}
}

static inline void zed_rom_create(void) {
	zed_rom_bytes = 0;
	zed_rom_data = (uint8_t*) 0;

	// we need to allocate a main label at least

	zed_position_label_count = 1;
	zed_position_labels = (zed_token_t*) malloc(sizeof(zed_token_t));

	zed_position_labels->name = "main";
	zed_position_labels->position = 0; // default main position, even if not specified
	
	zed_data_label_count = 0;
	zed_data_labels = (zed_token_t*) 0;

	zed_logic_section_words = 0;
	zed_logic_section_data = (uint64_t*) 0;
}

static inline void zed_rom_free(void) {
	// no need to free anything really, user isn't expected to call zed_rom_create multiple times during the process' lifespan
}

static inline void zed_rom_build(void) {
	zed_rom_validate_instruction(); // make sure we validate the last instruction
	
	// meta section

	zed_meta_section_t meta_section = {
		.magic = ZED_MAGIC,
		.version = ZED_VERSION,

		.data_section_element_count = zed_data_label_count,
		.position_section_element_count = zed_position_label_count,
	};

	zed_rom_bytes = sizeof(meta_section);
	zed_rom_data = (uint8_t*) malloc(zed_rom_bytes);

	// data section
	
	zed_data_section_element_t* data_section_elements = (zed_data_section_element_t*) malloc(zed_data_label_count * sizeof(zed_data_section_element_t));

	printf("zed_data_label_count: %ld\n", zed_data_label_count);

	for (uint64_t i = 0; i < zed_data_label_count; i++) { // write each data element and record a pointer to it
		data_section_elements[i].bytes = zed_data_labels[i].data_bytes;
		data_section_elements[i].data_offset = zed_rom_bytes;

		zed_rom_data = (uint8_t*) realloc(zed_rom_data, zed_rom_bytes + zed_data_labels[i].data_bytes);
		memcpy(zed_rom_data + zed_rom_bytes, zed_data_labels[i].data, zed_data_labels[i].data_bytes);

		zed_rom_bytes += zed_data_labels[i].data_bytes;
	}

	zed_rom_bytes = ((zed_rom_bytes - 1) / sizeof(zed_data_section_element_t) + 1) * sizeof(zed_data_section_element_t); // snap to nearest 128 bit boundary (sizeof(zed_data_section_element_t))
	meta_section.data_section_offset = zed_rom_bytes;

	for (uint64_t i = 0; i < zed_data_label_count; i++) {
		zed_rom_data = (uint8_t*) realloc(zed_rom_data, zed_rom_bytes + sizeof(zed_data_section_element_t));
		memcpy(zed_rom_data + zed_rom_bytes, &data_section_elements[i], sizeof(zed_data_section_element_t));

		zed_rom_bytes += sizeof(zed_data_section_element_t);
	}

	// position section

	meta_section.position_section_offset = zed_rom_bytes;

	for (uint64_t i = 0; i < zed_position_label_count; i++) {
		zed_rom_data = (uint8_t*) realloc(zed_rom_data, zed_rom_bytes + sizeof(uint64_t));
		*((uint64_t*) (zed_rom_data + zed_rom_bytes)) = zed_position_labels[i].position;
		zed_rom_bytes += sizeof(uint64_t);
	}

	// write logic section
	
	meta_section.logic_section_offset = zed_rom_bytes;
	meta_section.logic_section_words = zed_logic_section_words;

	uint64_t zed_logic_section_bytes = zed_logic_section_words * sizeof(zed_instruction_t);

	zed_rom_data = (uint8_t*) realloc(zed_rom_data, zed_rom_bytes + zed_logic_section_bytes);
	memcpy(zed_rom_data + zed_rom_bytes, zed_logic_section_data, zed_logic_section_bytes);

	zed_rom_bytes += zed_logic_section_bytes;
	memcpy(zed_rom_data, &meta_section, sizeof(meta_section)); // write the meta section to the rom
}

// macros for mnemonics to make it easier to design compilers
// this is very much Ugly Code™, but it isn't meant to be anything beautiful

#if defined(COMPILER_MNEMONICS)
	// label mnemonics

	static zed_token_t* zed_compiler_mnemonics_current_data_label;

	// this DATA_LABEL/POS_LABEL vs DATA_LABEL_STR/POS_LABEL_STR bs is really here to make compiler code look cooler

	#define DATA_LABEL_STR(name) zed_compiler_mnemonics_current_data_label = zed_add_token(&zed_data_labels, &zed_data_label_count, name);
	#define DATA_LABEL(name) DATA_LABEL_STR(#name)
	
	#define DATA_LABEL_ADD(byte) zed_token_add_data(zed_compiler_mnemonics_current_data_label, (byte));
	#define DATA_LABEL_SET(data, bytes) zed_token_set_data(zed_compiler_mnemonics_current_data_label, (uint8_t*) (data), (bytes));
	#define DATA_LABEL_SET_STR(string) DATA_LABEL_SET((string), strlen(string) + 1)

	static inline void zed_create_position_label(const char* name) { // ties in with zed_rom_add_potentially_undeclared_position
		zed_token_t* self;
		int64_t index = zed_token_find(zed_position_labels, zed_position_label_count, name);

		if (index < 0) { // undeclared position, declare it
			self = zed_add_token(&zed_position_labels, &zed_position_label_count, name);
			
		} else {
			self = &zed_position_labels[index];
		}
		
		self->position = zed_logic_section_words;
	}

	#define POS_LABEL_STR(name) zed_create_position_label(name);
	#define POS_LABEL(name) POS_LABEL_STR(#name)
	#define MAIN POS_LABEL(main)

	// operand mnemonics

	#define REG(name) (zed_rom_add(ZED_OPERAND_16_TYPE_REGISTER, ZED_REGISTER_##name))

	#define CONST_UNKNOWN (-1ull) // 64 bit number filled with 1's

	#define CONST16(expression) (zed_rom_add(ZED_OPERAND_16_TYPE_CONSTANT, (uint16_t) (expression)))
	#define CONST64(expression) (zed_rom_add(ZED_OPERAND_64_TYPE_CONSTANT, (expression)))

	// generic 'CONST', automatically evaluates if 'expression' fits in 16 bits
	// uses 'CONST16' if it is, 'CONST64' if not

	#define CONST(expression) ((uint64_t) (expression) < 0x10000 ? CONST16(expression) : CONST64(expression))

	#define ADDR64(name) (zed_rom_add(ZED_OPERAND_16_TYPE_ADDRESS_64, ZED_REGISTER_##name))
	#define ADDR8( name) (zed_rom_add(ZED_OPERAND_16_TYPE_ADDRESS_8 , ZED_REGISTER_##name))

	static inline void zed_rom_add_potentially_undeclared_position(const char* name) { // retardedly long function names ftw... MHHHHHH
		int64_t index = zed_token_find(zed_position_labels, zed_position_label_count, name);

		if (index < 0) { // undeclared position, declare it
			zed_add_token(&zed_position_labels, &zed_position_label_count, name);
			index = zed_token_find(zed_position_labels, zed_position_label_count, name);
		}

		zed_rom_add(ZED_OPERAND_16_TYPE_POSITION_INDEX, index);
	}

	// _DATA because DATA conflicts with y.tab.c

	#define POS_STR(  name) (zed_rom_add_potentially_undeclared_position(name))
	#define _DATA_STR(name) (zed_rom_add(ZED_OPERAND_16_TYPE_DATA_INDEX, zed_token_find(zed_data_labels, zed_data_label_count, name)))

	#define POS(  name) POS_STR(#name)
	#define KFUNC(name) (zed_rom_add(ZED_OPERAND_16_TYPE_KFUNC_INDEX, ZED_KFUNC_##name))
	#define _DATA(name) _DATA_STR(#name)

	// opcode mnemonics

	#define OPCODE(opcode) (zed_rom_add(ZED_TYPE_OPCODE, opcode)),

	#define MOV OPCODE(ZED_OPCODE_MOV)

	#define RED OPCODE(ZED_OPCODE_RED)
	#define INV OPCODE(ZED_OPCODE_INV)

	#define JMP OPCODE(ZED_OPCODE_JMP)
	#define CAL OPCODE(ZED_OPCODE_CAL)

	#define CPE OPCODE(ZED_OPCODE_CPE)

	#define ADD OPCODE(ZED_OPCODE_ADD)
	#define SUB OPCODE(ZED_OPCODE_SUB)
	#define MUL OPCODE(ZED_OPCODE_MUL)
	#define DIV OPCODE(ZED_OPCODE_DIV)

	#define AND OPCODE(ZED_OPCODE_AND)
	#define OR  OPCODE(ZED_OPCODE_OR )
	#define XOR OPCODE(ZED_OPCODE_XOR)

	#define SHL OPCODE(ZED_OPCODE_SHL)
	#define SHR OPCODE(ZED_OPCODE_SHR)

	// subroutine macros

	// push subroutine: subtract 8 bytes from stack pointer and move 'expression' into address pointed to by it

	#define SUB_PSH(expression) { \
		SUB REG(SP), REG(SP), CONST16(8); \
		MOV ADDR64(SP), (expression); \
	}

	// pop subroutine: move value pointed to by stack pointer to expression add 8 bytes from it

	#define SUB_POP(expression) { \
		MOV (expression), ADDR64(SP); \
		ADD REG(SP), REG(SP), CONST16(8); \
	}

	// call subroutine: push stack pointer to the stack and jump to 'expression'

	static uint64_t zed_call_count = 0;

	#define SUB_CAL(expression, call_count) { \
		char temp[64]; \
		sprintf(temp, "$call%d", (call_count)); \
		SUB_PSH(POS_STR(temp)); \
		JMP (expression); \
		POS_LABEL_STR(temp) {} \
	}

	// return subroutine: pop last value of instruction pointer off stack pointer and jump to that
	// idk if 'mov ip, sp' is very standard-conforming
	// using the 'f3' register because I can be pretty sure it's not going to be used by anything else

	#define SUB_RET() { \
		SUB_POP(REG(F3)); \
		JMP REG(F3); \
	}
#endif

#endif