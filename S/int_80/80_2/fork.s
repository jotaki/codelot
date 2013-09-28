#; I'm introducing interrupt 80, 4 in here.. but don't worry about it, it's
#; only for the example, we'll cover it soon.

.data
wap:	#; we are parent
	.string "Hello from the parent\n"
	wapl = . - wap
wac:	#; we are child
	.string "Hello from the child\n"
	wacl = . - wac

.text
.global _start
_start:
	movl $2, %eax   #; eax = 2 (fork)
	int $0x80       #; call interrupt 80 :)

	cmp $0, %eax     #; check if we're the parent
	jz parent        #; jump if zf

	movl $4, %eax    #; eax = 4 (write)
	movl $1, %ebx    #; ebx = 1 (stdout)
	movl $wap, %ecx  #; ecx = wap
	movl $wapl, %edx #; edx = wapl (strlen(wap))
	int $0x80        #; call interrupt 80
	jmp quit         #; jump to quit

parent:
	movl $4, %eax    #; eax = 4 (write)
	movl $1, %ebx    #; ebx = 1 (stdout)
	movl $wac, %ecx  #; ecx = wac
	movl $wacl, %edx #; edx = wacl (strlen(wac))
	int $0x80        #; call intterupt 80

quit:
	movl $1, %eax    #; eax = 1 (exit)
	movl $0, %ebx    #; ebx = 0 (exit code)
	int $0x80        #; call interrupt 80

	ret
