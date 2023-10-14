#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 定义EFI分区表头结构体
typedef struct {
    char signature[8]; // 固定为EFI PART
    int revision; // 版本号
    int header_size; // 头部大小
    int header_crc32; // 头部CRC32校验值
    int reserved; // 保留字段
    long long current_lba; // 当前LBA地址
    long long backup_lba; // 备份LBA地址
    long long first_usable_lba; // 第一个可用LBA地址
    long long last_usable_lba; // 最后一个可用LBA地址
    char disk_guid[16]; // 磁盘GUID
    long long partition_entry_lba; // 分区表项起始LBA地址
    int number_of_partition_entries; // 分区表项数量
    int size_of_partition_entry; // 分区表项大小
    int partition_entry_array_crc32; // 分区表项数组CRC32校验值
} efi_header;

// 定义EFI分区表项结构体
typedef struct {
    char partition_type_guid[16]; // 分区类型GUID
    char unique_partition_guid[16]; // 分区唯一GUID
    long long starting_lba; // 分区起始LBA地址
    long long ending_lba; // 分区结束LBA地址
    long long attributes; // 分区属性
    char partition_name[72]; // 分区名称，UTF-16编码
} efi_entry;

// 从文件中读取指定大小的数据到缓冲区中，返回实际读取的字节数，出错则返回-1
int read_data(FILE* fp, void* buffer, int size) {
    if (fp == NULL || buffer == NULL || size <= 0) {
        return -1;
    }
    int bytes_read = fread(buffer, 1, size, fp);
    if (bytes_read != size) {
        if (ferror(fp)) {
            return -1;
        }
    }
    return bytes_read;
}

// 打印分区名称，将UTF-16编码转换为UTF-8编码，忽略非法字符
// 修改后的函数，接收一个char指针作为参数，用于存储分区名
void print_partition_name(char* output, char* name, int size) {
    if (name == NULL || size <= 0 || output == NULL) {
        return;
    }
    // 创建一个变量，用于记录输出数组的索引
    int output_index = 0;
    for (int i = 0; i < size; i += 2) {
        char c1 = name[i];
        char c2 = name[i + 1];
        if (c1 == '\0' && c2 == '\0') { // 结束符
            break;
        }
        if (c2 == '\0') { // ASCII字符
            // 将字符复制到输出数组中
            output[output_index] = c1;
            // 增加输出数组的索引
            output_index++;
        }
        else { // 非ASCII字符，转换为UTF-8编码
            char c3 = name[i + 2];
            char c4 = name[i + 3];
            if (c3 == '\0' && c4 == '\0') { // 只有两个字节的UTF-16编码
                // 将两个字节的UTF-8编码复制到输出数组中
                output[output_index] = (char)(0xc0 | (c2 >> 2));
                output[output_index + 1] = (char)(0x80 | ((c2 & 0x03) << 6) | c1);
                // 增加输出数组的索引
                output_index += 2;
            }
            else { // 四个字节的UTF-16编码，忽略高位字节，只转换低位字节
             // 将两个字节的UTF-8编码复制到输出数组中
                output[output_index] = (char)(0xc0 | (c4 >> 2));
                output[output_index + 1] = (char)(0x80 | ((c4 & 0x03) << 6) | c3);
                // 增加输出数组的索引
                output_index += 2;
                // 跳过高位字节
                i += 2;
            }
        }
    }
    // 在输出数组末尾添加结束符
    output[output_index] = '\0';
}

// 定义扇区大小
#define SECTOR_SIZE 512

// 定义最大扇区数
#define MAX_SECTORS 32

// 主函数，打开文件，读取分区表头和分区表项，打印分区信息，关闭文件，返回0表示成功，返回-1表示失败
int main(int argc,char **argv) {
    FILE* fp = fopen(argv[1], "rb"); // 打开文件，只读二进制模式
    if (fp == NULL) {
        printf("无法打开文件\n");
        return -1;
    }
    efi_header header; // 创建分区表头结构体变量
    int bytes_read; // 读取分区表头数据
    // 创建一个缓冲区，用于存储每个扇区的数据
    char buffer[SECTOR_SIZE];

    // 创建一个变量，用于记录当前扇区的索引
    int sector_index = 0;

    // 创建一个标志，用于记录是否找到了分区表头
    int found = 0;

    // 使用一个循环来遍历每个扇区
    while (sector_index < MAX_SECTORS) {
        // 读取当前扇区的数据到缓冲区中
        bytes_read = read_data(fp, buffer, SECTOR_SIZE);
        if (bytes_read != SECTOR_SIZE) {
            printf("无法读取第%d个扇区\n", sector_index + 1);
            fclose(fp);
            return -1;
        }
        // 检查缓冲区中是否有EFI PART的签名
        if (strncmp(buffer, "EFI PART", 8) == 0) {
            // 找到了分区表头，将缓冲区中的数据复制到分区表头结构体变量中
            memcpy(&header, buffer, sizeof(header));
            // 设置标志为1，表示找到了分区表头
            found = 1;
            // 跳出循环
            break;
        }
        // 没有找到分区表头，继续下一个扇区
        sector_index++;
    }
    // 检查标志是否为1，如果不是，表示没有找到分区表头
    if (found == 0) {
        printf("没有找到分区表头\n");
        fclose(fp);
        return -1;
    }
    int real_SECTOR_SIZE = SECTOR_SIZE * sector_index;
    efi_entry* entries = malloc(header.number_of_partition_entries * sizeof(efi_entry)); // 分配内存空间，存储分区表项数组
    if (entries == NULL) {
        printf("无法分配内存空间\n");
        fclose(fp);
        return -1;
    }
    fseek(fp, header.partition_entry_lba * real_SECTOR_SIZE, SEEK_SET); // 定位到分区表项起始位置
    bytes_read = read_data(fp, entries, header.number_of_partition_entries * sizeof(efi_entry)); // 读取分区表项数组数据
    if (bytes_read != header.number_of_partition_entries * sizeof(efi_entry)) {
        printf("only read %d/%d\n", bytes_read, header.number_of_partition_entries * sizeof(efi_entry));
    }
    FILE* fo = fopen("partition.xml", "wb");
    fprintf(fo,"<Partitions>\n");
    for (int i = 0; i < header.number_of_partition_entries; i++) { // 遍历分区表项数组，打印每个分区的信息
        efi_entry entry = entries[i];
        if (entry.starting_lba == 0 && entry.ending_lba == 0) { // 空闲分区，跳过
            continue;
        }
        char name[72];
        print_partition_name(name, entry.partition_name, sizeof(entry.partition_name)); // 打印分区名称，转换编码
        long long partition_size = entry.ending_lba - entry.starting_lba + 1; // 计算分区大小，单位为扇区数
        double partition_size_mb = partition_size * real_SECTOR_SIZE / (1024 * 1024); // 转换为MB单位
        if(strcmp(name, "userdata")) fprintf(fo, "\t<Partition id=\"%s\" size=\"%d\"/>\n", name, (int)partition_size_mb);
        else fprintf(fo, "\t<Partition id=\"%s\" size=\"0xFFFFFFFF\"/>\n", name);
    }
    fprintf(fo, "</Partitions>");
    fclose(fo);
    free(entries); // 释放内存空间
    fclose(fp); // 关闭文件
    return 0;
}
