section text align=1 execute
?_amber_f0:
mov rax 0
ret
mov rdi ?_amber_d0
call print
mov [rsp-8] rax
ret
main:
mov [rsp-0] ?_amber_f0
mov [rsp-24] 8
mov [rsp-16] [rsp-24]
mov [rsp-32] 2
mov rax [rsp-16]
add rax [rsp-32]
mov [rsp-40] rax
mov rdi [rsp-40]
call print
mov [rsp-48] rax
mov rdi 
call [rsp-0]
mov [rsp-56] rax
mov [rsp-64] 1337
mov rax [rsp-64]
ret
section data align=1
?_amber_d0: db 48H 65H 6cH 6cH 6fH 20H 77H 6fH 72H 6cH 64H 21H 5cH 6eH 0
