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

NodeDescPtr
IcePatch::DirectoryI::describe(const Ice::Current& current)
{
    DirectoryDescPtr desc = new DirectoryDesc;
    desc->directory = DirectoryPrx::uncheckedCast(_adapter->createProxy(current.identity));
    return desc;
}

NodeDescSeq
IcePatch::DirectoryI::getContents(const Ice::Current& current)
{
    string path = normalizePath(current.identity.name);

    struct dirent **namelist;
    int n = scandir(path.c_str(), &namelist, 0, alphasort);
    if (n < 0)
    {
	NodeAccessException ex;
	ex.reason = "cannot read directory `" + path + "':" + strerror(errno);
	throw ex;
    }

    NodeDescSeq result;
    result.reserve(n - 2);

    int i;

    try
    {
	Identity identity;
	identity.category = "IcePatch";
	
	for (i = 0; i < n; ++i)
	{
	    string name = namelist[i]->d_name;

	    if (name != ".." && name != ".")
	    {
		identity.name = path + '/' + name;
		NodePrx node = NodePrx::uncheckedCast(_adapter->createProxy(identity));
		try
		{
		    result.push_back(node->describe());
		}
		catch (const ObjectNotExistException&)
		{
		    //
		    // Ignore. This can for example happen if the node
		    // locator cannot call stat() on the file.
		    //
		}
	    }
	}

	for (i = 0; i < n; ++i)
	{
	    free(namelist[i]);
	}
	free(namelist);

    }
    catch (...)
    {
	for (i = 0; i < n; ++i)
	{
	    free(namelist[i]);
	}
	free(namelist);

	throw;
    }
	
    return result;
}

IcePatch::FileI::FileI(const ObjectAdapterPtr& adapter) :
    NodeI(adapter)
{
}

NodeDescPtr
IcePatch::FileI::describe(const Ice::Current& current)
{
    FileDescPtr desc = new FileDesc;
    desc->file = FilePrx::uncheckedCast(_adapter->createProxy(current.identity));
    return desc;
}

ByteSeq
IcePatch::FileI::getBytes(Int startPos, Int howMuch, const Ice::Current& current)
{
    string path = normalizePath(current.identity.name);
    return ByteSeq();
}
