#include "bsreader.hpp"
#include <random>
#include <time.h>

int main()
{
	time_t time;
	srand(time);
	//printf("rand:%i\n",rand());

	char buffer[4];
	BSReader reader = BSReader("../testfile",buffer,4);
	char num[2];

	reader.debug();
	reader.read(num,2);
	//printf("buffer: %i\n",buffer[1]);
	//reader.debug();
	printf("chars: %c%c\n",num[0],num[1]);
	
	reader.debug();
	reader.read(num,2);
	printf("chars: %c%c\n",num[0],num[1]);
	//reader.debug();
	reader.debug();
	reader.read(num,2);
	printf("chars: %c%c\n",num[0],num[1]);
	//reader.debug();

	reader.debug();
	reader.read(num,2);
	printf("chars: %c%c\n",num[0],num[1]);

	reader.seek(2,0);
	reader.debug();
};