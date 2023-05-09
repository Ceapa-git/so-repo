#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

#define VALID_SECT_COUNT 7

const int32_t VALID_SECT_TYPES[VALID_SECT_COUNT] = {58, 20, 86, 28, 99, 89, 28};
const int16_t VALID_MAGIC_NUMBER = 0x7948; // 0x7948 = "Hy"
const int8_t VALID_VERSION_MIN = 46;
const int8_t VALID_VERSION_MAX = 118;
const int8_t VALID_SECT_COUNT_MIN = 6;
const int8_t VALID_SECT_COUNT_MAX = 20;

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

int read_sf(volatile void *file_addr, int file_size, sf_header *file_header)
{
    file_header->header_size = *(int16_t *)(file_addr + file_size - 4);
    file_header->magic_nmber = *(int16_t *)(file_addr + file_size - 2);
    if (file_header->magic_nmber != VALID_MAGIC_NUMBER)
        return 0;

    file_header->version = *(int8_t *)(file_addr + file_size - file_header->header_size);
    if (file_header->version < VALID_VERSION_MIN || file_header->version > VALID_VERSION_MAX)
        return 0;

    file_header->no_of_sections = *(int8_t *)(file_addr + file_size - file_header->header_size + 1);
    if (file_header->no_of_sections < VALID_SECT_COUNT_MIN || file_header->no_of_sections > VALID_SECT_COUNT_MAX)
        return 0;

    file_header->section_headers = (section_header *)calloc(file_header->no_of_sections, sizeof(section_header));
    int offset = file_size - file_header->header_size + 2;
    for (int i = 0; i < file_header->no_of_sections; i++)
    {
        for (int j = 0; j < 16; j++)
            file_header->section_headers[i].sect_name[j] = *(char *)(file_addr + (offset++));
        file_header->section_headers[i].sect_type = *(int32_t *)(file_addr + offset);
        offset += 4;
        file_header->section_headers[i].sect_offset = *(int32_t *)(file_addr + offset);
        offset += 4;
        file_header->section_headers[i].sect_size = *(int32_t *)(file_addr + offset);
        offset += 4;
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
            return 0;
        }
    }
    return 1;
}

