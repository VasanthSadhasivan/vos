global virtualization_start
extern kernel_virtualized_main
section .text
bits 64
virtualization_start:
	mov cr3, rdi
	mov rsp, 0xffffffffff
	mov rbp, 0xffffffffff
	call kernel_virtualized_main
	
	hlt
