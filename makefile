.phony all:
all: diskinfo disklist diskget diskput

diskinfo: diskinfo.c
	gcc diskinfo.c -lreadline -lhistory -o diskinfo -std=c99

disklist: disklist.c
	gcc disklist.c -lreadline -lhistory -o disklist -std=c99
	
diskget: diskget.c
	gcc diskget.c -lreadline -lhistory -o diskget -std=c99

diskput: diskput.c
	gcc diskput.c -lreadline -lhistory -o diskput -std=c99

.PHONY clean:
clean:
	-rm -rf *.o *.exe