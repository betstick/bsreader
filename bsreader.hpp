#pragma once
#include <stdint.h>
#include <cstdlib>
#include <stdio.h>
#include <cstring>
#include <string>
#include <stdexcept>
#include <bits/stdc++.h> //is this standard?
#include <wchar.h> //needed for fmemopen

//Buffered file reader utility. Supercedes stdio reader.
//Pick one buffer size and stick with it. Switching files
//and buffer sizes may cause strange behavior. I dunno.
class BSReader
{
	private:
	FILE* file;
	char* buffer;
	uint64_t bufferSize = 0;
	uint64_t bufferPos = 0;
	uint64_t writeOffset = 0;

	std::queue<uint64_t> stepStack[64];
	std::queue<uint64_t> markStack[64];

	int32_t errorStatus = 0; //0 is none, -1 is EOF, -2 is unk.

    public:
	bool isFile; //false means its memory
	uint64_t fileSize = 0;
	uint64_t readPos = 0;

	BSReader(){};

	BSReader(std::string filePath, uint64_t bufferSize)
	{
		isFile = true;
		file = fopen(filePath.c_str(),"rb");

		if(file == NULL)
			throw std::runtime_error("Failed to open file!\n");

		this->bufferSize = bufferSize;
		buffer = new char[bufferSize];
		bufferAutoAdjust();
	};

	BSReader(void* buffer, size_t size)
	{
		isFile = false;
		file = fmemopen(buffer,size,"rb");

		if(file == NULL)
			throw std::runtime_error("Failed to open memory!\n");
	
		this->bufferSize = 0;
		bufferAutoAdjust();
	};

	~BSReader()
	{
		delete[] buffer;
	};

	//Don't use this. It's for testing '_>'
	void open(std::string filePath, uint64_t bufferSize)
	{
		cleanup();

		isFile = true;
		file = fopen(filePath.c_str(),"rb");

		if(file == NULL)
			throw std::runtime_error("Failed to open file!\n");

		this->bufferSize = bufferSize;
		buffer = new char[bufferSize];
		bufferAutoAdjust();
	};

	void open(void* buffer, size_t size)
	{
		cleanup();

		isFile = false;
		file = fmemopen(buffer,size,"rb");

		if(file == NULL)
			throw std::runtime_error("Failed to open memory!\n");
	
		this->bufferSize = 0;
		bufferAutoAdjust();
	};

	//reset values back to default
	void cleanup()
	{
		//init back to zero
		bufferPos = 0;
		writeOffset = 0;
		readPos = 0;
		errorStatus = 0;

		//clear the buffer as well
		if(buffer != NULL)
			delete buffer;

		//clear these out to prevent strange behavior
		*stepStack = std::queue<uint64_t>();
		*markStack = std::queue<uint64_t>();
	};

	//Copies readSize number of bytes to dest pointer. Mimics fread().
	//Returns readSize on success. Does unsafe things on failure and returns 0.
	int read(void* dest, uint64_t readSize)
	{
		if(isFile)
		{
			//Design: a read can be broken into 3 distinct parts.
			//Part 1 is within the current buffer and may or may not stretch to the end.
			//Part 2 is the number of full buffer reads needed after the part 1.
			//Part 3 is anything needed after 1 and 2 are finished.
			//All reads will have part one, part 2 and 3 are dependent on read size.

			//gets distance between end of buffer, and current read position
			bool readIsTooBig = (bufferPos + bufferSize) - readPos < readSize;

			uint64_t startSize = readIsTooBig ? (bufferPos+bufferSize) - readPos : readSize;

			//Initial read. May be incomplete and need subsequent reads.
			memcpy(dest,buffer+(readPos-bufferPos),startSize);
			readPos += startSize;
			writeOffset = startSize;
			bufferAutoAdjust();
			
			if(readIsTooBig)
			{
				uint64_t fullReads = (readSize - startSize) / bufferSize;

				for(uint64_t i = 0; i < fullReads; i++)
				{
					//Full buffer reads. Ran as many times as needed.
					memcpy((char*)dest+writeOffset,buffer+(readPos-bufferPos),bufferSize);
					readPos += bufferSize;
					writeOffset += bufferSize;
					bufferAutoAdjust();
				}

				//i knew how this line worked for 1.2 seconds. it does work though. trust me.
				uint64_t remainder = (readSize - (fullReads * bufferSize)) - startSize;

				if(remainder > 0)
				{
					//Ending read.
					memcpy((char*)dest+writeOffset,buffer+(readPos-bufferPos),remainder);
					readPos += remainder;
					bufferAutoAdjust();
				}
			}
		}
		else
		{
			fread(dest,readSize,1,file);

			if(feof(file) != 0)
				this->errorStatus = -1;
			if(ferror(file) != 0)
				this->errorStatus = -2;
		}

		if(this->errorStatus == 0)
			return readSize;
		else
			return 0;
	};

