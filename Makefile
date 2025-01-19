# make file for osdp discovery tool

#  (C)2025 Smithee Solutions LLC


all:
	(cd src; make)

clean:
	(cd src; make clean)
	(cd package; make clean)
	rm -rf opt *.deb

package:	all
	mkdir -p opt/tester/etc
	cp test/discovery-settings.json opt/tester/etc
	(cd package; make package; mv *.deb ..)

