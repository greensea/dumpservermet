/**
 * 作者：greensea (gs@bbxy.net)
 * 时间：2011年12月25日
 * 授权：BSD Lisence
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


#ifndef inet_ntoa
const char* inet_ntoa(uint32_t ip) {
	static char ipstr[19];
	snprintf(ipstr, 18, "%u.%u.%u.%u", *(0 + (unsigned char*)&ip), *(1 + (unsigned char*)&ip), *(2 + (unsigned char*)&ip), *(3 + (unsigned char*)&ip));
	ipstr[18] = 0;
	
	return (const char*)ipstr;
}
#endif

int main(int argc, char** argv) {
	FILE* fp;
	
	if (argc <= 1) {
		printf("Usage: %s [filepath]\n", basename(argv[0]));
		return 0;
	}
	
	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		fprintf(stderr, "Could not open file: %s\n", argv[1]);
		return -1;
	}
	
	// 读文件头
	unsigned char file_magic;
	file_magic = fgetc(fp);
	if (file_magic != 0xE0 && file_magic != 0x0E) {
		fprintf(stderr, "Not a valid server.met file\n");
		fclose(fp);
		return -1;
	}
	
	uint32_t server_count;
	fread(&server_count, sizeof(uint32_t), 1, fp);
	
	// 读服务器表
	int i;
	for (i = 0; i < server_count; i++) {
		// 读服务器节点头
		uint32_t ip, tag_count;
		uint16_t port;
		
		fread(&ip, sizeof(uint32_t), 1, fp);
		fread(&port, sizeof(uint16_t), 1, fp);
		fread(&tag_count, sizeof(uint32_t), 1, fp);
		
		printf("server: #%d\n\taddress: %s:%u\n\ttags count: %u\n", i + 1, inet_ntoa(ip), port, tag_count);
		
		// 读标签并跳过去

		int k;
		for (k = 0; k < tag_count; k++) {
		
			unsigned char value[1024];
			uint16_t name_len, value_len;
			unsigned char tag_type;
			
			// 标签类型
			tag_type = fgetc(fp);

			printf("\t\ttag %d, type = %u\n", k + 1, tag_type);
			
			// 标签名字
			fread(&name_len, sizeof(uint16_t), 1, fp);
			memset(value, 0x00, 1024);
			printf("\t\t\tname(%u): ", name_len);
			fread(value, name_len, 1, fp);

			if (name_len == 1) {
				printf("{0x%02X}\n", value[0]);
			}
			else {
				printf("%s\n", value);
			}
			
			// 标签值
			if (tag_type == 0x02) {	// string
				fread(&value_len, sizeof(uint16_t), 1, fp);
				memset(value, 0x00, 1024);
				printf("\t\t\tstring(%u): ", value_len);
				fread(value, value_len, 1, fp);
				printf("%s\n", value);
			}
			else if (tag_type == 0x03) {	// unsigned 32 bit
				printf("\t\t\tuint32_t: ");
				uint32_t value32;
				fread(&value32, 4, 1, fp);
				printf("%u\n", value32);
			}
			else if (tag_type == 0x08) {	// unsigned 16 bit
				printf("\t\t\tuint16_t: ");
				uint16_t value16;
				fread(&value16, 2, 1, fp);
				printf("%u\n", value16);
			}
			else if (tag_type == 0x09) {	// unsigned 8 bit
				printf("\t\t\tuint8_t: ");
				uint8_t value8;
				fread(&value8, 2, 1, fp);
				printf("%u\n", value8);
			}
			else {	// FIXME: see http://hydranode.com/docs/ed2k/ed2k-tag.php#desc
				fprintf(stderr, "Tag type %u not supported\n", tag_type);
				fclose(fp);
				return -1;
			}
		}
	}
	
	fclose(fp);
	return 0;
}

