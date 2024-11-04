CC = gcc
LDFLAGS = -pthread
NAME = numgen fork thread

all : $(NAME)

%.o: %.c
	$(CC) -o $@ $<

clean:
	rm -rf $(NAME)

re:
	rm -rf tmp*
	sync
	echo 3 | sudo tee /proc/sys/vm/drop_caches