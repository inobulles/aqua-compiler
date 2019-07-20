
/*
MIT License

Copyright (c) 2018 Inobulles

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/* VERSION HISTORY
 * 2: Adds byte addressing support
 * 3: Reform of ISA. Removes some redundant instructions
 */

#if __ASM_ZVM_V__
	#define VERSION __ASM_ZVM_V__
#else
	#define VERSION 3
#endif

#ifndef VERBOSE_MODE
	#define VERBOSE_MODE 1
#endif
#ifndef DEBUGGING_MODE
	#define DEBUGGING_MODE 0
#endif

#define ULTRA_VERBOSE 0
#define HINT          1

#ifndef VERBOSE_MODE
	#define VERBOSE_MODE 1
#endif
#ifndef DEBUGGING_MODE
	#define DEBUGGING_MODE 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compat with C20

typedef   signed long long var;
typedef unsigned long long uvar;

typedef var chr;

#define print printf

#define mfree(ptr, ...) free(ptr)

// global

#define SECTION_NULL 0
#define SECTION_TEXT 1
#define SECTION_DATA 2

typedef struct {
	var error;
	var warning;
	
	var* string;
	var  bytes;
	
	#define MAX_TOKEN_LENGTH 4096
	chr __token_buffer[MAX_TOKEN_LENGTH];
	chr*  token_buffer;
	var   token_buffer_pointer;
	
	var asm_index;
	
	// flags
	
	var section;
	var in_comment;
	var awaiting_section;
	
	// rom
	
	var* rom;
	var  rom_length;
	
	var  rom_offset;
	var  rom_pack;
	
	// rom data
	
	var length;
	
	var base_reserved_count;
	var label_position_offset;
	var main_reserved_position;
	
	// data section
	
	var data_section_element_count;
	
	// data section element
	
	var data_section_element_element_count;
	var data_section_element_qtype;
	
	var data_section_offset;
	var data_section_pack;
	
	// reserved positions section
	
	var  reserved_positions_count;
	var* reserved_positions_markers;
	var  reserved_positions_markers_pointer;
	
	var* label_positions;
	var  label_positions_pointer;
	
	// reserved / prereserved
	
	#define               MAX_RESERVED_COUNT 0x10000
	var* reserved        [MAX_RESERVED_COUNT];
	var  reserved_lengths[MAX_RESERVED_COUNT];
	var  reserved_pointer;
	
	#define                     MAX_DATA_RESERVED_COUNT    0x10000
	var* data_reserved         [MAX_DATA_RESERVED_COUNT];
	var  data_reserved_lengths [MAX_DATA_RESERVED_COUNT];
	var  data_reserved_pointer;
	
} compiler_t;

static void load_asm(compiler_t* this) {
	FILE* fp = fopen("code.asm", "rb");
	
	fseek(fp, 0, SEEK_END);
	this->bytes = ftell(fp);
	rewind(fp);
	
	this->string = (var*) malloc(++this->bytes);
	fread(this->string, sizeof(char), this->bytes, fp);
	
	((char*) this->string)[this->bytes] = 0;
	
	//~ fclose(fp); // for some reason, this causes `corrupted size vs. prev_size`
	
}

static void free_asm(compiler_t* this) {
	mfree(this->string, this->bytes);
	
}

static void write_rom(compiler_t* this) {
	FILE* fp = fopen("ROM.zed", "wb");
	fwrite(this->rom, sizeof(var), this->rom_length, fp);
	fclose(fp);
	
}

// the following is what is in common with C20

static chr strindex(var* string, var index) {
	var aligned     = index  / 8;
	var offset      = index  % 8;
	var offset_bits = offset * 8;
	
	return (string[aligned] >> offset_bits) & 0xFF;
	
}

static void print_token(chr* token_buffer) {
	var i;
	for (i = 0; token_buffer[i]; i++) {
		print("%c", (int) token_buffer[i]);
		
	}
	
}

static var tokeneq(chr* token, const char* string) {
	var i;
	for (i = 0; i < MAX_TOKEN_LENGTH; i++) {
		chr string_current = strindex((var*) string, i);
		chr token_current  = token[i];
		
		if      (!token_current && !string_current) return 1;
		else if (!token_current || !string_current) return 0;
		else if ( token_current !=  string_current) return 0;
		
	}
	
	return 0;
	
}

static var tokeneqtoken(chr* a, chr* b) {
	var i;
	for (i = 0; i < MAX_TOKEN_LENGTH; i++) {
		chr _a = a[i];
		chr _b = b[i];
		
		if      (!_a && !_b) return 1;
		else if (!_a || !_b) return 0;
		else if ( _a !=  _b) return 0;
		
	}
	
	return 0;
	
}

static var tokenlen(chr* token) {
	var length = 0;
	while (*token++) length++;
	return length;
	
}

static var is_number(chr* token) {
	var i;
	for (i = 0; token[i]; i++) {
		if (!(token[i] == 0 || (token[i] >= 48 && token[i] <= 57))) {
			return 0;
			
		}
		
	}
	
	return 1;
	
}

static var pow(var x, var power) {
	if (power == 0) {
		return 1;
		
	}
	
	var result = x;
	
	var i;
	for (i = 0; i < power - 1; i++) {
		result *= x;
		
	}
	
	return result;
	
}

static uvar strint(chr* token) {
	var  digits[64];
	var  length = 0;
	uvar result = 0;
	
	var i;
	for (i = 0; token[i]; i++) {
		digits[length++] = token[i] - 48;
		
	}
	
	for (i = length - 1; i >= 0; i--) {
		result += pow(10, i) * digits[(length - 1) - i];
		
	}
	
	return result;
	
}

