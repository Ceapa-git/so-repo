#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 1024
#define VALID_SECT_COUNT 7
#define FIND_ALL_MAX_SIZE 955

const int32_t VALID_SECT_TYPES[VALID_SECT_COUNT] = {58, 20, 86, 28, 99, 89, 28};
const int16_t VALID_MAGIC_NUMBER = 0x7948; // 0x7948 = "Hy"
const int8_t VALID_VERSION_MIN = 46;
const int8_t VALID_VERSION_MAX = 118;
const int8_t VALID_SECT_COUNT_MIN = 6;
const int8_t VALID_SECT_COUNT_MAX = 20;

int succes_list = 0;

typedef struct _section_header
{
	char sect_name[17];
	int32_t sect_type;
	int32_t sect_offset;
	int32_t sect_size;
} section_header;

typedef struct _sf_header
{
	int8_t version;
	int8_t no_of_sections;
	section_header *section_headers;
	int16_t header_size;
	int16_t magic_nmber;
} sf_header;

int read_sf(char *file_path, sf_header *file_header)
{
	int fd = open(file_path, O_RDONLY);
	if (fd < 0)
		return -1;

	lseek(fd, -4, SEEK_END);
	read(fd, &(file_header->header_size), 2);
	read(fd, &(file_header->magic_nmber), 2);
	if (file_header->magic_nmber != VALID_MAGIC_NUMBER)
	{
		close(fd);
		return -2;
	}

	lseek(fd, -(file_header->header_size), SEEK_END);
	read(fd, &(file_header->version), 1);
	if (file_header->version < VALID_VERSION_MIN ||
		file_header->version > VALID_VERSION_MAX)
	{
		close(fd);
		return -3;
	}

	read(fd, &(file_header->no_of_sections), 1);
	if (file_header->no_of_sections < VALID_SECT_COUNT_MIN || file_header->no_of_sections > VALID_SECT_COUNT_MAX)
	{
		close(fd);
		return -4;
	}

	file_header->section_headers = (section_header *)calloc(file_header->no_of_sections, sizeof(section_header));
	for (int i = 0; i < file_header->no_of_sections; i++)
	{
		read(fd, file_header->section_headers[i].sect_name, 16);
		read(fd, &(file_header->section_headers[i].sect_type), 4);
		read(fd, &(file_header->section_headers[i].sect_offset), 4);
		read(fd, &(file_header->section_headers[i].sect_size), 4);
		int valid = 0;

		for (int j = 0; j < VALID_SECT_COUNT; j++)
		{
			if (file_header->section_headers[i].sect_type == VALID_SECT_TYPES[j])
			{
				valid = 1;
				break;
			}
		}

		if (valid == 0)
		{
			free(file_header->section_headers);
			close(fd);
			return -5;
		}
	}
	return fd;
}

void cmd_list(const char *path, int recursive, char *name_ends_with, int has_perm_execute, int find_all) // TODO broken
{
	DIR *dir = NULL;
	struct dirent *entry = NULL;

	dir = opendir(path);
	if (dir == NULL)
	{
		return;
	}

	if (!succes_list)
	{
		printf("SUCCESS\n");
		succes_list = 1;
	}

	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
		{
			char full_path[BUFF_SIZE + 1] = {};
			snprintf(full_path, BUFF_SIZE, "%s/%s", path, entry->d_name);
			struct stat statbuf;

			if (lstat(full_path, &statbuf) == 0)
			{
				if (S_ISDIR(statbuf.st_mode))
				{
					if (recursive)
					{
						cmd_list(full_path, recursive, name_ends_with, has_perm_execute, find_all);
					}
				}

				int ok = 1;
				if (name_ends_with != NULL)
				{
					int off_set = strlen(entry->d_name) - strlen(name_ends_with);
					if (strcmp(entry->d_name + off_set, name_ends_with) != 0)
					{
						ok = 0;
					}
				}

				if (has_perm_execute)
				{
					if ((statbuf.st_mode & S_IXUSR) == 0)
					{
						ok = 0;
					}
				}

				if (find_all)
				{
					sf_header file_header;
					int fd = read_sf(full_path, &file_header);
					if (fd < 0)
					{
						ok = 0;
					}
					else
					{
						close(fd);
						for (int i = 0; i < file_header.no_of_sections; i++)
						{
							if (file_header.section_headers[i].sect_size > FIND_ALL_MAX_SIZE)
							{
								ok = 0;
								break;
							}
						}
						free(file_header.section_headers);
					}
				}

				if (ok)
				{
					printf("%s\n", full_path);
				}
			}
		}
	}
	closedir(dir);
	return;
}

