#include <stdio.h>
#include <stdlib.h>

unsigned char table1[16] = {3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12};
unsigned char table3[16] = {3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8, 15, 14, 13, 12};

unsigned char table2[16] = {5, 2, 3, 0, 1, 14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4};
unsigned char table4[16] = {3, 4, 1, 2, 15, 0, 13, 14, 11, 12, 9, 10, 7, 8, 5, 6};

void encrypt(unsigned char *input, unsigned char *output, int len)
{
    unsigned char temp;
    for (int i = 0; i < len; i++)
    {
        temp = table2[(input[i] + 2) % 256 / 16] * 16;
        temp += table1[(input[i] + 2) % 16];
        output[i] = temp;
    }
}

void decrypt(unsigned char *input, unsigned char *output, int len)
{
    unsigned char temp;
    for (int i = 0; i < len; i++)
    {
        temp = table4[input[i] / 16] * 16;
        temp += table3[input[i] % 16];
        output[i] = temp - 2;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage: %s e|d input_file output_file\n", argv[0]);
        exit(1);
    }

    FILE *fp_in = fopen(argv[2], "rb");
    if (fp_in == NULL)
    {
        perror("Error: cannot open input file");
        exit(2);
    }

    FILE *fp_out = fopen(argv[3], "wb");
    if (fp_out == NULL)
    {
        perror("Error: cannot create output file");
        exit(3);
    }

    unsigned char c;

    if (argv[1][0] == 'e')
    {
        while (fread(&c, 1, 1, fp_in) == 1)
        {
            encrypt(&c, &c, sizeof(c));
            fwrite(&c, sizeof(c), sizeof(c), fp_out);
        }
    }
    else if (argv[1][0] == 'd')
    {
        while (fread(&c, sizeof(c), sizeof(c), fp_in) == sizeof(c))
        {
            decrypt(&c,&c,sizeof(c));
            fwrite(&c,sizeof(c),sizeof(c),fp_out);
        }
    }
    else
    {
        printf("Error: invalid option %s\n", argv[1]);
        exit(4);
    }

    fclose(fp_in);
    fclose(fp_out);

    return 0;
}
