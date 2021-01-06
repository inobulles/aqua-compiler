#if !defined(__AQUA_COMPILER__COMPILER_COMMON_H)
#define __AQUA_COMPILER__COMPILER_COMMON_H

#include <stdint.h>

#define UNIQUE_PATH "unique"
#define ROM_PATH "rom.zed"

#define META_NAME_PATH "meta/name"
#define META_AUTHOR_PATH "meta/author"
#define META_VERSION_PATH "meta/version"
#define META_ORGANIZATION_PATH "meta/organization"
#define META_DESCRIPTION_PATH "meta/description"

#define ZED_MAGIC 0x7A647A647A647A64
#define ZED_VERSION 5

// note that these following macros aren't really here to make changing the ZED standard easier
// rather, they're for improving readability inside of dependant source files
// so it's really not a big deal if you don't use them

#define ZED_INSTRUCTION_COUNT 16
#define ZED_REGISTER_COUNT 16

typedef struct {
	uint64_t magic, version;
	uint64_t data_section_offset, data_section_element_count;
	uint64_t position_section_offset, position_section_element_count;
	uint64_t logic_section_offset, logic_section_words;
} zed_meta_section_t;

typedef struct {
	uint64_t bytes;
	uint64_t data_offset;
} zed_data_section_element_t;

typedef struct {
	unsigned operation       : 4;

	unsigned operand1_64_bit : 1; // TODO update assembler with this and also addresses being 16 bit
	unsigned operand1_type   : 3;

	unsigned operand2_64_bit : 1;
	unsigned operand2_type   : 3;

	unsigned operand3_64_bit : 1;
	unsigned operand3_type   : 3;

	unsigned operand1_data   : 16;
	unsigned operand2_data   : 16;
	unsigned operand3_data   : 16;
} zed_instruction_t;

#define ZED_OPERAND_16_TYPE_NONE           (0b0000 | 0)
#define ZED_OPERAND_16_TYPE_REGISTER       (0b0000 | 1)
#define ZED_OPERAND_16_TYPE_CONSTANT       (0b0000 | 2)
#define ZED_OPERAND_16_TYPE_ADDRESS_64     (0b0000 | 3)
#define ZED_OPERAND_16_TYPE_ADDRESS_8      (0b0000 | 4)
#define ZED_OPERAND_16_TYPE_POSITION_INDEX (0b0000 | 5)
#define ZED_OPERAND_16_TYPE_KFUNC_INDEX    (0b0000 | 6)
#define ZED_OPERAND_16_TYPE_DATA_INDEX     (0b0000 | 7)

#define ZED_OPERAND_64_TYPE_CONSTANT       (0b1000 | 2)
#define ZED_OPERAND_64_TYPE_POSITION_INDEX (0b1000 | 5)
#define ZED_OPERAND_64_TYPE_DATA_INDEX     (0b1000 | 7)

#endif