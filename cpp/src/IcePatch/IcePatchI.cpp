// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IcePatch/IcePatchI.h>
#include <IcePatch/Util.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

IcePatch::FileI::FileI(const ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

IcePatch::DirectoryI::DirectoryI(const ObjectAdapterPtr& adapter) :
    FileI(adapter)
{
}

FileDescPtr
IcePatch::DirectoryI::describe(const Ice::Current& current)
{
    DirectoryDescPtr desc = new DirectoryDesc;
    desc->directory = DirectoryPrx::uncheckedCast(_adapter->createProxy(current.identity));
    return desc;
}

FileDescSeq
IcePatch::DirectoryI::getContents(const Ice::Current& current)
{
    StringSeq paths = readDirectory(identityToPath(current.identity));

    FileDescSeq result;
    result.reserve(paths.size());

    StringSeq::const_iterator p;
    for (p = paths.begin(); p != paths.end(); ++p)
    {
	if (ignoreSuffix(*p))
	{
	    continue;
	}
	
	FilePrx file = FilePrx::uncheckedCast(_adapter->createProxy(pathToIdentity(*p)));
	try
	{
	    result.push_back(file->describe());
	}
	catch (const ObjectNotExistException&)
	{
	    //
	    // Ignore. This can for example happen if the file
	    // locator cannot call stat() on the file.
	    //
	}
    }
	
    return result;
}

IcePatch::RegularI::RegularI(const ObjectAdapterPtr& adapter) :
    FileI(adapter)
{
}

FileDescPtr
IcePatch::RegularI::describe(const Ice::Current& current)
{
    RegularDescPtr desc = new RegularDesc;
    desc->regular = RegularPrx::uncheckedCast(_adapter->createProxy(current.identity));
    desc->md5 = getMD5(identityToPath(current.identity));
    return desc;
}

Int
IcePatch::RegularI::getBZ2Size(const Ice::Current& current)
{
    return getFileInfo(identityToPath(current.identity) + ".bz2", true).size;
}

ByteSeq
IcePatch::RegularI::getBZ2(Ice::Int pos, Ice::Int num, const Ice::Current& current)
{
    return IcePatch::getBZ2(identityToPath(current.identity), pos, num);
}
