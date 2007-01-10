// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/Properties.h>

#include <IceGrid/FileCache.h>
#include <IceGrid/Exception.h>

#include <deque>
#include <fstream>

using namespace std;
using namespace IceGrid;

FileCache::FileCache(const Ice::CommunicatorPtr& com) : 
    _messageSizeMax(com->getProperties()->getPropertyAsIntWithDefault("Ice.MessageSizeMax", 1024) * 1024 - 256)
{
}

Ice::Long
FileCache::getOffsetFromEnd(const string& file, int originalCount)
{
    ifstream is(file.c_str());
    if(is.fail())
    {
	throw FileNotAvailableException("failed to open file `" + file + "'");
    }

    int blockSize = 16 * 1024; // Start reading a block of 16K from the end of the file.
    is.seekg(0, ios::end);
    Ice::Long endOfFileOffset = is.tellg();
    Ice::Long lastBlockOffset = endOfFileOffset;
    int totalCount = 0;
    int totalSize = 0;
    string line;
    do
    {
	//
	// Move the current position of the stream to the new block to
	// read.
	//
	is.clear();
	if(lastBlockOffset - blockSize > 0)
	{
#ifdef _WIN32
	    is.seekg(static_cast<int>(lastBlockOffset - blockSize));
#else
	    is.seekg(static_cast<streampos>(lastBlockOffset - blockSize));
#endif
	    getline(is, line); // Ignore the first line as it's most likely not complete.
	}
	else
	{
	    is.seekg(0); // We've reach the begining of the file.
	}
	
	//
	// Read the block and count the number of lines in the block
	// as well as the number of bytes read. If we found the "first
	// last N line", we start throwing out the lines read at the
	// begining of the file. The total size read will give us the
	// position from the end of the file.
	//
	deque<string> lines;
	int count = originalCount - totalCount; // Number of lines left to find.
#ifdef __sun
	while(is.good() && is.tellg() < static_cast<streampos>(lastBlockOffset))
#else
	while(is.good() && is.tellg() < lastBlockOffset)
#endif
	{
	    getline(is, line);

	    lines.push_back(line);
	    ++totalCount;
	    totalSize += line.size() + 1;
	    if(lines.size() == static_cast<unsigned int>(count + 1))
	    {
		--totalCount;
		totalSize -= lines.front().size() + 1;
		lines.pop_front();
	    }
	}

	if(lastBlockOffset - blockSize < 0)
	{
	    break; // We're done if the block started at the begining of the file.
	}
	else if(totalCount < originalCount)
	{
	    //
	    // Otherwise, it we still didn't find the required number of lines, 
	    // read another block of text before this block.
	    //
	    lastBlockOffset -= blockSize; // Position of the block we just read.
	    blockSize *= 2; // Read a bigger block.
	}
    }
    while(totalCount  < originalCount && !is.bad());

    if(is.bad())
    {
	throw FileNotAvailableException("unrecoverable error occured while reading file `" + file + "'");
    }

    return endOfFileOffset - totalSize;
}

bool
FileCache::read(const string& file, Ice::Long offset, int size, Ice::Long& newOffset, Ice::StringSeq& lines)
{
    assert(size > 0);

    if(size > _messageSizeMax)
    {
	size = _messageSizeMax;
    }

    if(size <= 5)
    {
	throw FileNotAvailableException("maximum bytes per read request is too low");
    }

    ifstream is(file.c_str());
    if(is.fail())
    {
	throw FileNotAvailableException("failed to open file `" + file + "'");
    }

    //
    // Check if the requested offset is past the end of the file, if
    // that's the case return an empty sequence of lines and indicate
    // the EOF.
    //
    is.seekg(0, ios::end);
    if(offset >= is.tellg())
    {
	newOffset = is.tellg();
	lines = Ice::StringSeq();
	return true;
    }

    //
    // Read lines from the file until we read enough or reached EOF.
    // 
    newOffset = offset;
    lines = Ice::StringSeq();
#ifdef _WIN32
    is.seekg(static_cast<int>(offset));
#else
    is.seekg(static_cast<streampos>(offset));
#endif
    int totalSize = 0;
    string line;
    for(int i = 0; is.good(); ++i)
    {
	getline(is, line);
	int lineSize = line.size() + 5; // 5 bytes for the encoding of the string size (worst case scenario)
	if(lineSize + totalSize > size)
	{
	    if(size - totalSize - 5) // If there's some room left for a part of the string, return a partial string
	    {
		line.substr(0, size - totalSize - 5);
		lines.push_back(line);
		newOffset += line.size();
	    }
	    else
	    {
		lines.push_back("");
	    }
	    break;
	}

	totalSize += lineSize;
	if(!is.fail())
	{
	    newOffset = is.tellg();
	}
	lines.push_back(line);
    }

    if(is.bad())
    {
	throw FileNotAvailableException("unrecoverable error occured while reading file `" + file + "'");
    }

    return is.eof();
}

