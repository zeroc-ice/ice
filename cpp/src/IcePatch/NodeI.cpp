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
#include <IcePatch/Util.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

IcePatch::NodeI::NodeI(const ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
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
    StringSeq paths = readDirectory(identityToPath(current.identity));

    NodeDescSeq result;
    result.reserve(paths.size());

    StringSeq::const_iterator p;
    for (p = paths.begin(); p != paths.end(); ++p)
    {
	string suffix = getSuffix(*p);
	if (suffix == "md5" || suffix == "bz2")
	{
	    continue;
	}
	
	NodePrx node = NodePrx::uncheckedCast(_adapter->createProxy(pathToIdentity(*p)));
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
    desc->md5 = getMD5(identityToPath(current.identity));
    return desc;
}

ByteSeq
IcePatch::FileI::getBlock(Int n, const Ice::Current& current)
{
    return getBZ2(identityToPath(current.identity), n);
}
