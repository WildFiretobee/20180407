SOURCES:=$(wildcard *.c)

OBJS:=$(patsubst %.c,%.o,${SOURCES})

main:${OBJS}
	
	gcc $^ -o $@ -pthread -I /usr/include/mysql/ -lmysqlclient -lcrypt
clean:
	rm main ${OBJS}
