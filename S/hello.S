.section .data
message:
	.ascii "Hello, World!\n"
	
.section .text

.global _start
_start:
	movq $1, %rax
	movq $1, %rdi
	movq $message, %rsi
	movq $14, %rdx
	syscall

	movq $60, %rax
	xorq %rdi, %rdi
	syscall
