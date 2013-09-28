#; again, I will bring in write for this example :)

.data
i:
	.string " "

.text
.global _start
_start:
	movl $3, %eax   #; eax = 3 (read)
	movl $0, %ebx   #; ebx = 0 (stdin)
	movl $i, %ecx   #; ecx = i
	movl $1, %edx   #; edx = 1 (how many chars to read) (man 2 read)
	int $0x80       #; call interrupt 80

	movl $4, %eax   #; eax = 4 (write)
	movl $1, %ebx   #; ebx = 1 (stdout)
	movl $i, %ecx   #; ecx = i
	movl $1, %edx   #; edx = 1 (how many chars to write ;-))
	int $0x80

	movl i, %eax     #; move contents of i, into eax
	cmp $'\n', %eax  #; compare our input to \n
	jz done          #; jump if zf
	jmp _start       #; loop

done:
	movl $1, %eax    #; eax = 1 (exit)
	movl $0, %ebx    #; ebx = 0 (exit code)
	int $0x80        #; call interrupt 80

	ret