static uvar strhex(chr* token) {
	var  digits[64];
	var  length = 0;
	uvar result = 0;
	
	var i;
	for (i = 0; token[i]; i++) {
		if      (token[i]=='A'||token[i]=='a') digits[length++] = 0xA;
		else if (token[i]=='B'||token[i]=='b') digits[length++] = 0xB;
		else if (token[i]=='C'||token[i]=='c') digits[length++] = 0xC;
		else if (token[i]=='D'||token[i]=='d') digits[length++] = 0xD;
		else if (token[i]=='E'||token[i]=='e') digits[length++] = 0xE;
		else if (token[i]=='F'||token[i]=='f') digits[length++] = 0xF;
		else                                   digits[length++] = token[i] - 48;
		
	}
	
	for (i = length - 1; i >= 0; i--) {
		result += pow(16, i) * digits[(length - 1) - i];
		
	}
	
	return result;
	
}

static uvar strbin(chr* token) {
	var  digits[64];
	var  length = 0;
	uvar result = 0;
	
	var i;
	for (i = 0; token[i]; i++) {
		if (\
			token[i]=='1'||token[i]=='.'||\
			token[i]=='I'||token[i]=='i'||\
			token[i]=='Y'||token[i]=='y'||\
			token[i]=='H'||token[i]=='h'||\
			token[i]=='T'||token[i]=='t') digits[length++] = 1;
		else if (\
			token[i]=='0'||token[i]=='_'||\
			token[i]=='O'||token[i]=='o'||\
			token[i]=='N'||token[i]=='n'||\
			token[i]=='L'||token[i]=='l'||\
			token[i]=='F'||token[i]=='f') digits[length++] = 0;
		else                              digits[length++] = 0;
		
	}
	
	for (i = length - 1; i >= 0; i--) {
		result += pow(2, i) * digits[(length - 1) - i];
		
	}
	
	return result;
	
}

// this is the actual compiler

static var parse_number(chr* token_buffer, var token_buffer_pointer, uvar* __number) {
	#define __parse_number_c(index, compare) (        token_buffer[index] == (compare))
	#define __parse_number_e(       compare) tokeneq( token_buffer,           compare)
	
	var  has_number = 1;
	uvar number     = 0;
	var  negative   = token_buffer[0] == '-';
	
	if (negative) {
		token_buffer++;
		
	}
	
	if (token_buffer[token_buffer_pointer - 1] == 'H') { // hex number
		token_buffer[token_buffer_pointer - 1] = 0;
		number = strhex(token_buffer);
		token_buffer[token_buffer_pointer - 1] = 'H';
		
	} else if (__parse_number_c(0,'0')&&__parse_number_c(1,'x')) { // hex number
		token_buffer += 2;
		number = strhex(token_buffer);
		
	} else if (__parse_number_c(0,'0')&&__parse_number_c(1,'b')) { // bin number
		token_buffer += 2;
		number = strbin(token_buffer);
		
	} else if (is_number(token_buffer)) { // is number
		number = strint(token_buffer);
		
	} else {
		has_number = 0;
		
	}
	
	*__number = negative ? -number : number;
	return has_number;
	
}

static void reset_token_buffer(compiler_t* this) {
	this->token_buffer_pointer = 0;
	this->token_buffer[0]      = 0;
	
}

static var __add_rom(compiler_t* this, uvar value) {
	uvar old_bytes = this->rom_length * sizeof(uvar);
	
	uvar*   temp = (uvar*) malloc(old_bytes);
	memcpy(temp, this->rom,       old_bytes);
	
	mfree(this->rom, old_bytes);
	
	this->rom_length++;
	this->rom = malloc(this->rom_length * sizeof(uvar));
	this->rom[this->rom_length - 1] = value;
	
	memcpy(this->rom, temp, old_bytes);
	mfree(            temp, old_bytes);
	
	return this->rom_length - 1;
	
}

static var add_rom(compiler_t* this, uvar value) {
	if (this->rom_offset % 8) {
		__add_rom(this, this->rom_pack);
		
		this->rom_pack   = 0;
		this->rom_offset = 0;
		
	}
	
	return __add_rom(this, value);
	
}

static void add_rom_byte(compiler_t* this, uvar value) {
	var offset = this->rom_offset++ % 8;
	this->rom_pack += value << (offset * 8);
	
	if (offset == 7) {
		__add_rom(this, this->rom_pack);
		
		this->rom_pack   = 0;
		this->rom_offset = 0;
		
	}
	
}

static void add_rom_instruction(compiler_t* this, uvar type, uvar data) {
	add_rom_byte(this, type);
	add_rom_byte(this, data);
	
}

static void set_rom_value(compiler_t* this, var index, var value) {
	if (index < this->rom_length) {
		this->rom[index] = value;
		
	}
	
}

static var get_rom_value(compiler_t* this, var index) {
	return this->rom[index];
	
}

static var get_label_position(compiler_t* this) {
	return this->rom_length * sizeof(var) + this->rom_offset % 8;
	
}

static void add_reserved(compiler_t* this, var is_data, chr* token) {
	var length = tokenlen(token) + 1;
	
	if (is_data) {
		this->data_reserved_lengths[this->data_reserved_pointer] = length;
		this->data_reserved        [this->data_reserved_pointer] = (var*) malloc(length * sizeof(var));
		memcpy(this->data_reserved [this->data_reserved_pointer], token,         length * sizeof(var));
		
		if (++this->data_reserved_pointer >= MAX_DATA_RESERVED_COUNT) {
			printf("ERROR You have surpassed MAX_DATA_RESERVED_COUNT (%d)\n", MAX_DATA_RESERVED_COUNT);
			exit(1);
			
		}
		
	}
	
	this->reserved_lengths[this->reserved_pointer] = length;
	this->reserved        [this->reserved_pointer] = (var*) malloc(length * sizeof(var));
	memcpy(this->reserved [this->reserved_pointer], token,         length * sizeof(var));

	if (++this->reserved_pointer >=      MAX_RESERVED_COUNT) {
		print("ERROR You have surparssed MAX_RESERVED_COUNT (%d)\n", MAX_RESERVED_COUNT);
		exit(1);
		
	}
	
}

