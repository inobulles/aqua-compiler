
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

#ifndef __AQUA__COMPILER_DATABASE_H
	#define __AQUA__COMPILER_DATABASE_H
	
	typedef struct {
		const char* name;
		
	} token_t;
	
	#define    len_conditions 12
	static token_t conditions[len_conditions] = {
		{ .name = "z" },
		{ .name = "nz" },
		{ .name = "s" },
		{ .name = "ns" },
		{ .name = "e" },
		{ .name = "ne" },
		{ .name = "lt" },
		{ .name = "gt" },
		{ .name = "le" },
		{ .name = "ge" },
		{ .name = "c" },
		{ .name = "nc" },
	};
	
	#define    len_dangerous 1
	static token_t dangerous[len_dangerous] = {
		{ .name = "system" },
	};
	
	#define    len_base_reserved 26
	static token_t base_reserved[len_base_reserved] = {
		{ .name = "print" },
		{ .name = "debug" },
		{ .name = "__exit" },
		{ .name = "__stack_chk_fail" },
		
		{ .name = "malloc" },
		{ .name = "mfree" },
		{ .name = "memcpy" },
		{ .name = "memset" },
		
		{ .name = "get_platform" },
		{ .name = "platform_system" },
		{ .name = "create_device" },
		{ .name = "send_device" },
		{ .name = "native" },
		
		{ .name = "video_width" },
		{ .name = "video_height" },
		{ .name = "video_fps" },
		{ .name = "video_clear" },
		{ .name = "video_flip" },
		
		{ .name = "create_machine" },
		{ .name = "execute_machine" },
		{ .name = "kill_machine" },
		{ .name = "give_machine_events" },
		{ .name = "current_machine" },
		
		{ .name = "break_point" },
		{ .name = "get_events" },
		{ .name = "sprint" },
	};
	
	#define    len_registers 52
	static token_t registers[len_registers] = {
		{ .name = "rax" },
		{ .name = "rcx" },
		{ .name = "rdx" },
		{ .name = "rbx" },
		{ .name = "eax" },
		{ .name = "ecx" },
		{ .name = "edx" },
		{ .name = "ebx" },
		{ .name = "ax" },
		{ .name = "cx" },
		{ .name = "dx" },
		{ .name = "bx" },
		{ .name = "al" },
		{ .name = "cl" },
		{ .name = "dl" },
		{ .name = "bl" },
		{ .name = "sp" },
		{ .name = "bp" },
		{ .name = "si" },
		{ .name = "di" },
		{ .name = "esi" },
		{ .name = "edi" },
		{ .name = "ebp" },
		{ .name = "esp" },
		{ .name = "rsp" },
		{ .name = "rbp" },
		{ .name = "rsi" },
		{ .name = "rdi" },
		{ .name = "ss" },
		{ .name = "cs" },
		{ .name = "ds" },
		{ .name = "es" },
		{ .name = "fs" },
		{ .name = "gs" },
		{ .name = "eip" },
		{ .name = "rip" },
		{ .name = "r8d" },
		{ .name = "r8" },
		{ .name = "addr" },
		{ .name = "r12" },
		{ .name = "r13" },
		{ .name = "r14" },
		{ .name = "r15" },
		{ .name = "r9" },
		{ .name = "r10" },
		{ .name = "r11" },
		{ .name = "cf" },
		{ .name = "of" },
		{ .name = "zf" },
		{ .name = "sf" },
		{ .name = "pf" },
		{ .name = "af" },
	};
	
	#define    len_keywords 40
	static token_t keywords[len_keywords] = {
		{ .name = "mov" },
		{ .name = "ret" },
		{ .name = "jmp" },
		{ .name = "call" },
		{ .name = "push" },
		{ .name = "pop" },
		{ .name = "add" },
		{ .name = "sub" },
		{ .name = "mul" },
		{ .name = "div" },
		{ .name = "mod" },
		{ .name = "and" },
		{ .name = "or" },
		{ .name = "xor" },
		{ .name = "inc" },
		{ .name = "dec" },
		{ .name = "lea" },
		{ .name = "shl" },
		{ .name = "shr" },
		{ .name = "cmp" },
		{ .name = "imul" },
		{ .name = "idiv" },
		{ .name = "movsx" },
		{ .name = "cdq" },
		{ .name = "test" },
		{ .name = "sal" },
		{ .name = "sar" },
		{ .name = "neg" },
		{ .name = "cla" },
		{ .name = "mul3" },
		{ .name = "div3" },
		{ .name = "mul1" },
		{ .name = "div1" },
		{ .name = "cqo" },
		{ .name = "not" },
		{ .name = "rep" },
		{ .name = "movsq" },
		{ .name = "cnd" },
		{ .name = "movzx" },
		{ .name = "set" },
	};
	
#endif
