
CFLAGE= -lm
LIBS += -lm

all: final

final: main.o
	gcc $(CFLAGS) main.o -o worbs $(LIBS)
main.o: main.c
	gcc $(CFLAGS) -c main.c -c

clean:
	rm main.o
