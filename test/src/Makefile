CC = gcc
CFLAGS = -Wall 
targ=main 
all +=main.o
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