#define TOKEN_KEYWORD     1ll
#define TOKEN_REGISTER    2ll
#define TOKEN_RESERVED    3ll
#define TOKEN_NUMBER      4ll
#define TOKEN_ADDRESS     6ll
#define TOKEN_PRERESERVED 9ll
#define TOKEN_QTYPE      12ll
#define TOKEN_BYTE       13ll
#define TOKEN_BYTE_ADDR  14ll

static void add_instruction(compiler_t* this, uvar type, uvar argument) {
	if (type == TOKEN_NUMBER && argument < 0x100) {
		type =  TOKEN_BYTE;
		
	}
	
	if (type==TOKEN_NUMBER||type==TOKEN_RESERVED||type==TOKEN_PRERESERVED||type==TOKEN_QTYPE) {
		add_rom_instruction(this, type, 0);
		add_rom            (this, argument);
		
	} else {
		add_rom_instruction(this, type, argument);
		
	}
	
}

#include "database.h"
#include "defs.h"

static var get_reg_id(compiler_t* this, chr* token) {
	var i;
	for (i = 0; i < len_registers; i++) {
		if (tokeneq(token, registers[i].name)) { // found register
			return i;
			
		}
		
	}
	
	this->warning++;
	print("WARNING Could not find a register match for token `");
	print_token(token);
	print("`\n");
	return -1;
	
}

