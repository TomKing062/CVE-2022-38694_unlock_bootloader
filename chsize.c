#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ERR_EXIT(...) \
    do { fprintf(stderr, __VA_ARGS__); exit(1); } while (0)

static uint8_t* loadfile(const char* fn, size_t* num, size_t extra) {
    size_t n, j = 0; uint8_t* buf = 0;
    FILE* fi = fopen(fn, "rb");
    if (fi) {
        fseek(fi, 0, SEEK_END);
        n = ftell(fi);
        if (n) {
            fseek(fi, 0, SEEK_SET);
            buf = (uint8_t*)malloc(n + extra);
            if (buf) j = fread(buf, 1, n, fi);
        }
        fclose(fi);
    }
    if (num) *num = j;
    return buf;
}

int main(int argc, char** argv)
{
    if (argc < 2) ERR_EXIT("Usage: %s <filename>\n", argv[0]);

    char* filename = argv[1];
    uint8_t* mem; size_t size = 0;
    mem = loadfile(filename, &size, 0);
    if (!mem) ERR_EXIT("loadfile(\"%s\") failed\n", filename);
    if ((uint64_t)size >> 32) ERR_EXIT("file too big\n");

    if (!(*(uint64_t*)&mem[0x30])) ERR_EXIT("The file is not sprd trusted firmware\n");
    if (*(uint64_t*)&mem[0x30] + 0x200 > size) ERR_EXIT("The file is not complete\n");
    if (*(uint64_t*)&mem[0x30] != *(uint64_t*)&mem[(*(uint64_t*)&mem[0x30]) + 0x200 + 0x10])
        size = *(uint64_t*)&mem[0x30] + 0x200;
    else if (*(uint64_t*)&mem[(*(uint64_t*)&mem[0x30]) + 0x200 + 0x50])
        size = *(uint64_t*)&mem[(*(uint64_t*)&mem[0x30]) + 0x200 + 0x50] + *(uint64_t*)&mem[(*(uint64_t*)&mem[0x30]) + 0x200 + 0x58];
    else if (*(uint64_t*)&mem[(*(uint64_t*)&mem[0x30]) + 0x200 + 0x30])
        size = *(uint64_t*)&mem[(*(uint64_t*)&mem[0x30]) + 0x200 + 0x30] + *(uint64_t*)&mem[(*(uint64_t*)&mem[0x30]) + 0x200 + 0x38];
    else
        size = *(uint64_t*)&mem[(*(uint64_t*)&mem[0x30]) + 0x200 + 0x20] + *(uint64_t*)&mem[(*(uint64_t*)&mem[0x30]) + 0x200 + 0x28];
    printf("0x%zx\n", size);

    FILE* file = fopen("temp", "wb");  // 以二进制模式打开文件
    if (file == NULL) ERR_EXIT("Failed to create the file.\n");
    size_t bytes_written = fwrite(mem, sizeof(unsigned char), size, file);
    if (bytes_written != size) ERR_EXIT("Failed to write the file.\n");
    fclose(file);

    if (remove(filename)) ERR_EXIT("Failed to delete the file.\n");
    if (rename("temp", filename)) ERR_EXIT("Failed to rename the file.\n");
    free(mem);

    return 0;
}
