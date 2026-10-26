CC = cc
CPPFLAGS = -D_POSIX_C_SOURCE=200809L -I. -Iinclude
CFLAGS = -Wall -Werror -Wextra -pedantic -std=c99

TARGET = sws

OBJS = main.o socket.o src/http.o src/cgi.o
LOG_SOURCE = $(wildcard src/log.c log.c)
OBJS += $(LOG_SOURCE:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c sws.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

test-http:
	$(CC) $(CPPFLAGS) $(CFLAGS) src/http.c src/test_http.c -o test_http
	./test_http

clean:
	rm -f $(OBJS) $(TARGET) test_http

.PHONY: all clean test-http
