#pragma once
#include <stdint.h>
#include <cstdlib>
#include <stdio.h>
#include <cstring>
#include <string>
#include <stdexcept>
#include <bits/stdc++.h> //is this standard?

//Buffered file reader utility. Supercedes stdio reader.
class BSReader
{
	private:
	FILE* file;
	char* buffer;
	uint64_t bufferSize = 0;
	uint64_t bufferPos = 0;
	uint64_t writeOffset = 0;

	std::queue<uint64_t> stepStack[64]; 

    public:
	uint64_t fileSize = 0;
	uint64_t readPos = 0;

	BSReader(){};

	BSReader(std::string filePath, uint64_t bufferSizeIn)
	{
		file = fopen(filePath.c_str(),"rb");

		if(file == NULL)
			throw std::runtime_error("Failed to open file!\n");

		bufferSize = bufferSizeIn;
		buffer = new char[bufferSize];
		bufferAutoAdjust();
	};

	~BSReader()
	{
		delete[] buffer;
	};

	//Copies readSize number of bytes to dest pointer.
	void read(void* dest, uint64_t readSize)
	{
		//Design: a read can be broken into 3 distinct parts.
		//Part 1 is within the current buffer and may or may not stretch to the end.
		//Part 2 is the number of full buffer reads needed after the part 1.
		//Part 3 is anything needed after 1 and 2 are finished.
		//All reads will have part one, part 2 and 3 are dependent on read size.
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

			for(int i = 0; i < fullReads; i++)
			{
				//Full buffer reads. Ran as many times as needed.
				memcpy(dest+writeOffset,buffer+(readPos-bufferPos),bufferSize);
				readPos += bufferSize;
				writeOffset += bufferSize;
				bufferAutoAdjust();
			}

			uint64_t remainder = fullReads * bufferSize + startSize - readSize;

			if(remainder > 0)
			{
				//Ending read.
				memcpy(dest+writeOffset,buffer+(readPos-bufferPos),remainder);
				readPos += remainder;
				bufferAutoAdjust();
			}
		}
	};

	private:
	//Sets the correct buffer position in the file.
	void setBufferPosition(uint64_t position)
	{
		bufferPos = position;
		fseek(file,bufferPos,SEEK_SET);
		refillBuffer();
	};

	//Fills buffer based on position in file.
	void refillBuffer()
	{
		fread(buffer,bufferSize,1,file);
	};

	//Calculates the correct buffer position.
	void bufferAutoAdjust()
	{
		setBufferPosition((readPos / bufferSize) * bufferSize);
	};

	public:
	//Returns size of file, also sets the member variable.
	uint64_t getSize()
	{
		fseek(file,0,SEEK_END);
		fileSize = ftell(file);
		rewind(file);
		return fileSize;
	};

	//Place current read position in queue. Travel to target.
	void stepIn(uint64_t targetPosition)
	{
		stepStack->push(readPos);
		readPos = targetPosition;
		bufferAutoAdjust();
	};

	//Return to top of queue.
	void stepOut()
	{
		readPos = stepStack->front();
		stepStack->pop();
		bufferAutoAdjust();
	};

	//Travel to a specified offset in the file.
	void seek(uint64_t targetPosition)
	{
		readPos = targetPosition;
		bufferAutoAdjust();
	};
};