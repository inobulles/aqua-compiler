
static token_t assembler_instructions[] = { // TODO make the assembler automatically translate jmp, ret, psh, and pop tokens (and also add second function to "cal" token if not calling kfunc)
	{"cad"}, {"mov"},
	{"cnd"}, {"cmp"},
	{"jmp"}, {"cal"},
	{"add"}, {"sub"}, {"mul"}, {"div"},
	{"and"}, {"or" }, {"xor"}, {"not"},
	{"shl"}, {"shr"},
};

static token_t assembler_registers[] = {
	{"ip"}, {"sp"}, {"bp"}, {"ad"},
	{"g0"}, {"g1"}, {"g2"}, {"g3"},
	{"a0"}, {"a1"}, {"a2"}, {"a3"},
	{"sf"}, {"zf"}, {"of"}, {"cf"},
};

static token_t assembler_kfuncs[] = {
	{"noop"},           {"print"},            {"exit"},
	{"malloc"},         {"mfree"},            {"mcpy"},          {"mset"},
	{"get_platform"},   {"platform_command"}, {"create_device"}, {"send_device"}, {"native"},
	{"get_events"},
};
