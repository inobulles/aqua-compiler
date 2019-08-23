
#define TOKEN_INSTRUCTION 0
#define TOKEN_REGISTER    1

#define TOKEN_RESERVED    2
#define TOKEN_RES_POS     3

#define TOKEN_ADDRESS     4
#define TOKEN_BYTE_ADDR   5

#define TOKEN_NUMBER      6
#define TOKEN_BYTE        7

static token_t assembler_instructions[] = {
	{"cla"}, {"mov"},
	{"cnd"}, {"cmp"},
	{"jmp"}, {"cal"}, {"ret"},
	{"psh"}, {"pop"},
	{"add"}, {"sub"}, {"mul"}, {"div"},
	{"and"}, {"or" }, {"xor"}, {"not"},
	{"shl"}, {"shr"}, {"ror"},
};

static token_t assembler_registers[] = {
	{"ip"}, {"sp"}, {"bp"}, {"ad"},
	{"g0"}, {"g1"}, {"g2"}, {"g3"},
	{"a0"}, {"a1"}, {"a2"}, {"a3"},
	{"sf"}, {"zf"}, {"of"}, {"cf"},
};

static token_t assembler_prereserved[] = {
	{"noop"},           {"print"},            {"exit"},
	{"malloc"},         {"mfree"},            {"mcpy"},          {"mset"},
	{"get_platform"},   {"platform_command"}, {"create_device"}, {"send_device"},         {"native"},
	{"video_width"},    {"video_height"},     {"video_fps"},     {"video_clear"},         {"video_flip"},
	{"create_machine"}, {"execute_machine"},  {"kill_machine"},  {"give_machine_events"}, {"current_machine"},
	{"get_events"},
};
