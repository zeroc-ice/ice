// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IcePatch/NodeI.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

string
IcePatch::NodeI::normalizePath(const string& path)
{
    string result = path;
    
    string::size_type pos;
    
    for (pos = 0; pos < result.size(); ++pos)
    {
	if (result[pos] == '\\')
	{
	    result[pos] = '/';
	}
    }
    
    pos = 0;
    while ((pos = result.find("//", pos)) != string::npos)
    {
	result.erase(pos, 1);
    }
    
    pos = 0;
    while ((pos = result.find("/./", pos)) != string::npos)
    {
	result.erase(pos, 2);
    }

    return result;
}

Nodes
IcePatch::DirectoryI::getNodes(const Ice::Current& current)
{
    //
    // No synchronization necessary, this servant is completely
    // stateless.
    //

    string path = normalizePath(current.identity.name);

    return Nodes();
}

ByteSeq
IcePatch::FileI::getBytes(Int startPos, Int howMuch, const Ice::Current& current)
{
    //
    // No synchronization necessary, this servant is completely
    // stateless.
    //

    string path = normalizePath(current.identity.name);

    return ByteSeq();
}
