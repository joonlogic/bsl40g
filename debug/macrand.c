#include <stdio.h>
#include <string.h>
#include <time.h>

int main(void)
{
	char mac[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};

	for(int i=0; i<6; i++)
		printf("mac[%d] %02X\n", i, mac[i]);

	unsigned int imac = *(unsigned int*)mac;
	imac = htonl(imac);
	printf("imac : %08X -> This is a seed.\n", imac);

	srand(imac);
	unsigned int value1;
	unsigned short value2;
	for(int i=1; i<4; i++) {
		value1 = rand();
		value2 = rand();

		memcpy(mac, &value1, 4);
		memcpy(mac+4, &value2, 2);

		printf("[%d] \n", i);
		for(int j=0; j<6; j++)
			printf("\tmac[%d] %02X\n", j, mac[j]);
	} 

	return 0;
}
