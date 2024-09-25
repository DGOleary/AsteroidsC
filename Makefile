CC = gcc 
CFLAGS = -I C:/MinGW/include -L C:/MinGW/bin -m32 -g -O0
LIBS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image
OTHERS = drawfunctions.c defaultfunctions.c

all: program test

program: asteroids.c $(OTHERS)
	$(CC) $(CFLAGS) -o asteroids asteroids.c $(OTHERS) $(LIBS)

test: test.c $(OTHERS)
	$(CC) $(CFLAGS) -o test test.c $(OTHERS) $(LIBS)

clean:
	rm -f asteroids test
