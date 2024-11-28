#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FILE_NAME 100
#define ENTRY 100
#define BLOCKS 1024
#define BYTES 32

struct file_entry {
	char	file_name[FILE_NAME+1];
	int		start;
	int		file_size;
};

/// FAT file system components: these are in memory ///
struct file_entry	File_entries[ENTRY]; // simillar with Linked-list
int					Entry_size;
int					FAT[BLOCKS];
char				Disk[BLOCKS][BYTES]; // consider null: treat as EOF

void create_file(const char *name)
{

}

void write_file(const char *name, const char *content)
{

}

void read_file(const char *name)
{

}

void delete_file(const char *name)
{

}

void list_files(void)
{

}

int save_file_system(void)
{
	// FE: file entry, FA: FAT, FD: Dist //
	FILE	*FE, *FA, *FD;
    char	buf[BYTES];

	if (!(FE = fopen("File_entry", "w")))
		return -1;
	if (!(FA = fopen("FAT", "w"))) {
		fclose(FE);
		return -1;
	}
	if (!(FD = fopen("Disk", "w"))) {
		fclose(FE);
		fclose(FA);
		return -1;
	}

	/// File entry: file name, size, start block ///
	for (int i = 0; i < Entry_size; i++)
		fprintf(FE, "%s%d%d\n", File_entries[i].file_name,
			File_entries[i].file_size, File_entries[i].start);

	// FAT: integers //
	for (int i = 0; i < BLOCKS; i++)
		fprintf(FA, "%d\n", FAT[i]);

	// Disk: bytes -> empty block: "" //
	for (int i = 0; i < BLOCKS; i++)
		fprintf(FD, "%s\n", Disk[i]);
	fclose(FE);
	fclose(FA);
	fclose(FD);
	return 0;
}

int load_file_system(void)
{
	// FE: file entry, FA: FAT, FD: Dist //
	FILE	*FE, *FA, *FD;
    char	buf[BYTES];

	Entry_size = 0;
	// file open //
	if (!(FE = fopen("File_entry", "r"))) {
		// init file system //
		printf("Warning: No saved state found. Starting fresh.\n");
		for (int i = 0; i < BLOCKS; i++)
			FAT[i] = -1;
    }
	else {
		if (!(FA = fopen("FAT", "r"))) {
			fclose(FE);
			return -1;
		}
		if (!(FD = fopen("Disk", "r"))) {
			fclose(FE);
			fclose(FA);
			return -1;
		}
		/// File entry: file name, size, start block ///
		while (fscanf(FE, "%s%d%d", File_entries[Entry_size].file_name,
			&(File_entries[Entry_size].file_size), &(File_entries[Entry_size].start)))
			Entry_size++;

		// FAT: integers //
		for (int i = 0; i < BLOCKS; i++)
			fscanf(FA, "%d", &(FAT[i]));

		// Disk: bytes -> empty block: "" //
		for (int i = 0; i < BLOCKS && fgets(buf, BYTES, FD); i++)
			strcpy(Disk[i], buf);
		fclose(FE);
		fclose(FA);
		fclose(FD);
	}
	return 0;
}

int main (int argc, char **argv) {
    if (argc < 2 || argc > 4) {
        perror("Argument format: [API] [file name] [contents to write]");
        return 0;
    }

	if (load_file_system() < 0) {
		perror("files are not loaded");
		return 0;
	}

	if (!strcmp(argv[1], "create"))
		create_file(argv[2]);
	else if (!strcmp(argv[1], "list"))
		list_files();
	else if (!strcmp(argv[1], "write"))
		write_file(argv[2], argv[3]);
	else if (!strcmp(argv[1], "read"))
		read_file(argv[2]);
	else if (!strcmp(argv[1], "delete"))
		delete_file(argv[2]);
	else
		perror("wrong command");
	
	if (save_file_system() < 0)
		perror("disk is damaged");

    return 0;
}