void to_pipe_string(char *string)
{
    int i = -1;
    while (string[++i] != '\0')
        ;
    string[i] = '.';
}
void to_c_string(char *string)
{
    int i = -1;
    while (string[++i] != '.')
        ;
    string[i] = '\0';
}
int main(int argc, char **argv)
{
    int resp_pd, req_pd;
    if (mkfifo("RESP_PIPE_26934", 0666))
    {
        printf("ERROR\ncannot create the response pipe\n");
        return 0;
    }
    resp_pd = open("RESP_PIPE_26934", 0666);
    if ((req_pd = open("REQ_PIPE_26934", 0666)) < 0)
    {
        printf("ERROR\ncannot open the request pipe\n");
        return 0;
    }

    char begin_message[] = "BEGIN";
    to_pipe_string(begin_message);
    write(resp_pd, begin_message, 6);

    int shm_fd = -1;
    volatile void *shm_addr = (void *)-1;
    int file_fd = -1;
    int file_size = -1;
    volatile void *file_addr = (void *)-1;

    while (1)
    {
        char buff[300] = {};
        int i = 0;
        do
        {
            read(req_pd, buff + i, 1);
        } while (buff[i++] != '.');
        to_c_string(buff);
        if (strcmp(buff, "ECHO") == 0)
        {
            char echo_message[] = "ECHO";
            unsigned int variant = 26934;
            char variant_message[] = "VARIANT";
            to_pipe_string(echo_message);
            to_pipe_string(variant_message);
            write(resp_pd, echo_message, 5);
            write(resp_pd, &variant, sizeof(unsigned int));
            write(resp_pd, variant_message, 8);
        }
        else if (strcmp(buff, "CREATE_SHM") == 0)
        {
            unsigned int val_shm = 0;
            read(req_pd, &val_shm, sizeof(unsigned int));

            char shm_message[] = "CREATE_SHM";
            to_pipe_string(shm_message);
            write(resp_pd, shm_message, 11);
            if ((shm_fd = shm_open("/fX247y", O_RDWR | O_CREAT, 0664)) < 0)
            {
                char shm_status[] = "ERROR";
                to_pipe_string(shm_status);
                write(resp_pd, shm_status, 6);
            }
            else
            {
                ftruncate(shm_fd, val_shm);
                if ((shm_addr = (volatile void *)mmap(0, val_shm, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == (void *)-1)
                {
                    char shm_status[] = "ERROR";
                    to_pipe_string(shm_status);
                    write(resp_pd, shm_status, 6);
                }
                else
                {
                    char shm_status[] = "SUCCESS";
                    to_pipe_string(shm_status);
                    write(resp_pd, shm_status, 8);
                }
            }
        }
        else if (strcmp(buff, "WRITE_TO_SHM") == 0)
        {
            unsigned int offset_write, value_write;
            read(req_pd, &offset_write, sizeof(unsigned int));
            read(req_pd, &value_write, sizeof(unsigned int));

            char write_message[] = "WRITE_TO_SHM";
            to_pipe_string(write_message);
            write(resp_pd, write_message, 13);

            if (offset_write + sizeof(unsigned int) >= 3384148 || shm_addr == (void *)-1)
            {
                char write_status[] = "ERROR";
                to_pipe_string(write_status);
                write(resp_pd, write_status, 6);
            }
            else
            {
                *(unsigned int *)(shm_addr + offset_write) = value_write;
                char write_status[] = "SUCCESS";
                to_pipe_string(write_status);
                write(resp_pd, write_status, 8);
            }
        }
        else if (strcmp(buff, "MAP_FILE") == 0)
        {
            char file_map[260] = {};
            int i = 0;
            int first_dot = 0;
            int second_dot = 0;
            do
            {
                read(req_pd, file_map + i, 1);
                if (file_map[i] == '.' && !first_dot)
                    first_dot = 1;
                else if (file_map[i] == '.')
                    second_dot = 1;
                i++;
            } while (!second_dot);
            file_map[--i] = '\0';

            char map_message[] = "MAP_FILE";
            to_pipe_string(map_message);
            write(resp_pd, map_message, 9);
            if ((file_fd = open(file_map, O_RDONLY)) < 0)
            {
                char map_status[] = "ERROR";
                to_pipe_string(map_status);
                write(resp_pd, map_status, 6);
            }
            else
            {
                file_size = lseek(file_fd, 0, SEEK_END);
                lseek(file_fd, 0, SEEK_SET);
                if ((file_addr = (volatile void *)mmap(0, file_size, PROT_READ, MAP_SHARED, file_fd, 0)) == (void *)-1)
                {
                    char map_status[] = "ERROR";
                    to_pipe_string(map_status);
                    write(resp_pd, map_status, 6);
                }
                else
                {
                    char map_status[] = "SUCCESS";
                    to_pipe_string(map_status);
                    write(resp_pd, map_status, 8);
                }
            }
        }
        else if (strcmp(buff, "READ_FROM_FILE_OFFSET") == 0)
        {
            unsigned int offset_read, no_bytes_read;
            read(req_pd, &offset_read, sizeof(unsigned int));
            read(req_pd, &no_bytes_read, sizeof(unsigned int));

            char read_message[] = "READ_FROM_FILE_OFFSET";
            to_pipe_string(read_message);
            write(resp_pd, read_message, 22);

            if (shm_addr == (void *)-1 || no_bytes_read >= 3384148 || file_addr == (void *)-1 || offset_read + no_bytes_read >= file_size)
            {
                char read_status[] = "ERROR";
                to_pipe_string(read_status);
                write(resp_pd, read_status, 6);
            }
            else
            {
                for (int i = 0; i < no_bytes_read; i++)
                    *(char *)(shm_addr + i) = *(char *)(file_addr + offset_read + i);
                char read_status[] = "SUCCESS";
                to_pipe_string(read_status);
                write(resp_pd, read_status, 8);
            }
        }
        else if (strcmp(buff, "READ_FROM_FILE_SECTION") == 0)
        {
            unsigned int section_no_section, offset_section, no_bytes_section;
            read(req_pd, &section_no_section, sizeof(unsigned int));
            read(req_pd, &offset_section, sizeof(unsigned int));
            read(req_pd, &no_bytes_section, sizeof(unsigned int));

            char read_message[] = "READ_FROM_FILE_SECTION";
            to_pipe_string(read_message);
            write(resp_pd, read_message, 23);
            sf_header header;
            if (file_addr == (void *)-1 || shm_addr == (void *)-1)
            {
                char section_status[] = "ERROR";
                to_pipe_string(section_status);
                write(resp_pd, section_status, 6);
            }
            else
            {
                if (!read_sf(file_addr, file_size, &header))
                {
                    char section_status[] = "ERROR";
                    to_pipe_string(section_status);
                    write(resp_pd, section_status, 6);
                }
                else
                {
                    if (section_no_section > header.no_of_sections || offset_section + no_bytes_section >= header.section_headers[section_no_section - 1].sect_size)
                    {
                        free(header.section_headers);
                        char section_status[] = "ERROR";
                        to_pipe_string(section_status);
                        write(resp_pd, section_status, 6);
                    }
                    else
                    {
                        for (int i = 0; i < no_bytes_section; i++)
                            *(char *)(shm_addr + i) = *(char *)(file_addr + offset_section + header.section_headers[section_no_section - 1].sect_offset + i);
                        free(header.section_headers);
                        char section_status[] = "SUCCESS";
                        to_pipe_string(section_status);
                        write(resp_pd, section_status, 8);
                    }
                }
            }
        }
        else if (strcmp(buff, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0)
        {
            unsigned int offset_logical, no_bytes_logical;
            read(req_pd, &offset_logical, sizeof(unsigned int));
            read(req_pd, &no_bytes_logical, sizeof(unsigned int));

            char logical_message[] = "READ_FROM_LOGICAL_SPACE_OFFSET";
            to_pipe_string(logical_message);
            write(resp_pd, logical_message, 31);
            sf_header header;
            if (file_addr == (void *)-1 || shm_addr == (void *)-1)
            {
                char logical_status[] = "ERROR";
                to_pipe_string(logical_status);
                write(resp_pd, logical_status, 6);
            }
            else
            {
                if (!read_sf(file_addr, file_size, &header))
                {
                    char logical_status[] = "ERROR";
                    to_pipe_string(logical_status);
                    write(resp_pd, logical_status, 6);
                }
                else
                {
                    int align = 5120;
                    int current_offset = 0;
                    int new_offset = 0;
                    int i = 0;
                    int done = 0;
                    for (; i < header.no_of_sections; i++)
                    {
                        int reamaing_size = header.section_headers[i].sect_size;
                        new_offset = current_offset;
                        while (reamaing_size > align)
                        {
                            reamaing_size -= align;
                            new_offset += align;
                        }
                        new_offset += align;
                        if (offset_logical < new_offset)
                        {
                            offset_logical -= current_offset;
                            if (offset_logical + no_bytes_logical >= header.section_headers[i].sect_size)
                            {

                                free(header.section_headers);
                                char logical_status[] = "ERROR";
                                to_pipe_string(logical_status);
                                write(resp_pd, logical_status, 6);
                            }
                            else
                            {
                                for (int j = 0; j < no_bytes_logical; j++)
                                    *(char *)(shm_addr + j) = *(char *)(file_addr + offset_logical + header.section_headers[i].sect_offset + j);
                                free(header.section_headers);
                                char logical_status[] = "SUCCESS";
                                to_pipe_string(logical_status);
                                write(resp_pd, logical_status, 8);
                            }
                            done = 1;
                        }
                        if (done)
                            break;
                        current_offset = new_offset;
                    }
                    if (!done)
                    {
                        free(header.section_headers);
                        char logical_status[] = "ERROR";
                        to_pipe_string(logical_status);
                        write(resp_pd, logical_status, 6);
                    }
                }
            }
        }
        else if (strcmp(buff, "EXIT") == 0)
        {
            break;
        }
    }

    close(req_pd);
    close(resp_pd);
    unlink("RESP_PIPE_26934");
    return 0;
}