	private:
	//Sets the correct buffer position in the file.
	//Returns 0 on seccess, -1 on failure.
	void setBufferPosition(uint64_t position)
	{
		if(isFile)
		{
			bufferPos = position;
			fseek(file,bufferPos,SEEK_SET);
			refillBuffer();
		}
	};

	//Fills buffer based on position in file.
	//0 on success, -1 on error.
	void refillBuffer()
	{
		fread(buffer,bufferSize,1,file);

		if(feof(file) != 0)
			this->errorStatus = -1;
		if(ferror(file) != 0)
			this->errorStatus = -2;
	};

	//Calculates the correct buffer position.
	//0 on success, -1 on error.
	void bufferAutoAdjust()
	{
		setBufferPosition((readPos / bufferSize) * bufferSize);

		if(!isFile) //if its memory
		{
			fseek(file,readPos,SEEK_SET);
			
			if(feof(file) != 0)
				this->errorStatus = -1;
			if(ferror(file) != 0)
				this->errorStatus = -2;
		}
	};

	public:
	//Returns size of file, also sets the member variable.
	uint64_t getSize()
	{
		fseek(file,0,SEEK_END);
		fileSize = ftell(file);
		rewind(file);
		//put bufferpos back where it was :)
		fseek(file,bufferPos,SEEK_SET);
		return fileSize;
	};

	//Place current read position in step queue. Travel to target.
	void stepIn(uint64_t targetPosition)
	{
		stepStack->push(readPos);
		readPos = targetPosition;
		bufferAutoAdjust();
	};

	//Return to top of step queue.
	void stepOut()
	{
		readPos = stepStack->front();
		stepStack->pop();
		bufferAutoAdjust();
	};

	//Place current read position in marker queue. Stays there.
	void markPos()
	{
		markStack->push(readPos);
	};

	//Return to top of marker queue.
	void returnToMark()
	{
		readPos = markStack->front();
		markStack->pop();
		bufferAutoAdjust();
	};

	//Travel to a specified location in the file.
	void seek(uint64_t targetPosition)
	{
		readPos = targetPosition;
		bufferAutoAdjust();
	};

	//Unsupported, in progress. May segfault.
	void seek(int64_t offset, int whence)
	{
		uint64_t position = 0;
		switch(whence)
		{
			case 0:
				position = (offset > 0 ? offset : 0);
			case 1:
				position = this->readPos + offset; break;
			case 2:
				position = this->fileSize + offset; break;
		}

		this->readPos = position;
		bufferAutoAdjust();
	};

	//Prints position in hex and bytes.
	void printPos()
	{
		printf("Position: %lx\t%lu\n",readPos,readPos);
	};

	//Returns error status. 0 is none, -1 is EOF, -2 is ERROR.
	int getErrorStatus()
	{
		return this->errorStatus;
	};

	//Gives info useful for debugging purposes.
	void debug()
	{
		printf("ReadPosition: %lx\t%lu\n",readPos,readPos);
		printf("Buffer Start: %lx\t%lu\n",bufferPos,bufferPos);
		printf("Buffer End:   %lx\t%lu\n",bufferPos+bufferSize,bufferPos+bufferSize);
	};
};