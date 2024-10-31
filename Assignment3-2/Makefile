CC = gcc
LDFLAGS = -pthread
NAME = numgen fork thread

all : $(NAME)

%.o: %.c
	$(CC) -o $@ $<

clean:
	rm -rf $(NAME)