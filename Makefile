CC = gcc -lm
CFLAGS =
OBJM = oss.o
OBJP = userProcess.o fileHandler.o
MAIN = oss
PROC = userProcess

.SUFFIXES: .c .o

all: $(MAIN) $(PROC) $(CONS)

$(MAIN): $(OBJM)
	$(CC) -o $@ $^

$(PROC): $(OBJP)
	$(CC) -o $@ $^

.c.o:
	$(CC) -c -o $@ $<

clean:
	rm *.o
	rm -f oss
	rm -f userProcess
