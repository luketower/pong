COMPILE = gcc -g -Wall main.c -o pong -I include -L lib -l SDL2-2.0.0

build:
	$(COMPILE)

run:
	$(COMPILE) && ./pong

clean:
	rm ./pong && rm -rf pong.dSYM
