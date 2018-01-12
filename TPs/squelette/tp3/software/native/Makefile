CC=gcc
INCLUDE=-I. -I../../
CFLAGS=$(INCLUDE) -Wall -O

main.o: ../main.c
	$(CC) -o main.o -c ../main.c $(CFLAGS)

.PHONY: clean realclean
clean:
	$(RM) main.o

realclean: clean
	-$(RM) *~
