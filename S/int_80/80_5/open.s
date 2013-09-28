#; I can't introduce open w/ out intoducing close, so :)

.data

#; general data
filename:
	.string "./int80_5\0"   #; needs to be nul-terminated of course :)

write_str:
	.string "Hello, from the int 80 reference by example :)\n"
	ws_len = .- write_str

fd:
	.long 0

r:
	.long 0

#; error data
err_ccreat:
	.string "Cannot create file\n"
	ecc_len = . - err_ccreat

.text
.global _start
_start:
	movl $5, %eax           #; eax = 5 (open)
	movl $filename, %ebx    #; ebx = filename
	movl $1, %ecx           #; ecx = 1 (O_WRONLY)
	orl $0100, %ecx         #; ecx |= 0100 (O_CREAT)
	movl $0644, %edx        #; edx = 0644 (-rw-r--r--)
	int $0x80               #; call interrupt 80

	cmp $-1, %eax           #; if(eax == -1)
	jz err_cc               #;   goto err_cc

	movl %eax, fd           #; fd = eax

	movl $0x05, %edi        #; edi = 5
.write:
	movl $4, %eax           #; eax = 4 (write)
	movl fd, %ebx           #; ebx = fd
	movl $write_str, %ecx   #; ecx = write_str
	movl $ws_len, %edx      #; edx = strlen(write_str)
	decl %edx               #; edx -= 1 (take away the ^@)
	int $0x80               #; call interrupt 80
	decl %edi               #; edi -= 1
	loopnz .write           #; if(edi != 0) goto .write;

	movl $6, %eax           #; eax = 6 (close)
	movl fd, %ebx           #; ebx = fd
	int $0x80               #; call interrupt 80

	xorl %eax, %eax         #; eax = 0
	pushl %eax              #; push 0
	jmp exit                #; jump to exit

exit:                       #; exit :)
	movl $1, %eax           #; eax = 1 (exit)
	popl %ebx               #; ebx = exit code
	int $0x80               #; call interrupt 80
	ret

err_cc:                     #; err_cc :)
	movl $4, %eax           #; eax = 4 (write)
	movl $1, %ebx           #; ebx = 1 (stdout)
	movl $err_ccreat, %ecx  #; ecx = err_ccreat
	movl $ecc_len, %edx     #; edx = strlen(err_ccreat)
	int $0x80               #; call interrupt 80

	movl $1, %eax           #; eax = 1
	pushl %eax              #; push 1
	jmp exit                #; goto exit
