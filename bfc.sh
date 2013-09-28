#! /bin/bash
#
# Brainfuck compiler
# Joseph Kinsella and Vegard Nossum
#

file=`mktemp`
(echo '# start'
echo .globl _start
echo .section .text
echo _start:
echo movl \$.mem, %esi

sp=0
or=0
ir=0

for opc in `cat $1 | perl -e 'while(chomp($_ = <STDIN>)) { for my $c (split m//, $_) { print "$c\n" if $c =~ m/[.,\[\]+\-<>]/}}'`;
do
	if [ $opc == '.' ];
	then
		echo '# .'
		echo .out_retry_${or}_${sp}:
		echo movl \$4, %eax
		echo movl \$1, %ebx
		echo movl %esi, %ecx
		echo movl \$1, %edx
		echo int \$0x80

		echo cmpl \$0, %eax
		echo je .out_retry_${or}_${sp}
		echo

		or=$[$or + 1];
	elif [ $opc == ',' ];
	then
		echo '# ,'
		echo .in_retry_${ir}_${sp}:
		echo movl \$3, %eax
		echo movl \$0, %ebx
		echo movl %esi, %ecx
		echo movl \$1, %edx
		echo int \$0x80

		echo cmpl \$0, %eax
		echo je .in_retry_${ir}_${sp}
		echo

		ir=[$ir + 1];
	elif [ $opc == '[' ];
	then
		echo '# ['
		echo .loop_${sp}_${spo[$sp]}:
		echo

		sp=$[sp + 1]
		spo[$sp]=0
	elif [ $opc == ']' ];
	then
		sp=$[sp - 1]

		echo '# ]'
		echo cmpb \$0, \(%esi\)
		echo jnz .loop_${sp}_${spo[$sp]}
		echo

		spo[$sp]=$[spo[$sp] + 1]
	elif [ $opc == '<' ];
	then
		echo '# <' 
		echo dec %esi
		echo
	elif [ $opc == '>' ];
	then
		echo '# >'
		echo inc %esi
		echo
	elif [ $opc == '-' ];
	then
		echo '# -'
		echo decb \(%esi\)
		echo
	elif [ $opc == '+' ];
	then
		echo '# +'
		echo incb \(%esi\)
		echo
	fi
done

echo "# end"
echo .done:
echo movl \$1, %eax
echo xor %ebx, %ebx
echo int \$0x80
echo .section .data
echo .mem:
echo .zero 30000) | as -o $file && ld $file; c=$?; rm -rf $file; exit $c

