section text align=1 execute
?_amber_s1:
mov rdi ?_amber_d1
call print
mov [rsp-56] rax
ret
?_amber_s0:
mov [rsp-32] 5
mov [rsp-40] 5
mov rax [rsp-32]
sub rax [rsp-40]
mov [rsp-48] rax
cmp [rsp-48] 1
cnd z
call ?_amber_s1
mov rdi ?_amber_d2
call print
mov [rsp-64] rax
ret
main:
mov rdi ?_amber_d0
call print
mov [rsp-0] rax
mov [rsp-8] 5
mov [rsp-16] 5
mov rax [rsp-8]
add rax [rsp-16]
mov [rsp-24] rax
cmp [rsp-24] 1
cnd z
call ?_amber_s0
section data align=1
?_amber_d0: db 4fH 72H 69H 67H 69H 6eH 61H 6cH 5cH 6eH 0
?_amber_d1: db 74H 65H 73H 74H 5cH 6eH 0
?_amber_d2: db 48H 65H 6cH 6cH 6fH 20H 77H 6fH 72H 6cH 64H 21H 5cH 6eH 0
