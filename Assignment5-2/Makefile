CC = gcc
NAME = fat
FILES = Disk FAT File_entry
all : $(NAME)

%.o : %.c
	$(CC) -o $(NAME) $@ $<

clean : 
	rm -rf $(NAME)

reset :
	rm -rf $(FILES)