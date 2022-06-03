global put_value_in_separate_pt

section .text
bits 64

put_value_in_separate_pt:
	mov [temp], r12
	mov r12, cr3
	mov cr3, rdx
	mov [rdi], rsi
	mov cr3, r12
	mov r12, [temp]
	ret

section .data
temp:
	dq 0
;put_value_in_separate_pt(destination + i, source[i], page_table_base);
