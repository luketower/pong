COMPILE = gcc main.c -o pong

build:
	$(COMPILE)

run:
	$(COMPILE) && ./pong
