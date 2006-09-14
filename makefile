CFLAGS=-Wall -g
all: aiginfo aignm aigsim aigstrip aigtoaig aigtocnf aigtosmv smvtoaig test
test: testaigtoaig
aiginfo: aiger.o aiginfo.o makefile
	$(CC) -o $@ aiginfo.o aiger.o
aignm: aiger.o aignm.o makefile
	$(CC) -o $@ aignm.o aiger.o
aigsim: aiger.o aigsim.o makefile
	$(CC) -o $@ aigsim.o aiger.o
aigstrip: aiger.o aigstrip.o makefile
	$(CC) -o $@ aigstrip.o aiger.o
aigtoaig: aiger.o aigtoaig.o makefile
	$(CC) -o $@ aigtoaig.o aiger.o
aigtocnf: aiger.o aigtocnf.o makefile
	$(CC) -o $@ aigtocnf.o aiger.o
aigtosmv: aiger.o aigtosmv.o makefile
	$(CC) -o $@ aigtosmv.o aiger.o
andtoaig: aiger.o andtoaig.o makefile
	$(CC) -o $@ andtoaig.o aiger.o
smvtoaig: aiger.o smvtoaig.o makefile
	$(CC) -o $@ smvtoaig.o aiger.o
testaigtoaig: aiger.o testaigtoaig.o makefile
	$(CC) -o $@ testaigtoaig.o aiger.o
aigerversion.h: *.c FORMAT README makefile maxdate
	rm -f aigerversion.h
	echo "#define AIGER_VERSION \"`./maxdate`\"" > $@
aiger.h: aigerversion.h
aiger.o: aiger.h aiger.c makefile
aiginfo.o: aiginfo.c aiger.h makefile
aignm.o: aignm.c aiger.h makefile
aigsim.o: aigsim.c aiger.h makefile
aigstrip.o: aigstrip.c aiger.h makefile
aigtoaig.o: aigtoaig.c aiger.h makefile
aigtocnf.o: aigtocnf.c aiger.h makefile
aigtosmv.o: aigtosmv.c aiger.h makefile
smvtoaig.o: smvtoaig.c aiger.h makefile
testaigtoaig.o: testaigtoaig.c aiger.h makefile
clean:
	rm -f *.o 
	rm -f log/*.smvfromaig
	rm -f log/*.aig log/*.big log/*.cig
	rm -f log/*.aig.gz log/*.big.gz log/*.cig.gz
	rm -f aiginfo aignm aigsim aigstrip aigtoaig 
	rm -f aigtocnf aigtosmv andtoaig smvtoaig 
	rm -f testaigtoaig
	rm -f aigerversion.h
.PHONY: all test
