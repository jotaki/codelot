/*
 * file: mem.c
 * author: Joey Kinsella
 * date: 2010-04-28
 * description:
 *	tinker around with memory management.
 */

#include <stdio.h>
#include <unistd.h>

int main()
{
  unsigned long eax;

  __asm__ __volatile__ ("movl %%esp, %%eax":"=a"(eax));
  printf("%p\n", (void *) eax);
  __asm__ __volatile__ ("jmp $4");
  _exit(1);
  
	
	printf("Hello, World!\n");
	return (0);
}