int main(int argc, char **argv)
{
	if (argc == 1)
		return 0;

	for (int argi = 1; argi < argc; argi++)
	{
		if (strcmp(argv[argi], "variant") == 0)
		{
			if (argc != 2)
				return 0;

			write(1, "26934\n", 6);
			return 0;
		}
		else if (strcmp(argv[argi], "list") == 0)
		{
			if (argc < 3)
				return 0;

			char *dir_name = NULL;
			char *name_ends_with = NULL;
			int has_perm_execute = 0;
			int recursive = 0;

			for (argi = 1; argi < argc; argi++)
			{
				if (strncmp(argv[argi], "path=", 5) == 0)
				{
					dir_name = argv[argi] + 5;
				}
				else if (strncmp(argv[argi], "name_ends_with=", 15) == 0)
				{
					name_ends_with = argv[argi] + 15;
				}
				else if (strcmp(argv[argi], "has_perm_execute") == 0)
				{
					has_perm_execute = 1;
				}
				else if (strcmp(argv[argi], "recursive") == 0)
				{
					recursive = 1;
				}
			}

			cmd_list(dir_name, recursive, name_ends_with, has_perm_execute, 0);
			if (succes_list == 0)
			{
				printf("ERROR\ninvalid directory path\n");
			}
			return 0;
		}
		else if (strcmp(argv[argi], "parse") == 0)
		{
			if (argc != 3)
				return 0;

			char *file_path = NULL;
			for (argi = 1; argi < argc; argi++)
			{
				if (strncmp(argv[argi], "path=", 5) == 0)
				{
					file_path = argv[argi] + 5;
				}
			}

			sf_header file_header;
			int err = read_sf(file_path, &file_header);
			if (err < 0)
			{
				switch (err)
				{
				case -2:
					printf("ERROR\nwrong magic\n");
					break;
				case -3:
					printf("ERROR\nwrong version\n");
					break;
				case -4:
					printf("ERROR\nwrong sect_nr\n");
					break;
				case -5:
					printf("ERROR\nwrong sect_types\n");
					break;
				default:
					break;
				}
				return 0;
			}

			close(err);
			printf("SUCCESS\n");
			printf("version=%d\n", (int)file_header.version);
			printf("nr_sections=%d\n", (int)file_header.no_of_sections);

			for (int i = 0; i < file_header.no_of_sections; i++)
			{
				printf("section%d: ", i + 1);
				printf("%s ", file_header.section_headers[i].sect_name);
				printf("%d ", (int)file_header.section_headers[i].sect_type);
				printf("%d\n", (int)file_header.section_headers[i].sect_size);
			}
			free(file_header.section_headers);
			return 0;
		}
		else if (strcmp(argv[argi], "extract") == 0) // TODO too slow
		{
			if (argc != 5)
				return 0;

			char *file_path = NULL;
			int line_nr = 0;
			int section_nr = 0;

			for (argi = 1; argi < argc; argi++)
			{
				if (strncmp(argv[argi], "path=", 5) == 0)
				{
					file_path = argv[argi] + 5;
				}
				else if (strncmp(argv[argi], "section=", 8) == 0)
				{
					sscanf(argv[argi] + 8, "%d", &section_nr);
				}
				else if (strncmp(argv[argi], "line=", 5) == 0)
				{
					sscanf(argv[argi] + 5, "%d", &line_nr);
				}
			}

			sf_header file_header;
			int fd = read_sf(file_path, &file_header);
			if (fd < 0)
			{
				printf("ERROR\ninvalid file\n");
				return 0;
			}

			if (section_nr <= 0 || section_nr > file_header.no_of_sections)
			{
				printf("ERROR\ninvalid section\n");
				free(file_header.section_headers);
				close(fd);
				return 0;
			}

			section_nr--;
			lseek(fd, file_header.section_headers[section_nr].sect_offset, SEEK_SET);
			int lines = 1;
			off_t *offsets = (off_t *)malloc(sizeof(off_t) * line_nr);
			offsets[0] = 0;

			for (int i = 0; i < file_header.section_headers[section_nr].sect_size; i++)
			{
				char c;
				read(fd, &c, 1);
				if (c == '\n')
				{
					for (int j = line_nr - 1; j > 0; j--)
					{
						offsets[j] = offsets[j - 1];
					}
					offsets[0] = i + 1;
					lines++;
				}
			}

			if (lines < line_nr)
			{
				printf("ERROR\ninvalid line\n");
				free(offsets);
				free(file_header.section_headers);
				close(fd);
				return 0;
			}

			char c;
			int i = offsets[line_nr - 1];
			lseek(fd, file_header.section_headers[section_nr].sect_offset + i, SEEK_SET);
			printf("SUCCESS\n");
			while (i < file_header.section_headers[section_nr].sect_size)
			{
				read(fd, &c, 1);
				if (c == '\n')
					break;
				printf("%c", c);
				i++;
			}

			printf("\n");
			free(offsets);
			free(file_header.section_headers);
			close(fd);
			return 0;
		}
		else if (strcmp(argv[argi], "findall") == 0)
		{
			if (argc != 3)
				return 0;

			char *dir_name = NULL;
			for (argi = 1; argi < argc; argi++)
			{
				if (strncmp(argv[argi], "path=", 5) == 0)
				{
					dir_name = argv[argi] + 5;
				}
			}

			cmd_list(dir_name, 1, NULL, 0, 1);
			if (succes_list == 0)
			{
				printf("ERROR\ninvalid directory path\n");
			}
			return 0;
		}
	}
	return 0;
}
