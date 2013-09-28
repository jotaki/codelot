#; << The reason for the semi-colon, is because vim doesn't look at # as a
#; comment in assembly, and I like pretty-syntax highlighting, so :)

.text
.global _start
_start:
	movl $1, %eax   #; eax = 1 (exit)
	movl $0, %ebx   #; ebx = 0 (exit code) (man 3 exit)
	int $0x80       #; call interrupt 0x80

	ret
