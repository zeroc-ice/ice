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
#include <dirent.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

IcePatch::NodeI::NodeI(const ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

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

IcePatch::DirectoryI::DirectoryI(const ObjectAdapterPtr& adapter) :
    NodeI(adapter)
{
}

Nodes
IcePatch::DirectoryI::getNodes(const Ice::Current& current)
{
    //
    // No synchronization necessary, this servant is completely
    // stateless.
    //

    string path = normalizePath(current.identity.name);

    struct dirent **namelist;
    int n = scandir(path.c_str(), &namelist, 0, alphasort);
    if (n < 0)
    {
	NodeAccessException ex;
	ex.reason = "cannot read directory `" + path + "':" + strerror(errno);
	throw ex;
    }

    Nodes result;
    result.reserve(n - 2);

    Identity identity;
    identity.category = "IcePatch";

    while(n--)
    {
	if (strcmp(namelist[n]->d_name, "..") != 0 && strcmp(namelist[n]->d_name, ".") != 0)
	{
	    identity.name = path + '/' + namelist[n]->d_name;
	    result.push_back(NodePrx::uncheckedCast(_adapter->createProxy(identity)));
	}
	free(namelist[n]);
    }
    free(namelist);

    return result;
}

IcePatch::FileI::FileI(const ObjectAdapterPtr& adapter) :
    NodeI(adapter)
{
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