var main(void) {
	print("Loaded\n");
	
	compiler_t _this;
	compiler_t* this = &_this;
	
	load_asm(this);
	this->asm_index = 0;
	
	this->error   = 0;
	this->warning = 0;
	
	// flags
	
	this->section          = SECTION_DATA;
	this->in_comment       = 0;
	this->awaiting_section = 0;
	
	// data section
	
	this->rom_offset = 8;
	this->rom_length = 0;
	this->rom = (var*) malloc(this->rom_length * sizeof(var));
	
	this->length = add_rom(this, 0);
	
	add_rom(this, VERSION); // version
	add_rom(this, 0);       // invalidated
	
	print("len_base_reserved = %d\n", len_base_reserved);
	
	this->base_reserved_count    = add_rom(this, len_base_reserved);
	this->label_position_offset  = add_rom(this, len_base_reserved);
	this->main_reserved_position = add_rom(this, 0);
	
	add_rom(this, DEBUGGING_MODE);
	add_rom(this, VERBOSE_MODE);
	
	this->reserved_positions_markers_pointer = 0;
	this->reserved_positions_count           = 0;
	
	this->reserved_pointer                   = 0;
	this->data_reserved_pointer              = 0;
	
	this->data_section_element_count = add_rom(this, 0);
	this->data_section_offset = 8;
	
	this->token_buffer = (chr*) this->__token_buffer;
	reset_token_buffer(this);
	
	chr current;
	chr previous_seperator = ' ';
	chr previous = 0;
	
	var data_section_index   = 0;
	var text_section_index   = 0;
	var parsing_data_section = 0;
	
	#define IS_WHITESPACE(_char) (_char==','||_char==' '||_char=='\t'||_char=='\n'||_char=='\r'||_char==';')
	print("\nGetting reserved_positions_count ...\n");
	
	while (1) { // get reserved_positions_count
		current = strindex(this->string, this->asm_index++);
		
		if (!this->in_comment) {
			if (IS_WHITESPACE(current)) {
				if (previous == ':') {
					#if ULTRA_VERBOSE
						print("Found text/data label, adding to reserved positions ...\n");
					#endif
					
					this->reserved_positions_count++;
					
					if (parsing_data_section) {
						set_rom_value(this, this->label_position_offset, get_rom_value(this, this->label_position_offset) + 1);
						
					}
					
				}
				
			} else if (current == '.') {
				if (strindex(this->string, this->asm_index)     == 'd' &&
					strindex(this->string, this->asm_index + 1) == 'a' &&
					strindex(this->string, this->asm_index + 2) == 't' &&
					strindex(this->string, this->asm_index + 3) == 'a') {
					data_section_index =   this->asm_index + 4;
					parsing_data_section = 1;
					
				} else if ( \
					strindex(this->string, this->asm_index)     == 't' &&
					strindex(this->string, this->asm_index + 1) == 'e' &&
					strindex(this->string, this->asm_index + 2) == 'x' &&
					strindex(this->string, this->asm_index + 3) == 't') {
					text_section_index =   this->asm_index + 4;
					parsing_data_section = 0;
					
				}
				
			}
			
		}
		
		if      (current == ';')  this->in_comment = 1;
		else if (current == '\n') this->in_comment = 0;
		else if (current == 0)    break;
		
		previous = current;
		
	}
	
	this->in_comment = 0;
	this->asm_index  = data_section_index;
	
	parsing_data_section = 1;
	
	var name_length = -1;
	print("\nGetting reserved_positions ...\n");
	
	while (1) { // get reserved_positions (the names of them labels)
		if (!parsing_data_section && this->asm_index >= data_section_index) {
			break;
			
		}
		
		current = strindex(this->string, this->asm_index++);
		
		if (current == 0) {
			this->asm_index      = text_section_index;
			parsing_data_section = 0;
			
		}
		
		if (!this->in_comment) {
			if (IS_WHITESPACE(current)) {
				if (previous == ':') {
					chr label_name[name_length + 1];
					    label_name[name_length] = 0;
					
					var i;
					for (i = 0; i < name_length; i++) {
						label_name[i] = strindex(this->string, this->asm_index - name_length + i - 2);
						
					}
					
					add_reserved(this, parsing_data_section, label_name);
					
					#if ULTRA_VERBOSE
						print("Found %s label (length = %lld) ", parsing_data_section ? "data" : "text", name_length);
						print_token(label_name);
						print(", adding to%sreserved positions names ...\n", parsing_data_section ? " data " : " ");
					#endif
					
				}
				
				name_length = -1;
				
			} else {
				name_length++;
				
			}
			
		}
		
		if      (current == ';')  this->in_comment = 1;
		else if (current == '\n') this->in_comment = 0;
		
		previous = current;
		
	}
	
	this->in_comment = 0;
	this->asm_index  = data_section_index;
	
	var done_data    = 0;
	var next_address = 0;
	
	print("Data section at %lld\n", data_section_index);
	print("\nParsing sections ...\n");
	
	#define MAX_ARGS_FOR_INSTR 6
	uvar current_type[MAX_ARGS_FOR_INSTR];
	uvar current_args[MAX_ARGS_FOR_INSTR];
	var  current_pointer = 0;
	
	#define __ADD_CURRENT(type, arg) { \
		current_type[current_pointer] = (type); \
		current_args[current_pointer] = (arg); \
		current_pointer++; \
	}
	
	#if VERSION >= 3
		#define TEST_AFTER_FACT_INSTRUCTION \
			if (inc_dec_instruction) { \
				inc_dec_instruction = 0; \
				__ADD_CURRENT(TOKEN_NUMBER, 1) \
			} else if (test_instruction) { \
				test_instruction = 2; \
				__ADD_CURRENT(TOKEN_NUMBER, 0); \
			}
	#else
		#define TEST_AFTER_FACT_INSTRUCTION
	#endif
	
	#define ADD_CURRENT(type, arg) { \
		__ADD_CURRENT((type), (arg)) \
		is_next_byte_address = 0; \
		TEST_AFTER_FACT_INSTRUCTION \
	}
	
	#define ADD_PREVIOUS_INSTRUCTION \
		if (current_type[0] == 1) { \
			if (current_args[0] == TOKEN_mul) { \
				if      (current_pointer == 4) current_args[0] = TOKEN_mul3; \
				else if (current_pointer == 2) current_args[0] = TOKEN_mul1; \
			} else if (current_args[0] == TOKEN_div) { \
				if      (current_pointer == 4) current_args[0] = TOKEN_div3; \
				else if (current_pointer == 2) current_args[0] = TOKEN_div1; \
			} \
		} \
		for (var i = 0; i < current_pointer; i++) { \
			add_instruction(this, current_type[i], current_args[i]); \
		} \
		current_pointer = 0;
	
	set_rom_value(this, this->data_section_element_count, this->data_reserved_pointer);
	var data_section_element_descriptor_markers[this->data_reserved_pointer];
	var data_section_element_descriptor_markers_index = 0;
	
	var k;
	for (k = 0; k < this->data_reserved_pointer; k++) {
		data_section_element_descriptor_markers[k] = add_rom(this, -1);
		add_rom(this, -1);
		
	}
	
	#if VERSION >= 3
		var inc_dec_instruction = 0;
		var    test_instruction = 0;
	#endif
	
	var ignore_next_token    = 0;
	var is_next_byte_address = 0;
	
	while (1) { // main loop
		current = strindex(this->string, this->asm_index++);
		
		if (current == 0) {
			if (!done_data) {
				 done_data = 1;
				
				// data section
				
				print("*this->data_section_element_count = %lld\n", get_rom_value(this, this->data_section_element_count));
				
				// reserved section
				
				this->reserved_positions_count -= get_rom_value(this, this->data_section_element_count) + 1;
				print("this->reserved_positions_count = %lld\n", this->reserved_positions_count);
				add_rom(this, this->reserved_positions_count);
				
				this->reserved_positions_markers = (var*) malloc(this->reserved_positions_count * sizeof(var));
				
				var i;
				for (i = 0; i < this->reserved_positions_count; i++) {
					this->reserved_positions_markers[i] = add_rom(this, -1);
					
				}
				
				// flow alteration
				
				print("Reached the end of data section. Parsing from the top ...\n");
				this->asm_index = 0;
				continue;
				
			} else {
				print("Reached the end of program\n");
				break;
				
			}
			
		}
		
		if (this->in_comment) {
			if (current == '\n') {
				this->in_comment = 0;
				
			} else {
				continue;
				
			}
			
		}
		
		if (current == ';') {
			this->in_comment = 1;
			continue;
			
		}
		
		if (current == '\n') {
			ADD_PREVIOUS_INSTRUCTION
			
		}
		
		if (IS_WHITESPACE(current)) { // process current token and start a new one
			previous_seperator = current;
			
			if (this->token_buffer[0]) { // is token empty? if not, check if it is known
				#define c(index, compare) (        this->token_buffer[index] == (compare))
				#define e(       compare) tokeneq( this->token_buffer,           compare)
				
				uvar number;
				var  has_number = 0;
				
				if (!this->awaiting_section) {
					has_number = parse_number(this->token_buffer, this->token_buffer_pointer, &number);
					
				}
				
				if (this->awaiting_section) {
					this->awaiting_section = 0;
					
					if (e(".text")||e("text")) {
						if (this->section != SECTION_TEXT) { // enter text section
							this->section  = SECTION_TEXT;
							print("SECTION TEXT\n");
							
						}
						
					} else if (e(".rodata")||e(".data")||e(".bss")) {
						if (done_data) {
							break;
							
						}
						
						if (this->section != SECTION_DATA) {
							this->section  = SECTION_DATA;
							print("SECTION DATA\n");
							
						}
						
					} else {
						this->warning++;
						print("WARNING Found unknown section: ");
						print_token(this->token_buffer);
						print("\n");
						
					}
					
				} else if ( \
					e("default") || \
					e("global")  || \
					e("extern")  || \
					(c(0,'a')&&c(1,'l')&&c(2,'i')&&c(3,'g')&&c(4,'n')&&c(5,'='))) {
					this->in_comment = 1;
					
				} else if (e("SECTION") || e(".section") || e("section")) {
					this->awaiting_section = 1;
					
				} else if (this->token_buffer[this->token_buffer_pointer - 1] == ':') { // label declaration
					this->token_buffer[this->token_buffer_pointer - 1] = 0;
					var pos = get_label_position(this);
					
					if (this->section == SECTION_DATA) {
						this->data_section_element_element_count = data_section_element_descriptor_markers[data_section_element_descriptor_markers_index];
						this->data_section_element_qtype         = data_section_element_descriptor_markers[data_section_element_descriptor_markers_index] + 1;
						
						set_rom_value(this, this->data_section_element_qtype, 0);
						data_section_element_descriptor_markers_index++;
						
					} else if (this->section == SECTION_TEXT) {
						set_rom_value(this, this->reserved_positions_markers[this->reserved_positions_markers_pointer++], pos);
						
						if (tokeneq(this->token_buffer, "main")) {
							print("Found main label declaration, setting this->main_reserved_position to %lld ...\n", pos);
							set_rom_value(this, this->main_reserved_position, pos);
							
						}
						
					} else {
						print("Found label declaration: ");
						print_token(this->token_buffer);
						print("\n");
						
					}
					
				} else if (this->section == SECTION_TEXT) { // for speeding stuff up
					var stop_from_address = 0;
					
					if (next_address || c(0, '[')) {
						stop_from_address = 1;
						
						if (e("[rel")) {
							next_address = 1;
							
						} else {
							this->token_buffer[this->token_buffer_pointer - 1] = 0;
							chr* format = next_address ? this->token_buffer : this->token_buffer + 1;
							chr* base   = format;
							
							#define MAX_OPERATIONS 5
							
							var operation_type = 0;
							chr operation_list[MAX_OPERATIONS] = {0, 0, 0, 0, 0};
							var operation_list_ptr = 0;
							
							chr operands[MAX_OPERATIONS][MAX_TOKEN_LENGTH] = {{0}, {0}, {0}, {0}, {0}};
							var operand_ptr       = 0;
							var operand_inner_ptr = 0;
							
							format--;
							
							while (*(++format)) {
								if (*format == '+' ||
									*format == '-' ||
									*format == '*') {
									operation_type = 1;
									operation_list[operation_list_ptr++] = *format;
									
									operands[operand_ptr++][operand_inner_ptr] = 0;
									operand_inner_ptr = 0;
									
								} else {
									operands[operand_ptr][operand_inner_ptr++] = *format;
									
								}
								
							}
							
							if (!operation_type) {
								stop_from_address = 0;
								
							} else {
								var pass;
								for (pass = 0; pass < 2; pass++) {
									var j;
									for (j = 0; j < operation_list_ptr; j++) {
										var current_operation = operation_list[j];
										
										var* left  = operands[j];
										var* right = operands[j + 1];
										
										var left_number;
										var right_number;
										
										#define SET_OPERAND_ADDR(op) { \
											op[0] = 'a'; \
											op[1] = 'd'; \
											op[2] = 'd'; \
											op[3] = 'r'; \
											op[4] = 0;   \
										}
										
										#define CREATE_CLA(operation) { \
											add_rom_instruction(this, TOKEN_KEYWORD, TOKEN_cla); /* start by adding the cla instruction */ \
											var __CREATE_CLA_type; \
											var __CREATE_CLA_arg; \
											if (parse_number(left, tokenlen(left), &left_number)) { \
												__CREATE_CLA_type = TOKEN_NUMBER; \
												__CREATE_CLA_arg  = left_number; \
											} else { \
												__CREATE_CLA_type = TOKEN_REGISTER; \
												__CREATE_CLA_arg  = get_reg_id(this, left); \
											} \
											add_instruction(this, __CREATE_CLA_type, __CREATE_CLA_arg); /* add left operand */ \
											add_instruction(this, TOKEN_KEYWORD, operation);            /* add operator     */ \
											if (parse_number(right, tokenlen(right), &right_number)) { \
												__CREATE_CLA_type = TOKEN_NUMBER; \
												__CREATE_CLA_arg  = right_number; \
											} else { \
												__CREATE_CLA_type = TOKEN_REGISTER; \
												__CREATE_CLA_arg  = get_reg_id(this, right); \
											} \
											add_instruction(this, __CREATE_CLA_type, __CREATE_CLA_arg); /* add right operand */ \
										}
										
										if (pass == 0) {
											if (current_operation == '*') {
												#if ULTRA_VERBOSE
													print("cla ");print_token(left);print(" mul ");print_token(right);print("\n");
												#endif
												
												CREATE_CLA(8) // create cla instruction with TOKEN_mul (8)
												
												SET_OPERAND_ADDR(left)
												SET_OPERAND_ADDR(right)
												
											} else if (current_operation == '-' || current_operation == '+') {
											} else {
												print("WARNING Unknown first priority operation %c\n", (char) current_operation);
												this->warning++;
												
											}
											
										} else if (pass == 1) {
											if (current_operation == '-') {
												#if ULTRA_VERBOSE
													print("cla ");print_token(left);print(" sub ");print_token(right);print("\n");
												#endif
												
												CREATE_CLA(7) // create cla instruction with TOKEN_sub (7)
												
												SET_OPERAND_ADDR(left)
												SET_OPERAND_ADDR(right)
												
											} else if (current_operation == '+') {
												#if ULTRA_VERBOSE
													print("cla ");print_token(left);print(" add ");print_token(right);print("\n");
												#endif
												
												CREATE_CLA(6) // create cla instruction with TOKEN_add (6)
												
												SET_OPERAND_ADDR(left)
												SET_OPERAND_ADDR(right)
												
											} else if (current_operation == '*') {
											} else {
												print("WARNING Unknown second priority operation %c\n", (char) current_operation);
												this->warning++;
												
											}
											
										}
										
									}
									
								}
								
								ADD_CURRENT(TOKEN_ADDRESS, REGISTER_addr) // value at address <addr register value>
								
								#if ULTRA_VERBOSE
									print("\n");
								#endif
								
							}
							
							next_address = 0;
							
						}
						
					} if (ignore_next_token > 0) {
						ignore_next_token--;
						
					} else if (stop_from_address) {
					} else if (has_number) { // is number
						ADD_CURRENT(TOKEN_NUMBER, number)
						
					} else if (e("nop")) { // ignore
					    
					} else if (e("ALIGN")) { // ignore this and the next token
						ignore_next_token++;
						
					} else if (e("qword")) {
					    is_next_byte_address = 0;
						
					} else if (e("byte")) {
					    #if VERSION >= 2
					        is_next_byte_address = 1;
						#else
						    this->warning++;
						    print("WARNING Found byte address. This is unsupported in current version (%llu), and thus will be replaced by qword. Switch to version 2 or higher to get support for byte addressing.\n", VERSION);
						#endif
						
					} else if (this->token_buffer[0] != '\r') { // could not match the token
						var address_type = 0;
						
						if (this->token_buffer[0] == '[') { // address type
							this->token_buffer++;
							address_type = 1;
							
						}
						
						var next = 0;
						var condition_evaluated = 0;
						
						// test against already known tokens
						
						#define TOKEN_REGA (address_type ? (is_next_byte_address ? TOKEN_BYTE_ADDR : TOKEN_ADDRESS) : TOKEN_REGISTER)
						
						#define CONFIRM_CONDITION(__CONFIRM_CONDITION_i) { \
							add_instruction(this, TOKEN_KEYWORD, 37); \
							add_instruction(this, TOKEN_BYTE,    (__CONFIRM_CONDITION_i)); \
							condition_evaluated = 1; \
							break; \
						}
						
						#define TEST_CONDITION(offset) do { \
							next = 1; \
							chr* condition_token = this->token_buffer + (offset); \
							condition_evaluated = 0; \
							if      (tokeneq(condition_token,"l") ||tokeneq(condition_token,"b") ||tokeneq(condition_token,"c"))  CONFIRM_CONDITION(6) \
							else if (tokeneq(condition_token,"g") ||tokeneq(condition_token,"a") ||tokeneq(condition_token,"nc")) CONFIRM_CONDITION(7) \
							else if (tokeneq(condition_token,"be"))                                                               CONFIRM_CONDITION(8) \
							else if (tokeneq(condition_token,"ae"))                                                               CONFIRM_CONDITION(9) \
							var i; \
							for (i = 0; i < len_conditions; i++) { \
								if (tokeneq(condition_token, conditions[i].name)) CONFIRM_CONDITION(i) \
							} \
						} while (0);
						
						#if VERSION >= 3
							if (test_instruction == 2) {
								test_instruction       = 0;
							} else
						#endif
						
						if      (e("edi"))    ADD_CURRENT(TOKEN_REGA,    REGISTER_rdi) // replace edi    by rdi
						else if (e("esi"))    ADD_CURRENT(TOKEN_REGA,    REGISTER_rsi) // replace esi    by rsi
						
						else if (e("eax"))    ADD_CURRENT(TOKEN_REGA,    REGISTER_rax)  // replace eax    by rax
						else if (e("edx"))    ADD_CURRENT(TOKEN_REGA,    REGISTER_rdx)  // replace edx    by rdx
						else if (e("ecx"))    ADD_CURRENT(TOKEN_REGA,    REGISTER_rcx)  // replace ecx    by rcx
						else if (e("ebx"))    ADD_CURRENT(TOKEN_REGA,    REGISTER_rbx)  // replace ebx    by rbx
						
						else if (e("movsxd")) ADD_CURRENT(TOKEN_KEYWORD, TOKEN_movsx) // replace movsxd by movsx
						else if (e("movsd"))  ADD_CURRENT(TOKEN_KEYWORD, TOKEN_mov)  // replace movsd  by mov
						else if (e("addsd"))  ADD_CURRENT(TOKEN_KEYWORD, TOKEN_add)  // replace addsd  by add
						
						else if (e("r8"))     ADD_CURRENT(TOKEN_REGA,    36) // replace r8     by r8d
						else if (e("r9d"))    ADD_CURRENT(TOKEN_REGA,    43) // replace r9d    by r9
						else if (e("r13d"))   ADD_CURRENT(TOKEN_REGA,    40) // replace r13d   by r13
						else if (e("r15d"))   ADD_CURRENT(TOKEN_REGA,    42) // replace r15d   by r15
						
						else if (e("r10d"))   ADD_CURRENT(TOKEN_REGA,    REGISTER_r10)
						else if (e("r11d"))   ADD_CURRENT(TOKEN_REGA,    REGISTER_r11)
						else if (e("r12d"))   ADD_CURRENT(TOKEN_REGA,    REGISTER_r12)
						else if (e("r14d"))   ADD_CURRENT(TOKEN_REGA,    REGISTER_r14)
						
						else if (e("psh"))    ADD_CURRENT(TOKEN_KEYWORD, TOKEN_push)  // replace psh    by push
						else if (e("cal"))    ADD_CURRENT(TOKEN_KEYWORD, TOKEN_call)  // replace cal    by call
						
						else if (e("imul")||e("mul")) { // set sf to tell mul to do a signed multiplication
							ADD_CURRENT(TOKEN_KEYWORD,  TOKEN_mov) // mov sf, e("imul")
							ADD_CURRENT(TOKEN_REGISTER, REGISTER_sf)
							ADD_CURRENT(TOKEN_BYTE,     e("imul"))
							
							ADD_PREVIOUS_INSTRUCTION
							ADD_CURRENT(TOKEN_KEYWORD, TOKEN_mul)
							
						} else if (e("idiv")||e("div")) { // set sf to tell div to do a signed division
							ADD_CURRENT(TOKEN_KEYWORD,  TOKEN_mov) // mov sf, e("idiv")
							ADD_CURRENT(TOKEN_REGISTER, REGISTER_sf)
							ADD_CURRENT(TOKEN_BYTE,     e("idiv"))
							
							ADD_PREVIOUS_INSTRUCTION
							ADD_CURRENT(TOKEN_KEYWORD, TOKEN_div)
							
						}
						
						#if VERSION >= 3
							else if (e("sal"))    ADD_CURRENT(TOKEN_KEYWORD, TOKEN_shl)    // replace sal     by shl
							
							else if (e("inc")) { // replace inc by add
								ADD_CURRENT(TOKEN_KEYWORD, TOKEN_add)
								inc_dec_instruction = 1;
								
							} else if (e("dec")) { // replace dec by sub
								ADD_CURRENT(TOKEN_KEYWORD, TOKEN_sub)
								inc_dec_instruction = 1;
								
							} else if (e("test")||e("tst")) {
								ADD_CURRENT(TOKEN_KEYWORD, TOKEN_cmp)
								test_instruction = 1;
								
							} else if (e("cqo")) {
								ADD_CURRENT(TOKEN_KEYWORD, TOKEN_mov); // mov rdx, rax
								ADD_CURRENT(TOKEN_REGA,    REGISTER_rdx);
								ADD_CURRENT(TOKEN_REGA,    REGISTER_rax);
								
								ADD_PREVIOUS_INSTRUCTION
								
								ADD_CURRENT(TOKEN_KEYWORD, TOKEN_and); // and rdx, 0x8000000000000000
								ADD_CURRENT(TOKEN_REGA,    REGISTER_rdx);
								ADD_CURRENT(TOKEN_NUMBER,  0x8000000000000000);
								
								ADD_PREVIOUS_INSTRUCTION
								
								ADD_CURRENT(TOKEN_KEYWORD, TOKEN_sar); // sar rdx, 63
								ADD_CURRENT(TOKEN_REGA,    REGISTER_rdx);
								ADD_CURRENT(TOKEN_BYTE,    63);
								
							} else if (e("cdqe")||e("cdq")) { /* Don't do anything; instruction deprecated */ }
						#else
							else if (e("cdqe"))   ADD_CURRENT(TOKEN_KEYWORD, TOKEN_cdq)  // replace cdqe by cdq
							else if (e("tst"))    ADD_CURRENT(TOKEN_KEYWORD, TOKEN_test) // replace tst  by test
						#endif
						
						else if (e("leave")||e("lev")) {
							ADD_CURRENT(TOKEN_KEYWORD, TOKEN_mov)  // mov rsp, rbp
							ADD_CURRENT(TOKEN_REGA,    REGISTER_rsp)
							ADD_CURRENT(TOKEN_REGA,    REGISTER_rbp)
							
							ADD_PREVIOUS_INSTRUCTION
							
							ADD_CURRENT(TOKEN_KEYWORD, TOKEN_pop)  // pop rbp
							ADD_CURRENT(TOKEN_REGA,    REGISTER_rbp)
							
						} else if (c(0, 'j')) { /// NOTE THE TWO FOLLOWING CONDITIONS *MUST* STAY AT THE END
							TEST_CONDITION(1);
							
							if (condition_evaluated) {
								this->token_buffer[1] = 'm';
								this->token_buffer[2] = 'p';
								this->token_buffer[3] = 0;
								
							}
							
						} else if (c(0,'c')&&c(1,'m')&&c(2,'o')&&c(3,'v')) {
							TEST_CONDITION(4);
							
							if (condition_evaluated) {
								this->token_buffer[0] = 'm';
								this->token_buffer[1] = 'o';
								this->token_buffer[2] = 'v';
								this->token_buffer[3] = 0;
								
							}
							
						} else if (c(0,'s')&&c(1,'e')&&c(2,'t')) {
							TEST_CONDITION(3);
							
							if (condition_evaluated) {
								this->token_buffer[3] = 0;
							}
							
						} else {
							next = 1;
							
						}
						
						// search through the database
						
						if (next) {
							if (address_type) {
								next = 0;
								
							}
							
							var i;
							for (i = 0; i < len_registers; i++) {
								if (tokeneq(this->token_buffer, registers[i].name)) { // found register
									ADD_CURRENT(TOKEN_REGA, i)
									next = 0;
									break;
									
								}
								
							} if (next) {
								for (i = 0; i < len_keywords; i++) {
									if (tokeneq(this->token_buffer, keywords[i].name)) { // found instruction
										ADD_CURRENT(TOKEN_KEYWORD, i)
										next = 0;
										break;
										
									}
									
								} if (next) {
									for (i = 0; i < len_base_reserved; i++) {
										if (tokeneq(this->token_buffer, base_reserved[i].name)) { // found base_reserved function
											var j;
											for (j = 0; j < len_dangerous; j++) {
												if (tokeneq(this->token_buffer, dangerous[j].name)) { // found dangerous function
													this->warning++;
													print("WARNING Found dangerous function: ");
													print_token(this->token_buffer);
													print("\n");
													
												}
												
											}
											
											ADD_CURRENT(TOKEN_PRERESERVED, i)
											next = 0;
											break;
											
										}
										
									} if (next) {
										for (i = 0; i < this->reserved_pointer; i++) {
											if (tokeneqtoken(this->token_buffer, this->reserved[i])) { // found reserved
												var __base_reserved_len   = get_rom_value(this, this->base_reserved_count);
												var label_position_offset = get_rom_value(this, this->label_position_offset);
												
												var temp = __base_reserved_len + i;
												var type = TOKEN_RESERVED;
												
												/// TODO qtypes
												//~ if (i in this->qtypes) type = TOKEN_QTYPE;
												/*else*/ if (temp < label_position_offset) type = TOKEN_PRERESERVED;
												
												ADD_CURRENT(type, type == TOKEN_RESERVED ? temp : temp);
												next = 0;
												break;
												
											}
											
										} if (next) {
											this->warning++;
											print("WARNING Found new, unknown token: ");
											print_token(this->token_buffer);
											print("\n");
											
											#if HINT
												if      (tokeneq(this->token_buffer, "dword")) print("HINT You may have used \"int\" instead of \"var\" or used some kind of data structure whose size is not a multiple of 8 bytes\n");
												else if (tokeneq(this->token_buffer, "word"))  print("HINT You may have used \"short\" instead of \"var\" or used some kind of data structure whose size is not a multiple of 8 bytes\n");
											#endif
											
										}
										
									}
									
								}
								
							}
							
						}
						
						if (address_type) {
							this->token_buffer--;
							
						}
						
					}
					
				} else if (has_number) { // number (after db or dq, probably)
					if (this->section == SECTION_DATA) {
						if (!get_rom_value(this, this->data_section_element_qtype)) {
							if (number > 0xFF) {
								this->warning++;
								print("WARNING number (0x%llx) is bigger than maximum byte value (0xFF)\n", number);
								
							}
							
							var value = number % 0x100;
							set_rom_value(this, this->data_section_element_element_count, get_rom_value(this, this->data_section_element_element_count) + sizeof(char));
							
							add_rom_byte(this, value);
							
							//~ var offset = this->data_section_offset++ % 8;
							//~ this->data_section_pack += value << (offset * 8);
							
							//~ if (offset == 7) {
								//~ add_rom(this, this->data_section_pack);
								
							//~ }
							
						} else {
							var  label = -1;
							uvar offset;
							
							var pointer = 0;
							while (this->token_buffer[pointer]) {
								if (this->token_buffer[pointer] == '+') {
									this->token_buffer[pointer] = 0;
									
									var i;
									for (i = 0; i < this->data_reserved_pointer; i++) {
										if (tokeneqtoken(this->token_buffer, this->data_reserved[i])) {
											label = i;
											break;
											
										}
										
									}
									
									if (label < 0) {
										label = 0;
										this->warning++;
										print("WARNING The first operand of qtype is not data label. Setting label to 0 ...\n");
										break;
										
									} else {
										if (!parse_number(this->token_buffer + pointer + 1, tokenlen(this->token_buffer + pointer + 1), &offset)) {
											offset = 0;
											this->warning++;
											print("WARNING The second operand of qtype is not number. Setting offset to 0 ...\n");
											
										}
										
										break;
										
									}
									
								}
								
								pointer++;
								
							}
							
							if (offset > 0xFFFFFFFF) {
								this->warning++;
								print("WARNING qtype offset (0x%llx) is bigger than maximum dword value (0xFFFFFFFF). Truncating to the last 32 bits (0x%llx) ...\n\n", offset, offset & 0xFFFFFFFF);
								
							}
							
							offset %= 0xFFFFFFFF;
							set_rom_value(this, this->data_section_element_element_count, get_rom_value(this, this->data_section_element_element_count) + sizeof(uvar));
							
							if (label > 0xFFFFFFFF) {
								this->warning++;
								print("WARNING qtype label (0x%llx) is bigger than maximum dword value (0xFFFFFFFF). Truncating to the last 32 bits (0x%llx) ...\n", label, label & 0xFFFFFFFF);
								
							}
							
							uvar qtype = label << 32;
							add_rom(this, qtype + (offset & 0xFFFFFFFF));
							
							/*
							new_jump = (instruction.data[1], reserved.index(instruction.data[0]) - len(base_reserved))
							
							new_data_section_text.append(new_jump[0] & 0x000000FF)
							new_data_section_text.append((new_jump[0] & 0x0000FF00) >> 8)
							new_data_section_text.append((new_jump[0] & 0x00FF0000) >> 16)
							new_data_section_text.append((new_jump[0] & 0xFF000000) >> 24)
							
							new_data_section_text.append(new_jump[1] & 0x000000FF)
							new_data_section_text.append((new_jump[1] & 0x0000FF00) >> 8)
							new_data_section_text.append((new_jump[1] & 0x00FF0000) >> 16)
							new_data_section_text.append((new_jump[1] & 0xFF000000) >> 24)
							*/
							
						}
						
					} else {
						print("DATA NUMBER %llx\n", number);
						
					}
					
				} else if (e("db")) { // db token
					if (this->section == SECTION_DATA) {
						this->data_section_pack   = 0;
						this->data_section_offset = 0;
						
					} else {
						print("Found `data bytes` token\n");
						
					}
					
				} else if (e("dq")) { // dq token
					if (this->section == SECTION_DATA) {
						set_rom_value(this, this->data_section_element_qtype, 1);
						
						this->data_section_pack   = 0;
						this->data_section_offset = 0;
						
					} else {
						print("Found `data qwords` token\n");
						
					}
					
				} else if (this->token_buffer[0] != '\r') { // could not match the token
					this->warning++;
					print("WARNING Found unknown token %lld: ", this->token_buffer[0]);
					print_token(this->token_buffer);
					print("\n");
					
				}
				
			}
			
			reset_token_buffer(this);
			
		} else { // append to token
			if (this->token_buffer_pointer >= MAX_TOKEN_LENGTH) {
				print("FATAL ERROR Current token has exceeded MAX_TOKEN_LENGTH (%lld). Aborting ...\n", (var) MAX_TOKEN_LENGTH);
				this->error++;
				break;
				
			}
			
			this->token_buffer[this->token_buffer_pointer++] = current;
			this->token_buffer[this->token_buffer_pointer]   = 0;
			
		}
		
	}
	
	print("\nFinished parsing everything\n");
	
	print("this->rom_length = %lld\n", this->rom_length);
	set_rom_value(this, this->length,  this->rom_length);
	
	add_rom(this, 0x44455A2D41555141); // signature (also add what was left of rom_add_byte) NOTE is not included with this->rom_length
	
	// free reserved / data reserved
	
	var j;
	for (j = 0; j < this->reserved_pointer; j++) {
		mfree(this->reserved[j], this->reserved_lengths[j] * sizeof(var));
		
	}
	
	mfree(this->reserved_positions_markers, this->reserved_positions_count * sizeof(var));
	
	for (j = 0; j < this->data_reserved_pointer; j++) {
		mfree(this->data_reserved[j], this->data_reserved_lengths[j] * sizeof(var));
		
	}
	
	free_asm(this);
	print("Compiler freed with %lld errors and %lld warnings\n", this->error, this->warning);
	
	print("Writing ROM.zed ...\n");
	write_rom(this);
	print("Done\n");
	
	return this->error;
	
}

