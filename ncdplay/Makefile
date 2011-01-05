all:
	make -C src all
	make -C doc all

clean:
	make -C src clean
	make -C doc clean

install: all
	install -m 0755 ncdplay /usr/local/bin
	make -C doc install
