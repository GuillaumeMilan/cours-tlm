CC=gcc
INCLUDE=-I. -I../../
CFLAGS=$(INCLUDE) -Wall -O

42: main.o
	$(CC) -o 42 main.o

main.o: ../main.c
	$(CC) -o main.o -c ../main.c $(CFLAGS)

.PHONY: clean realclean
clean:
	$(RM) main.o

realclean: clean
	-$(RM) *~
