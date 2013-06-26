all:
	gcc -W -Wall -s -O2 -o bmp2html bmp2html.c

clean:
	rm -f bmp2html

unzip:
	xz -d ft-lucy.bmp.xz ft-lucy.html.xz

zip:
	xz -9e ft-lucy.bmp ft-lucy.html
