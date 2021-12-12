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

class BSReader
{
    public: //public for use in seek
    uint64_t position = 0; //SEEK_CUR current position of the "read head"
    uint64_t fileSize; //SEEK_END later
    //just use 0 for SEEK_SET

    private:
    uint64_t offset = 0; //buffer start position relative to file
    
    FILE* file;

    char* buffer; //ptr to arbitrarily sized buffer
    uint64_t bufferSize;

    public:

	BSReader(){}; //needed a default that did nothing :/

    BSReader(std::string filePath, void* bufferPtr, size_t buffSize)
    {
        file = fopen(filePath.c_str(),"rb");

		if(file == NULL)
			throw std::runtime_error("no file");

        //TODO: get filesize, needed for sanity checks
        position = 0;
		offset = 0;
        bufferSize = buffSize;
		buffer = reinterpret_cast <char *>(bufferPtr);
        fread(buffer,bufferSize,1,file);
    };

    void read(void* dest, size_t size)
    {
		if(size > bufferSize || size < 0)
			throw std::runtime_error("invalid target size!");
			
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
    };

    void seek(int64_t positionAdjust, int64_t origin)
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

    private:
    
    void bufferSet() //refills buffer based on offset
    {
        fseek(file,offset,SEEK_SET);
        if(fread(buffer,bufferSize,1,file)==0)
			throw std::runtime_error("EOF");
    };
};