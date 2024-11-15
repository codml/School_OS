CC = gcc -g
NAME = page_replacement_simulator

all : $(NAME)

%.o : %.c
	$(CC) -o $(NAME) $@ $<

clean : 
	rm -rf $(NAME)