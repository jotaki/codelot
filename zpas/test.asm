; not a valid executable.. :p

mov al, 0x0f
mov ah, 0x0f
mov ax, 0x0f
mov eax, 0xab43f2e7

aad 0x10
aam 0x10
aas

clc
cld
cli
clts
cmc
cpuid
