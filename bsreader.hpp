#pragma once
#include <stdint.h>
#include <cstdlib>
#include <stdio.h>
#include <cstring>
#include <string>
#include <stdexcept>
#include <bits/stdc++.h> //is this standard?

class BSReader
{
	private:
	FILE* file;
	char* buffer;
	uint64_t bufferSize;
	uint64_t bufferPos;

	std::queue<uint64_t> stepStack[64]; 

    public:
	uint64_t fileSize;
	uint64_t readPos;

	BSReader(){};

	BSReader(std::string filePath, uint64_t bufferSizeIn)
	{
		file = fopen(filePath.c_str(),"rb");

		if(file == NULL)
			throw std::runtime_error("Failed to open file!\n");

		bufferSize = bufferSizeIn;
		buffer = new char[bufferSize];
	};

	~BSReader()
	{
		delete[] buffer;
	};

	void read(void* dest, uint64_t readSize)
	{
		//do first read, run second until not needed, if needed do third
		bool readIsTooBig = (bufferPos + bufferSize) - readPos < readSize;

		uint64_t startSize = readIsTooBig ? (bufferPos+bufferSize) - readPos : readSize;
		memcpy(dest,buffer+(readPos-bufferPos),startSize);
		readPos += startSize;
		bufferAutoAdjust();
		
		if(readIsTooBig)
		{
			uint64_t fullReads = (readSize - startSize) / bufferSize;

			for(int i = 0; i < fullReads; i++)
			{
				memcpy(dest+readPos-bufferPos,buffer+(readPos-bufferPos),bufferSize);
				readPos += bufferSize;
				bufferAutoAdjust();
			}

			uint64_t remainder = fullReads * bufferSize + startSize - readSize;

			if(remainder > 0)
			{
				memcpy(dest+readPos-bufferPos,buffer+(readPos-bufferPos),remainder);
				readPos += remainder;
				bufferAutoAdjust();
			}
		}
	};

	private:
	void setBufferPosition(uint64_t position)
	{
		bufferPos = position;
		fseek(file,bufferPos,SEEK_SET);
		refillBuffer();
	};

	void refillBuffer()
	{
		fread(buffer,bufferSize,1,file);
	};

	void bufferAutoAdjust()
	{
		setBufferPosition((readPos / bufferSize) * bufferSize);
	};

	void getSize()
	{
		fseek(file,0,SEEK_END);
		fileSize = ftell(file);
		rewind(file);
	};

	void stepIn(uint64_t targetPosition)
	{
		stepStack->push(readPos);
		readPos = targetPosition;
		bufferAutoAdjust();
	};

	void stepOut()
	{
		readPos = stepStack->front();
		stepStack->pop();
		bufferAutoAdjust();
	};
};