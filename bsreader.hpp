#pragma once
#include <stdint.h>
#include <cstdlib>
#include <stdio.h>
#include <cstring>
#include <string>
#include <stdexcept>
#include <bits/stdc++.h> //is this standard?

/*
This reader is designed to function similarly to the fread()
function but with a backend buffer that you don't need to
worry about. It'll handle buffer management to speed up 
reads and prevent some disk thrashing.

Recommended buffer size is around 4096 bytes. You can use more
if you want, but I'm not sure if it'll help or hurt.

Calling stepout() without having stepped in is undefined
behaviour. Don't do it if you value your sanity.

Unless otherwise specified all sizes are in bytes.

Buffer is kept private since touching it is dangerous. It is
supposed to be read from and only filled from the original
file. So writing to it isn't needed. Also no real reason to
read directly from it.

Use fileSize member as a "SEEK_END" for the seek method.
Use 0 as a "SEEK_SET" and position as "SEEK_CUR".
*/

class BSReader
{
    public: //public for use in seek
    uint64_t position = 0; //SEEK_CUR current position of the "read head"
    uint64_t fileSize; //SEEK_END later
    //just use 0 for SEEK_SET

    private:
    uint64_t offset = 0; //buffer start position relative to file in bytes
    
    FILE* file;

    char* buffer; //ptr to arbitrarily sized buffer
    uint64_t bufferSize; //size of said buffer in bytes

	//queue to allow nested stepin/stepouts 
	std::queue<uint64_t> stepStack[64]; 

    public:

	BSReader(){}; //needed a default that did nothing :/

	//uses an internal buffer to be safe.
	BSReader(std::string filePath, size_t buffSize)
	{
		file = fopen(filePath.c_str(),"rb");

		if(file == NULL)
			throw std::runtime_error("no file");

		buffer = new char[buffSize]; //throw buffer onto heap

		//init values
		getSize();
		position = 0;
		offset = 0;
		bufferSize = buffSize;

		//initial buffer fill
		fread(buffer,bufferSize,1,file);
	};

	//destructor, needed cause we mess with the heap!
	~BSReader()
	{
		delete[] buffer;
	};

    void read(void* dest, size_t size)
    {
#ifndef BLEED //these checks are not strictly needed, good code won't need them.
		//out of bounds position
		if(position > fileSize)
			throw std::runtime_error("position larger than file!\n");

		//size is negative, shouldn't be possible?
		/*if(size < 0)
			throw std::runtime_error("invalid target size!\n");*/
#endif

		//if size is greater than the buffer size
		if(size > bufferSize)
		{
			//more janky int math, division rounds down
			uint64_t chunks = size / bufferSize; //how many buffer sizes needed
			uint64_t remainder = size - (chunks * bufferSize); //amount of data left over

			for(uint64_t i = 0; i < chunks; i++)
			{
				read(&dest+(i*bufferSize),bufferSize);
			}

			//read the remainder onto end if needed
			if(remainder != 0)
				read(&dest+(chunks*bufferSize),remainder);
		}
		else
		{
			uint64_t relativePos = offset > 0 ? position - offset : position;
			int64_t bufferEnd = offset + bufferSize; //end byte of buffer
			int64_t overflow = (position + size) - bufferEnd;

			//if target is within buffer
			if(overflow < 0) //thefifthmatt figured this out. very clever
			{
				memcpy(dest,buffer+relativePos,size);
				position += size;

				if(position >= bufferEnd)
					bufferSet();
			}
			else //target DOES NOT end within the buffer
			{
				int64_t early = size - overflow;
				memcpy(dest,buffer+relativePos,early);
				seek(bufferEnd,0);
				memcpy(dest,buffer+early,overflow);
			}
		}
    };

    void seek(uint64_t positionAdjust, int64_t origin)
    {
        //calculate what buffer is needed to be loaded
        position = positionAdjust + origin;

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

        fseek(file,offset,SEEK_SET);

        if(fread(buffer,bufferSize,1,file)==0)
		{
			//if the buffer is GREATER this is normal behavior
			if(feof(file) && bufferSize <= fileSize)
				throw std::runtime_error("EOF");
			
			if(ferror(file))
			{
				printf("bufsz:%i\tofst:%i\n",bufferSize,offset);
				printf("i'm stupid\n");
				throw std::runtime_error("ferror");
			}
		}
    };
};