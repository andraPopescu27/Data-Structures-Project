all: tema1

build: tema1.c
	gcc -Wall tema1.c -o tema1 -lm

run: tema1.c
	./tema1

clean:
	rm tema1
