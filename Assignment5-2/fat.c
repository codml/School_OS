#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FILE_NAME 100
#define ENTRY 100
#define BLOCK_SIZE 1024
#define BYTES 32

// file entry(name, start block, size, use bit) //
struct file_entry {
	char	file_name[FILE_NAME+1];
	int		start;
	int		file_size;
	int		use;
};

/// FAT file system components: these are in memory ///
struct file_entry	File_entries[ENTRY]; // free space: use -> 1

// free space: -2, last block: -1
int					FAT[BLOCK_SIZE];

 // +1: for strlen(Disk[index]) -> block full or not
char				Disk[BLOCK_SIZE][BYTES+1];

void create_file(const char *name)
{
	int free; // free space index
	int fBlock; // free block

	// check for name duplicate //
	for (int i = 0; i < ENTRY; i++) {
		if (File_entries[i].use && !strcmp(File_entries[i].file_name, name)) {
			printf("File %s already exist\n", name);
			return;
		}
	}

	for (free = 0; free < ENTRY; free++) {
		// find free space //
		if (File_entries[free].use == 0) {
			
			// file first block set //
			for (fBlock = 0; fBlock < BLOCK_SIZE; fBlock++) {
				if (FAT[fBlock] == -2) {
					File_entries[free].start = fBlock;
					FAT[fBlock] = -1; // last block
					strcpy(Disk[fBlock], "\0");
					break;
				}
			}
			if (fBlock == BLOCK_SIZE) {
				perror("no free block");
				return ;
			}
			// file name set //
			strcpy(File_entries[free].file_name, name);
			File_entries[free].file_name[FILE_NAME] = '\0';
			// file size set //
			File_entries[free].file_size = 0;
			// use bit set //
			File_entries[free].use = 1;
			break;
		}
	}
	if(free == ENTRY)
		perror("Full of file entries");
	else
		printf("File \'%s\' created.\n", name);
}

void write_file(const char *name, const char *content)
{
	int file; // file entry
	int block; // block number
	int nBlock; // for next block
	int bIndex; // byte index

	// find file "name" //
	for (file = 0; file < ENTRY; file++) {
		if (File_entries[file].use && !strcmp(name, File_entries[file].file_name)) {

			block = File_entries[file].start;

			// find block to write //
			while (FAT[block] >= 0)
				block = FAT[block];
			// write bytes until block is full //
			while (*content != '\0') {

				// write content byte by byte //
				for (bIndex = strlen(Disk[block]); bIndex < BYTES && *content != '\0'; bIndex++) {
					// to avoid write newline to Disk(file) //
					if (*content == '\n')
						Disk[block][bIndex] = ' ';
					else
						Disk[block][bIndex] = *content;
					content++;
					File_entries[file].file_size += 1;
				}
				Disk[block][bIndex] = '\0';

				// if write not done //
				if (*content != '\0') {
					// find free block //
					for (nBlock = 0; nBlock < BLOCK_SIZE; nBlock++) {
						if (FAT[nBlock] == -2) {
							FAT[block] = nBlock;
							FAT[nBlock] = -1;
							strcpy(Disk[nBlock], "\0");
							block = nBlock;
							break;
						}
					}
					if (nBlock == BLOCK_SIZE) {
						perror("no free block, partial write");;
						return;
					}
				}
			}
			break;
		}
	}
	if (file == ENTRY)
		printf("File \'%s\' doesn't exist\n", name);
	else
		printf("Data written to \'%s\'.\n", name);
}

void read_file(const char *name)
{
	int file; // file entry
	int block; // block number

	printf("Content of \'%s\': ", name);

	// find file "name" //
	for (file = 0; file < ENTRY; file++) {
		if (File_entries[file].use && !strcmp(name, File_entries[file].file_name)) {

			block = File_entries[file].start;

			// iterate until last block read //
			while (FAT[block] >= 0) {
				printf("%s", Disk[block]);
				block = FAT[block];
			}
			printf("%s", Disk[block]); // print last block's content
			break;
		}
	}
	if (file == ENTRY) // file doesn't exist
		printf("None");
	printf("\n");
}

void delete_file(const char *name)
{
	int file; // file entry
	int block; // block number
	int nextB; // block to delete next


	// find file "name" //
	for (file = 0; file < ENTRY; file++) {
		if (File_entries[file].use && !strcmp(name, File_entries[file].file_name)) {

			block = File_entries[file].start;

			// iterate until reach last block //
			while (FAT[block] >= 0) {
				nextB = FAT[block];
				FAT[block] = -2;
				block = nextB;
			}
			FAT[block] = -2;
			File_entries[file].use = 0; // file entry delete
			break;
		}
	}
	if (file == ENTRY) // file doesn't exist
		printf("File \'%s\' doesn't exist.\n", name);
	else
		printf("File \'%s\' deleted.\n", name);
}

void list_files(void)
{
	int file; // file entry

	printf("Files in the file system:\n");

	// find file that has use bit 1: exist //
	for (file = 0; file < ENTRY; file++) {
		if (File_entries[file].use) {
			printf("File: %s, Size: %d bytes\n",
				File_entries[file].file_name, File_entries[file].file_size);
		}
	}
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
	for (int i = 0; i < ENTRY; i++)
		fprintf(FE, "%s %d %d %d\n", File_entries[i].file_name,
			File_entries[i].file_size, File_entries[i].start,
			File_entries[i].use);

	// FAT: integers //
	for (int i = 0; i < BLOCK_SIZE; i++)
		fprintf(FA, "%d\n", FAT[i]);

	// Disk: bytes -> empty block: "" //
	for (int i = 0; i < BLOCK_SIZE; i++)
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
    char	buf[BYTES+2];

	// file open //
	if (!(FE = fopen("File_entry", "r"))) {
		// init file system //
		printf("Warning: No saved state found. Starting fresh.\n");

		// File entry.use -> free space //
		for (int i = 0; i < ENTRY; i++)
			File_entries[i].use = 0;

		// FAT: -2 -> free space //
		for (int i = 0; i < BLOCK_SIZE; i++)
			FAT[i] = -2;
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
		for (int i = 0; i < ENTRY; i++) {
			fscanf(FE, "%s %d %d %d", File_entries[i].file_name,
			&(File_entries[i].file_size), &(File_entries[i].start),
			&(File_entries[i].use));
		}

		// FAT: integers //
		for (int i = 0; i < BLOCK_SIZE; i++)
			fscanf(FA, "%d", &(FAT[i]));

		// Disk: bytes -> empty block: "" //
		for (int i = 0; i < BLOCK_SIZE && fgets(buf, BYTES+2, FD); i++) {
			buf[strlen(buf)-1] = '\0';
			strcpy(Disk[i], buf);
		}
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

	if (!strcmp(argv[1], "create") && argc == 3)
		create_file(argv[2]);
	else if (!strcmp(argv[1], "list") && argc == 2)
		list_files();
	else if (!strcmp(argv[1], "write") && argc == 4)
		write_file(argv[2], argv[3]);
	else if (!strcmp(argv[1], "read") && argc == 3)
		read_file(argv[2]);
	else if (!strcmp(argv[1], "delete") && argc == 3)
		delete_file(argv[2]);
	else
		perror("wrong command or wrong argument");
	
	if (save_file_system() < 0)
		perror("disk is damaged");

    return 0;
}