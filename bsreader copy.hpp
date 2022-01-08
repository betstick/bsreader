#pragma once
#include <stdint.h>
#include <cstdlib>
#include <stdio.h>
#include <cstring>
#include <string>
#include <stdexcept>
#include <bits/stdc++.h> //is this standard?
#ifdef DEBUG
	#include <random>
	#include <time.h>
#endif

class BSReader
{
    public:
    uint64_t position = 0; //current pos of the "read head"
    uint64_t fileSize = 0; //total size of the file in bytes

    private:
    uint64_t offset = 0; //buffer start pos relative to file in bytes
    
    FILE* file = nullptr;

    char* buffer = nullptr; //ptr to arbitrarily sized buffer
    uint64_t bufferSize = 0; //size of said buffer in bytes

	//queue to allow nested stepin/stepouts 
	std::queue<uint64_t> stepStack[64]; 

    public:

	BSReader(){}; //needed a default that did nothing :/

	//uses an internal buffer to be safe.
	BSReader(std::string filePath, uint64_t buffSize)
	{
		file = fopen(filePath.c_str(),"rb");

		if(file == NULL || file == nullptr)
			throw std::runtime_error("failed to open file!\n");

		bufferSize = buffSize;
		buffer = new char[bufferSize]; //throw buffer onto heap

		//init values
		getSize();

		//initial buffer fill
		fread(buffer,bufferSize,1,file);
	};

	//destructor, needed cause we mess with the heap!
	~BSReader()
	{
#ifdef DEBUG
		srand(time(NULL));
		printf("DESTROYED_BSREADER %i\n",rand());
#endif
		delete[] buffer;
	};

    void read(void* dest, size_t size)
    {
		uint32_t fullReads = 1;
		uint32_t remainder = 0;

		//divides total read into buffer sized chunks with a remainder
		if(size > bufferSize)
		{
			fullReads = size / bufferSize;
			remainder = size - (fullReads * bufferSize);
		}

		uint32_t totalReads = remainder > 0 ? fullReads + 1 : fullReads;

		for(uint32_t i = 0; i < totalReads; i++)
		{
			//position of header relative to the start of the buffer
			uint64_t buffRelativePos = offset > 0 ? position - offset : position;

			uint64_t bufferEnd = offset + bufferSize; //end byte of buffer
			uint64_t overflow = (position + size) - bufferEnd; //bytes past buffer end
			
			if(overflow < 0) //if read is within buffer
			{
				memcpy(&dest,buffer+buffRelativePos,size);
				seek(size,0);
			}
			else //read extends beyond buffer
			{
				uint64_t early = size - overflow;
				memcpy(dest,buffer+buffRelativePos,early);
				seek(bufferEnd,0);
				memcpy(dest,buffer+early,overflow);
			}
		};

		/*
		things i care about:
		read head location
		buffer start location
		buffer size

		following can be derived:
		
		buffer end location
		*/

		uint64_t readEnd = position + size; //end of read request
		uint64_t bufferEnd = offset + bufferSize; //last byte of buffer
		uint64_t bufferPos = offset > 0 ? position - offset : position;

		if(readEnd > bufferEnd) //entire read is not within buffer
		{
			uint64_t frontSize = 0;
			uint64_t fullReads = 0;
			uint64_t endSize = 0;

			bufferEnd - position //frontsize to end of buffer
		}
		else
		{
			memcpy(&dest,buffer+bufferPos,size);
			seek(size,0);
		}
    };

    void seek(uint64_t positionAdjust, uint64_t origin)
    {
        //calculate what buffer is needed to be loaded
        position = (uint64_t)(positionAdjust + origin);

        //pos not within current buffer
        if(position >= offset + bufferSize || position < offset)
        {
            //because ints, this eliminates remainders.
            //there's probably a math function to do this.
            offset = (position / bufferSize) * bufferSize;
        }

        bufferSet(); //fill buffer based on new offset
    };

	void stepIn(uint64_t targetPosition)
	{
		stepStack->push(position);
		seek(targetPosition,0);
	};

	void stepOut()
	{
		seek(stepStack->front(),0);
		stepStack->pop();
	};

    private:
	void getSize()
	{
		fseek(file,0,SEEK_END);
		fileSize = ftell(file);
		rewind(file);
		
		printf("Filesize: %i bytes\n",fileSize);
	};

    void bufferSet() //refills buffer based on offset
    {
#ifdef DEBUG
		//printf("offset: 0x%x\t\t%i\n",offset,offset);
#endif 
		if(offset + bufferSize >= fileSize)
		{
			printf("offset: 0x%x\t\t%i\n",offset,offset);
			throw std::runtime_error("EOF\n");
		}

		if(file != NULL)
		{
        	fseek(file,offset,SEEK_SET);
		}
		else
			throw std::runtime_error("file became null!?!?\n");

        if(fread(buffer,bufferSize,1,file)==0)
		{
			//if the buffer is GREATER this is normal behavior
			if(feof(file) && bufferSize <= fileSize)
				throw std::runtime_error("EOF\n");
			
			if(ferror(file))
			{
				printf("bufsz:%i\tofst:%i\n",bufferSize,offset);
				printf("i'm stupid\n");
				throw std::runtime_error("ferror\n");
			}
		}
    };
};