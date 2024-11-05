CC = gcc -g
NAME = cpu_scheduler

all : $(NAME)

%.o : %.c
	$(CC) -o $(NAME) $@ $<

clean : 
	rm -rf $(NAME)