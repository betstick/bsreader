#include "bsreader.hpp"

int main()
{
	char buffer[4];
	BSReader reader = BSReader("../testfile",buffer,4);
	char num[2];

	reader.read(num,2);
	printf("chars: %c%c\n",num[0],num[1]);
	
	reader.read(num,2);
	printf("chars: %c%c\n",num[0],num[1]);

	reader.read(num,2);
	printf("chars: %c%c\n",num[0],num[1]);

	reader.read(num,2);
	printf("chars: %c%c\n",num[0],num[1]);

	reader.seek(2,0);
	reader.read(num,2);
	printf("chars: %c%c\n",num[0],num[1]);
};