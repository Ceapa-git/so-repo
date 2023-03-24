#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
		char* path=(char*)calloc(strlen(dir_name)+strlen(entry->d_name)+5,1);
		strcpy(path,dir_name);
		strcat(path,"/");
		strcat(path,entry->d_name);
		struct stat stat_buff;
		lstat(path,&stat_buff);
		if (has_perm_execute != 0)
		{
			if(!(stat_buff.st_mode&0100))
			{
				continue;
			}
		}
		printf("%s/%s\n", dir_name, entry->d_name);
		if (recursive != 0)
		{
			if (S_ISDIR(stat_buff.st_mode))
			{
				DIR *dirp = opendir(path);
				if (dirp == NULL)
				{
					printf("ERROR\ninvalid directory path\n");
				}
				cmd_list(path, dirp, recursive, name_ends_with, has_perm_execute);
				closedir(dirp);
			}
		}
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
			printf("26934\n");
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
				printf("ERROR\ninvalid directory path\n");
			}
			printf("SUCCESS\n");
			cmd_list(dir_name, dirp, recursive, name_ends_with, has_perm_execute);
			closedir(dirp);
			return 0;
		}
		else if (strcmp(argv[argi], "parse") == 0)
		{
			if (argc != 3)
				return 0;
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
