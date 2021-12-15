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

	//use for higher level stuff i guess like the real time reader?
    BSReader(std::string filePath, void* bufferPtr, size_t buffSize)
    {
        file = fopen(filePath.c_str(),"rb");

		if(file == NULL)
			throw std::runtime_error("no file");

		getSize(); //sets the filesize
        position = 0;
		offset = 0;
        bufferSize = buffSize;
		buffer = reinterpret_cast <char *>(bufferPtr);
        fread(buffer,bufferSize,1,file);
    };

	//uses an internal buffer, pointless, don't delete, might want later
	/*BSReader(std::string filePath, size_t buffSize)
	{
		file = fopen(filePath.c_str(),"rb");

		if(file == NULL)
			throw std::runtime_error("no file");

		buffer = (char*)malloc(buffSize);

		position = 0;
		offset = 0;
		bufferSize = buffSize;

		fread(buffer,bufferSize,1,file);
	};*/

    void read(void* dest, size_t size)
    {
		if(position > fileSize)
			throw std::runtime_error("position larger than file!\n");

		if(size < 0)
			throw std::runtime_error("invalid target size!\n");

		if(size > bufferSize)// || size < 0)
		{	
			//TODO: add support for going over the buffer size!
			printf("size was: %i\n",size);
			throw std::runtime_error("ADD SUPPORT FOR GOIGN OVER!!!!!\n");
		}
			
		uint64_t relativePos = offset > 0 ? position - offset : position;
		int64_t bufferEnd = offset + bufferSize; //end byte of buffer
		int64_t overflow = (position + size) - bufferEnd;

        //if target is within buffer
		if(overflow < 0) //thefifthmatt figured this out. very clever
        {
			/*if((offset + bufferSize) + ((position + size)-offset + bufferSize) == position)
				throw std::runtime_error("its the bug\n");*/
			//sometimes causes segfaults,
			//not caused by position convergence
			printf(
				"pos:%i+off:%i\trelPos:%i\tfSize:%i\toflow:%i\tbuffEnd:%i\n",
				position,offset,relativePos,fileSize,overflow,bufferEnd
			);
			printf("src:%i\n",buffer+relativePos);
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

	void getSize()
	{
		fseek(file,0,SEEK_END);
		fileSize = ftell(file);
		rewind(file);
		
		printf("Filesize: %i bytes\n",fileSize);
	};

    private:
    
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