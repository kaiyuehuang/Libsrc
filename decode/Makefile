CROSS_COMPILE	=mipsel-linux-
#CC=$(CROSS_COMPILE)gcc
CC=gcc

CFLAGS = -Wall  -I /home/sambashare/yue/server_src/encode_src/output/x86_output/include/
TAR = madplay
all += madplay.o

export CC
$(TAR): $(all)
	$(CC) $(CFLAGS) -o $(TAR) $(all) -lpthread -lmad
#	cp $(TAR) $(PWD)/bin/
#	cp $(TAR) /nfs/encoder/
	$(RM) -f *.gch *.bak $(all) 
	
%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $< 

.PHONY: clean
clean:
	rm -f $(TAR) $(all) 
