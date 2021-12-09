#pragma once
#include "common/stdafx.hpp"

/*
This reader is designed to function similarly to the fread()
function but with a backend buffer that you don't need to
worry about other than allocating it when you create the
reader instance. it'll handle buffer management to speed
up reads and prevent some disk thrashing.
*/
enum location = {SEEK_CUR};

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

    BSReader(std::string filePath, char* buffer, size_t size)
    {
        file = fopen64(filePath.c_str(),"br");
        //TODO: get filesize, needed for sanity checks
        position = 0;
        bufferSize = size;
        fread(buffer,bufferSize,1,file);
    };

    void bsread(void* dest, size_t size)
    {
        //uint64_t bufferStart = offset;
        //uint64_t bufferEnd = offset + bufferSize;

        //uint64_t targetStart = position;
        //uint64_t targetEnd = position + size;

        //uint64_t relativePos = offset > 0 ? position - offset : position;

        //targetStart MUST be positive, so never have to rewind!
        //targetStart CANNOT be greater than bufferSize! 

        //check that targetStart is within the buffer. if prior, rewind
        //the offset and start over.

        while(position + size > offset + bufferSize)
        {
            //go forward by bufferSize
            //recalc 
        }
    };

    void seek(int64_t positionAdjust, uint64_t origin)
    {
        //calculate what buffer is needed to be loaded
        position = positionAdjust + origin;

        //new pos higher than buffer
        if(position > offset + bufferSize)
        {
            //because ints, this eliminates remainders.
            //there's probably a math function to do this.
            offset = (position / bufferSize) * bufferSize;
        }
        //new pos lower than buffer
        else if(position < offset)
        {
            //i guess this works for both conditions?
            offset = (position / bufferSize) * bufferSize;
        }
        else
        {
            //if new pos is inside the current buffer do nothing
        }

        bufferSet();
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