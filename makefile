build:
	gcc -o main main.c -Wpedantic --pedantic -lcurl

run:
	./main