section text align=1 execute
main:
mov [rsp-8] 8
mov [rsp-0] [rsp-8]
mov [rsp-16] 2
mov rax [rsp-0]
add rax [rsp-16]
mov [rsp-24] rax
mov rdi [rsp-24]
call print
mov [rsp-32] rax
section data align=1
