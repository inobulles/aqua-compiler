 
#ifndef __AQUA__DECODER_DEFS_H
	#define __AQUA__DECODER_DEFS_H
	
	// registers
	
	#define REGISTER_addr 0 // address register
	#define REGISTER_rip 1 // instruction pointer register
	
	#define REGISTER_rsp 2 // stack pointer registers
	#define REGISTER_rbp 3
	
	#define REGISTER_rax 4 // functional registers
	#define REGISTER_rcx 5
	#define REGISTER_rdx 6
	#define REGISTER_rbx 7
	
	#define REGISTER_r8 8 // general purpose registers
	#define REGISTER_r9 9
	#define REGISTER_r10 10
	#define REGISTER_r11 11
	#define REGISTER_r12 12
	#define REGISTER_r13 13
	#define REGISTER_r14 14
	#define REGISTER_r15 15
	
	#define REGISTER_rsi 16 // argument registers
	#define REGISTER_rdi 17
	
	#define REGISTER_cf 18 // flag registers
	#define REGISTER_of 19
	#define REGISTER_zf 20
	#define REGISTER_sf 21
	#define REGISTER_pf 22
	#define REGISTER_af 23
	
	#define REGISTER_LAST REGISTER_af
	
	// instructions
	
	#define TOKEN_mov 0 // move instructions
	#define TOKEN_set 39
	#define TOKEN_movsx 22
	#define TOKEN_movzx 38
	
	#define TOKEN_push 4 // stack instructions
	#define TOKEN_pop 5
	
	#define TOKEN_add 6 // arithmetic instructions
	#define TOKEN_sub 7
	
	#define TOKEN_and 11 // bitwise instructions
	#define TOKEN_or 12
	#define TOKEN_xor 13
	#define TOKEN_shl 17
	#define TOKEN_shr 18
	#define TOKEN_sar 26
	#define TOKEN_neg 27
	#define TOKEN_not 34
	
	#define TOKEN_jmp 2 // instruction pointer modifying instructions
	#define TOKEN_call 3
	#define TOKEN_ret 1
	
	#define TOKEN_mul 8 // multiplication and division instructions
	#define TOKEN_div 9
	#define TOKEN_mul3 29
	#define TOKEN_div3 30
	#define TOKEN_mul1 31
	#define TOKEN_div1 32
	#define TOKEN_mod 10 /// MAYBE DEPRECATED
	
	#define TOKEN_cnd 37 // condition instructions
	#define TOKEN_cmp 19
	
	#define TOKEN_lea 16 // memory instructions
	#define TOKEN_cla 28
	
	// conditions
	
	#define CONDITION_Z 0
	#define CONDITION_NZ 1
	#define CONDITION_S 2
	#define CONDITION_NS 3
	#define CONDITION_E 4
	#define CONDITION_NE 5
	#define CONDITION_LT 6
	#define CONDITION_GT 7
	#define CONDITION_LE 8
	#define CONDITION_GE 9
	#define CONDITION_C 10
	#define CONDITION_NC 11
	
	// irrelevent
	
	#define TOKEN_END 255
	#define TOKEN_DATA_END 256
	#define TOKEN_DATA_SECTION_END 257
	#define TOKEN_RESERVED_POSITIONS_END -1
	
	#ifndef TOKEN_KEYWORD
		#define TOKEN_KEYWORD     1
		#define TOKEN_REGISTER    2
		#define TOKEN_RESERVED    3
		#define TOKEN_NUMBER      4
		#define TOKEN_ADDRESS     6
		#define TOKEN_PRERESERVED 9
		#define TOKEN_QTYPE       12
		#define TOKEN_BYTE_ADDR   13
	#endif
	
#endif
