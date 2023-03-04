#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define RES_INFO_SIZE 24
#define RES_FILE_SIZE 32

/*
    Resource types

    RECT RES_INFO[RES_INFO_SIZE] X Y W H
    POS2 RES_INFO[RES_INFO_SIZE] X Y
    BMAP RES_INFO[RES_INFO_SIZE] FILE[RES_FILE_SIZE]
    BMPA RES_INFO[RES_INFO_SIZE] FILE[RES_FILE_SIZE] R G B
    AUDO RES_INFO[RES_INFO_SIZE] FILE[RES_FILE_SIZE] LOOP_START LOOP_END
*/

static uint8_t buf[1024*64] = {0};
uint16_t buf_pos = 0;

int main(int argc, char** argv)
{
    FILE* fin = fopen(argv[1], "rb");
    
    uint16_t* file_size = (uint16_t*)&buf[0];
    uint16_t* res_count = (uint16_t*)&buf[2];
    buf_pos = 4;

    while(1)
    {
        uint8_t type[4] = {0};
        fscanf(fin, "%4s", type);
        
        if(!strncmp(type, "RECT", 4))
        {
            memcpy(&buf[buf_pos], "RECT", 4); buf_pos += 4;
            
            fscanf(fin, "%s %d %d %u %u\n",
                   &buf[buf_pos],
                   &buf[buf_pos+RES_INFO_SIZE], &buf[buf_pos+RES_INFO_SIZE+2],
                   &buf[buf_pos+RES_INFO_SIZE+4], &buf[buf_pos+RES_INFO_SIZE+6]);
                   
            printf("RECT - %*s %d %d %u %u\n", RES_INFO_SIZE, &buf[buf_pos],
                   *(int16_t*)&buf[buf_pos+RES_INFO_SIZE], *(int16_t*)&buf[buf_pos+RES_INFO_SIZE+2],
                   *(uint16_t*)&buf[buf_pos+RES_INFO_SIZE+4], *(uint16_t*)&buf[buf_pos+RES_INFO_SIZE+6]);
            
            buf_pos += RES_INFO_SIZE + 2 + 2 + 2 + 2;
        }
        else if(!strncmp(type, "POS2", 4))
        {
            memcpy(&buf[buf_pos], "POS2", 4); buf_pos += 4;
            
            fscanf(fin, "%s %d %d\n",
                   &buf[buf_pos], &buf[buf_pos+RES_INFO_SIZE], &buf[buf_pos+RES_INFO_SIZE+4]);
                   
            printf("POS2 - %*s %d %d\n", RES_INFO_SIZE, &buf[buf_pos],
                   *(int32_t*)&buf[buf_pos+RES_INFO_SIZE], *(int32_t*)&buf[buf_pos+RES_INFO_SIZE+4]);
            
            buf_pos += RES_INFO_SIZE + 4 + 4;
        }
        else if(!strncmp(type, "BMAP", 4))
        {
            memcpy(&buf[buf_pos], "BMAP", 4); buf_pos += 4;
            
            fscanf(fin, "%s %s\n",
                   &buf[buf_pos], &buf[buf_pos+RES_INFO_SIZE]);
            
            printf("BMAP - %*s %*s\n",
                   RES_INFO_SIZE, &buf[buf_pos],
                   RES_FILE_SIZE, &buf[buf_pos+RES_INFO_SIZE]);
            
            buf_pos += RES_INFO_SIZE + RES_FILE_SIZE;
        }
        else if(!strncmp(type, "BMPA", 4))
        {
            memcpy(&buf[buf_pos], "BMPA", 4); buf_pos += 4;
            
            fscanf(fin, "%s %s %hhu %hhu %hhu\n",
                   &buf[buf_pos], &buf[buf_pos+RES_INFO_SIZE],
                   &buf[buf_pos+RES_INFO_SIZE+RES_FILE_SIZE],
                   &buf[buf_pos+RES_INFO_SIZE+RES_FILE_SIZE+1],
                   &buf[buf_pos+RES_INFO_SIZE+RES_FILE_SIZE+2]); 
                   
            printf("BMPA - %*s %*s %hhu %hhu %hhu\n",
                   RES_INFO_SIZE, &buf[buf_pos],
                   RES_FILE_SIZE, &buf[buf_pos+RES_INFO_SIZE],
                   *(uint8_t*)&buf[buf_pos+RES_INFO_SIZE+RES_FILE_SIZE],
                   *(uint8_t*)&buf[buf_pos+RES_INFO_SIZE+RES_FILE_SIZE+1],
                   *(uint8_t*)&buf[buf_pos+RES_INFO_SIZE+RES_FILE_SIZE+2]); 
            
            buf_pos += RES_INFO_SIZE + RES_FILE_SIZE + 3;
        }
        else if(!strncmp(type, "AUDO", 4))
        {
            memcpy(&buf[buf_pos], "AUDO", 4); buf_pos += 4;
            
            fscanf(fin, "%s %s %u %u\n",
                   &buf[buf_pos], &buf[buf_pos+RES_INFO_SIZE],
                   &buf[buf_pos+RES_INFO_SIZE+RES_FILE_SIZE],
                   &buf[buf_pos+RES_INFO_SIZE+RES_FILE_SIZE+4]); 
                   
            printf("AUDO - %*s %*s %u %u\n",
                   RES_INFO_SIZE, &buf[buf_pos],
                   RES_FILE_SIZE, &buf[buf_pos+RES_INFO_SIZE],
                   *(uint32_t*)&buf[buf_pos+RES_INFO_SIZE+RES_FILE_SIZE],
                   *(uint32_t*)&buf[buf_pos+RES_INFO_SIZE+RES_FILE_SIZE+4]); 
            
            buf_pos += RES_INFO_SIZE + RES_FILE_SIZE + 4 + 4;
        }
        else
        {
            printf("Unknown type: %4s\n", type);
            printf("Pos: %u\n", buf_pos);
            break;
        }
        
        *res_count+=1;
        *file_size = buf_pos;
    }

    fclose(fin);
 
    uint32_t name_size = strlen(argv[1]);
    uint8_t* output_name = calloc(name_size, 1);
    memcpy(output_name, argv[1], name_size); 
    output_name[name_size-4] = '.';
    output_name[name_size-3] = 'D';
    output_name[name_size-2] = 'A';
    output_name[name_size-1] = 'T';
    
    FILE* fout = fopen(output_name, "wb");
    
    fwrite(buf, buf_pos, 1, fout);
    
    fclose(fout);
    
    free(output_name);
 
    return 0;
}

/*
int main(int argc, char** argv)
{
    FILE* fin = fopen(argv[1], "rb");
    FILE* fout = fopen(argv[2], "wb");
    
    uint8_t video_count = 0;
    fscanf(fin, "%hhu\n", &video_count);
    printf("Video count: %hhu\n", video_count);
    
    fwrite(&video_count, sizeof(uint8_t), 1, fout);
    
    for(uint8_t i = 0; i != video_count; ++i)
    {
        int16_t x, y;
        uint16_t w, h;
        uint8_t str[255] = {0};
        
        fscanf(fin, "%hd %hd %hu %hu %s\n", &x, &y, &w, &h, str);
        printf("%hd %hd %hu %hu %s\n", x, y, w, h, str);
        
        uint8_t str_len = strlen(str);
        
        fwrite(&x, sizeof(int16_t), 1, fout);
        fwrite(&y, sizeof(int16_t), 1, fout);
        fwrite(&w, sizeof(uint16_t), 1, fout);
        fwrite(&h, sizeof(uint16_t), 1, fout);
        fwrite(&str_len, sizeof(uint8_t), 1, fout);
        fwrite(str, 1, str_len, fout);
    }
    
    fclose(fin);
    fclose(fout);
    
    return 0;
}
*/