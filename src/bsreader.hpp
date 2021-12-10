#pragma once
#include <stdint.h>
#include <cstdlib>
#include <stdio.h>
#include <cstring>
#include <string>
#include <stdexcept>

/*
This reader is designed to function similarly to the fread()
function but with a backend buffer that you don't need to
worry about other than allocating it when you create the
reader instance. it'll handle buffer management to speed
up reads and prevent some disk thrashing.
*/
//enum location = {SEEK_CUR};

class BSReader
{
    public: //public for use in seek
    uint64_t position = 0; //SEEK_CUR current position of the "read head"
    uint64_t fileSize; //SEEK_END later
    //just use 0 for SEEK_SET

    private:
    uint64_t offset = 0; //buffer position relative to file
    
    FILE* file;

    char* buffer;
    uint64_t bufferSize;

    public:

    BSReader(std::string filePath, void* bufferPtr, size_t size)
    {
        file = fopen(filePath.c_str(),"rb");

		if(file == NULL)
			throw std::runtime_error("no file");

        //TODO: get filesize, needed for sanity checks
        position = 0;
		offset = 0;
        bufferSize = size;
		buffer = reinterpret_cast <char *>(bufferPtr);
        fread(buffer,bufferSize,1,file);
		//printf("BSReader object initialized:%i\n",rand());
    };

    void read(void* dest, size_t size)
    {
        //uint64_t bufferStart = offset;
        //uint64_t bufferEnd = offset + bufferSize;

        //uint64_t targetStart = position;
        //uint64_t targetEnd = position + size;

        //uint64_t relativePos = offset > 0 ? position - offset : position;

        //targetStart MUST be positive, so never have to rewind!
        //targetStart CANNOT be greater than bufferSize! 

		uint64_t relativePos = offset > 0 ? position - offset : position;
		//printf("relativePos initialized:%i\n",rand());

        //if target is within buffer
        if(position + size < offset + bufferSize)
        {
            memcpy(dest,buffer+relativePos,size);
            position += size;
			//printf("memcpy worked, %i\n", rand());
			//printf("memcpy initialized relpos: %i\n",relativePos);

            if(position >= offset + bufferSize)
                bufferSet();
        }
        else //target DOES NOT end within the buffer
        {
			//printf("request outside buffer! read size: %i\t\t%i\n",size,rand());
			int64_t overflow = (position + size) - (offset + bufferSize);
			int64_t early = size - overflow;
			memcpy(dest,buffer+relativePos,early);
			seek(bufferSize+offset,0);
			memcpy(dest,buffer+early,overflow);
        }
    };

    void seek(int64_t positionAdjust, uint64_t origin)
    {
        //calculate what buffer is needed to be loaded
        position = positionAdjust + origin;

        //pos not within current buffer
        if(position >= offset + bufferSize || position < offset)
        {
            //because ints, this eliminates remainders.
            //there's probably a math function to do this.
            offset = (position / bufferSize) * bufferSize;
			//printf("Set offset to:%i",offset);
        }
        else
        {
			//printf("no adjust needed Pos: %i\t Ofst: %i\n",position,offset);
            //if new pos is inside the current buffer do nothing
        }

        bufferSet(); //fill buffer based on new offset
    };
	
	void debug()
	{
		printf("Pos: %i\tBuffSize: %i,\tOffset: %i\n",position,bufferSize,offset);
	};

    private:

    //this may not be needed?
    //move the buffer n times the buffer size
    void bufferAdjust(int64_t adjustment)
    {
        offset += adjustment * bufferSize; //modify first
        fseek(file,offset,SEEK_SET);
        fread(buffer,bufferSize,1,file); //fill buffer
    };

    //refills buffer based on offset
    void bufferSet()
    {
        fseek(file,offset,SEEK_SET);
        fread(buffer,bufferSize,1,file);
    };
};