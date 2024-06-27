all: libmsocket.a initmsocket user1 user2

libmsocket.a: msocket.o
	ar rcs libmsocket.a msocket.o

init: initmsocket.o libmsocket.a
	gcc -o init initmsocket.o -L. -lmsocket

user1: user1.o libmsocket.a
	gcc -o user1 user1.o -L. -lmsocket

user2: user2.o libmsocket.a
	gcc -o user2 user2.o -L. -lmsocket

%.o: %.c
	gcc -c $<

clean:
	rm -f *.o *.a initmsocket user1 user2