#; well, here it is the example of write..this will actually be easier than
#; the last two examples -sighs-

.data
hello:
	.string "Hello, from the int 80 reference by example! :)\n"
	len = . - hello     #; incase your wondering, this gets the length of the
	                    #; string.
.text
.global _start
_start:
	movl $4, %eax       #; eax = 4 (write)
	movl $1, %ebx       #; ebx = 1 (stdout)
	movl $hello, %ecx   #; ecx = hello
	movl $len, %edx     #; edx = len (strlen(hello))
	int $0x80           #; call interrupt 80

	movl $1, %eax       #; eax = 1 (exit)
	movl $0, %ebx       #; ebx = 0 (exit code)
	int $0x80           #; call interrupt 80

	ret
