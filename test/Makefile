#CC = mipsel-linux-gcc
CC = gcc
CFLAGS = -Wall -I ./include
targ=main 
all +=main.o
#LDLIBS= -L./ -lbase
LDLIBS= -lbase 
export CC
$(targ): $(all)
	$(CC) $(CFLAGS) -o $(targ) $(all) $(LDLIBS) -lpthread -lm 
	$(RM) -f *.gch *.bak $(all) 
%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $< 

.PHONY: clean
clean:
	rm -f $(targ) $(all) 
