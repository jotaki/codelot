[BITS 16]
[ORG 0x7C00]

	xor ax, ax
	mov ds, ax
	mov si, message

	mov ah, 0x0e
	xor bh, bh
	mov bl, 0x07

nextc
	lodsb

	or al, al
	jz done

	int 0x10
	jmp nextc
done
	jmp $

message db "No boots here.",10,0
