#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void cmd_list(char *dir_name, DIR *root, int recursive, char *name_ends_with, int has_perm_execute)
{
	struct dirent *entry = NULL;
	while ((entry = readdir(root)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;
		if (name_ends_with != NULL)
		{
			int off_set = strlen(entry->d_name) - strlen(name_ends_with);
			if (strcmp(entry->d_name + off_set, name_ends_with) != 0)
				continue;
		}
		char *path = (char *)calloc(strlen(dir_name) + strlen(entry->d_name) + 5, 1);
		strcpy(path, dir_name);
		strcat(path, "/");
		strcat(path, entry->d_name);
		struct stat stat_buff;
		lstat(path, &stat_buff);
		if (has_perm_execute != 0)
		{
			if (!(stat_buff.st_mode & 0100))
			{
				free(path);
				continue;
			}
		}
		/*write(1,dir_name,strlen(dir_name));
		write(1,"/",1);
		write(1,entry->d_name,strlen(entry->d_name));
		write(1,"\n",1);*/
		printf("%s/%s\n", dir_name, entry->d_name);
		if (recursive != 0)
		{
			if (S_ISDIR(stat_buff.st_mode))
			{
				DIR *dirp = opendir(path);
				if (dirp == NULL)
				{
					// write(1,"ERROR\ninvalid directory path\n",30);
					printf("ERROR\ninvalid directory path\n");
				}
				else
				{
					cmd_list(path, dirp, recursive, name_ends_with, has_perm_execute);
					closedir(dirp);
				}
			}
		}
		free(path);
	}
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
			DIR *dirp = opendir(dir_name);
			if (dirp == NULL)
			{
				// write(1,"ERROR\ninvalid directory path\n",30);
				printf("ERROR\ninvalid directory path\n");
				return 0;
			}
			// write(1,"SUCCESS\n",9);
			printf("SUCCESS\n");
			cmd_list(dir_name, dirp, recursive, name_ends_with, has_perm_execute);
			closedir(dirp);
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
			int fd = open(file_path, O_RDONLY);
			if (fd < 0)
				return 0;
			sf_header file_header;
			lseek(fd, -4, SEEK_END);
			read(fd, &(file_header.header_size), 2);
			read(fd, &(file_header.magic_nmber), 2);

			if (file_header.magic_nmber != (0x7948))
			{
				printf("ERROR\nwrong magic\n");
				close(fd);
				return 0;
			}

			lseek(fd, -file_header.header_size, SEEK_END);

			read(fd, &(file_header.version), 1);
			read(fd, &(file_header.no_of_sections), 1);
			file_header.section_headers = (section_header *)malloc(file_header.no_of_sections * sizeof(section_header));
			for (int i = 0; i < file_header.no_of_sections; i++)
			{
				read(fd, file_header.section_headers[i].sect_name, 16);
				read(fd, &(file_header.section_headers[i].sect_type), 4);
				read(fd, &(file_header.section_headers[i].sect_offset), 4);
				read(fd, &(file_header.section_headers[i].sect_size), 4);
			}
			read(fd, &(file_header.header_size), 2);
			read(fd, &(file_header.magic_nmber), 2);

			if (file_header.version < 46 || file_header.version > 118)
			{
				printf("ERROR\nwrong version\n");
				close(fd);
				free(file_header.section_headers);
				return 0;
			}
			if (file_header.no_of_sections < 6 || file_header.no_of_sections > 20)
			{
				printf("ERROR\nwrong sect_nr\n");
				close(fd);
				free(file_header.section_headers);
				return 0;
			}
			for (int i = 0; i < file_header.no_of_sections; i++)
			{
				if (file_header.section_headers[i].sect_type != 58 &&
					file_header.section_headers[i].sect_type != 20 &&
					file_header.section_headers[i].sect_type != 86 &&
					file_header.section_headers[i].sect_type != 28 &&
					file_header.section_headers[i].sect_type != 99 &&
					file_header.section_headers[i].sect_type != 89 &&
					file_header.section_headers[i].sect_type != 28)
				{
					printf("ERROR\nwrong sect_types\n");
					close(fd);
					free(file_header.section_headers);
					return 0;
				}
			}
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
		}
		else if (strcmp(argv[argi], "extract") == 0)
		{
			if (argc != 5)
				return 0;
		}
		else if (strcmp(argv[argi], "findall") == 0)
		{
			if (argc != 3)
				return 0;
		}
	}
	return 0;
}
