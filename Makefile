tide : *.c
	gcc -O2 -Wall -Werror -march=native -o tide \
		tide.c tide_server.c tide_error.c tide_utils.c -lpthread

clean :
	\rm -f tide *.o
