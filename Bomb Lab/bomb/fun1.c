#include <stdio.h>

int main(int argc, char **argv)
{
    FILE *fp;
	char ch;
// 检查命令行参数个数是否正确
	if (argc != 2) {
		printf("Usage: %s <filename>\n", argv[0]);
		return 1;
	}
   	// 打开文件
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		printf("Failed to open file: %s\n", argv[1]);
		return 1;
	}
				
   	// 读取文件中的字符，并将其翻译成十六进制输出
	while ((ch = fgetc(fp)) != EOF){
		printf("%02x ", ch);            
	}

// 关闭文件
      fclose(fp);
	  return 0;
}
