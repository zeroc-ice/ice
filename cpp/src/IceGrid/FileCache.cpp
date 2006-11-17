// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/FileCache.h>
#include <IceGrid/Exception.h>

#include <fstream>

using namespace std;
using namespace IceGrid;

FileCache::FileCache()
{
}

Ice::StringSeq
FileCache::read(const string& filename, Ice::Long offset, int count, Ice::Long& newOffset)
{
    ifstream is(filename.c_str());
    if(is.fail())
    {
	throw FileNotAvailableException("failed to open file `" + filename + "'");
    }

    newOffset = offset;
    is.seekg(0, ios::end);
    if(offset >= is.tellg())
    {
	newOffset = is.tellg();
	return Ice::StringSeq();
    }

    is.seekg(offset);
    Ice::StringSeq lines;
    for(int i = 0; i < count && is.good(); ++i)
    {
	assert(!is.eof());
	string line;
	getline(is, line);
	if(!is.fail())
	{
	    newOffset = is.tellg();
	    assert(newOffset >= 0);
	}
	lines.push_back(line);
    }
    return lines;
}
