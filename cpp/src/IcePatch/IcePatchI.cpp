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

IceUtil::RWRecMutex IcePatch::FileI::_globalMutex;

IcePatch::FileI::FileI(const ObjectAdapterPtr& adapter) :
    _adapter(adapter),
    _logger(adapter->getCommunicator()->getLogger())
{
    _traceLevel = atoi(adapter->getCommunicator()->getProperties()->getProperty("IcePatch.Trace.Files").c_str());
}

IcePatch::DirectoryI::DirectoryI(const ObjectAdapterPtr& adapter) :
    FileI(adapter)
{
}

FileDescPtr
IcePatch::DirectoryI::describe(const Ice::Current& current)
{
    // No lock necessary.
    string path = identityToPath(current.identity);
    DirectoryDescPtr desc = new DirectoryDesc;
    desc->directory = DirectoryPrx::uncheckedCast(_adapter->createProxy(current.identity));
    return desc;
}

FileDescSeq
IcePatch::DirectoryI::getContents(const Ice::Current& current)
{
    StringSeq filteredPaths;

    {
	IceUtil::RWRecMutex::WLock sync(_globalMutex); // TODO: RLock as soon as lock promotion works.
	string path = identityToPath(current.identity);
	StringSeq paths = readDirectory(path);
	filteredPaths.reserve(paths.size() / 3);
	for (StringSeq::const_iterator p = paths.begin(); p != paths.end(); ++p)
	{
	    if (ignoreSuffix(*p))
	    {
		pair<StringSeq::const_iterator, StringSeq::const_iterator> r =
		    equal_range(paths.begin(), paths.end(), removeSuffix(*p));
		if (r.first == r.second)
		{
		    //IceUtil::RWRecMutex::WLock sync(_globalMutex);
		    StringSeq paths2 = readDirectory(path);
		    pair<StringSeq::const_iterator, StringSeq::const_iterator> r2 =
			equal_range(paths2.begin(), paths2.end(), removeSuffix(*p));
		    if (r2.first == r2.second)
		    {
			removeRecursive(*p);

			if (_traceLevel > 0)
			{
			    Trace out(_logger, "IcePatch");
			    out << "removed orphaned file `" << *p << "'";
			}
		    }
		}
	    }
	    else
	    {
		filteredPaths.push_back(*p);
	    }
	}
    }

    //
    // Call describe() outside the thread synchronization, to avoid
    // deadlocks.
    //
    FileDescSeq result;
    result.reserve(filteredPaths.size());
    for (StringSeq::const_iterator p = filteredPaths.begin(); p != filteredPaths.end(); ++p)
    {
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
    IceUtil::RWRecMutex::WLock sync(_globalMutex); // TODO: RLock as soon as lock promotion works.
    string path = identityToPath(current.identity);

    FileInfo info = getFileInfo(path, true);
    FileInfo infoMD5 = getFileInfo(path + ".md5", false);
    if (infoMD5.type != FileTypeRegular || infoMD5.time < info.time)
    {
	//IceUtil::RWRecMutex::WLock sync(_globalMutex);
	infoMD5 = getFileInfo(path + ".md5", false);
	if (infoMD5.type != FileTypeRegular || infoMD5.time < info.time)
	{
	    createMD5(path);

	    if (_traceLevel > 0)
	    {
		Trace out(_logger, "IcePatch");
		out << "created .md5 file for `" << path << "'";
	    }
	}
    }

    RegularDescPtr desc = new RegularDesc;
    desc->regular = RegularPrx::uncheckedCast(_adapter->createProxy(current.identity));
    desc->md5 = getMD5(path);
    return desc;
}

Int
IcePatch::RegularI::getBZ2Size(const Ice::Current& current)
{
    IceUtil::RWRecMutex::WLock sync(_globalMutex); // TODO: RLock as soon as lock promotion works.
    string path = identityToPath(current.identity);

    FileInfo info = getFileInfo(path, true);
    FileInfo infoBZ2 = getFileInfo(path + ".bz2", false);
    if (infoBZ2.type != FileTypeRegular || infoBZ2.time < info.time)
    {
	//IceUtil::RWRecMutex::WLock sync(_globalMutex);
	infoBZ2 = getFileInfo(path + ".bz2", false);
	if (infoBZ2.type != FileTypeRegular || infoBZ2.time < info.time)
	{
	    createBZ2(path);

	    if (_traceLevel > 0)
	    {
		Trace out(_logger, "IcePatch");
		out << "created .bz2 file for `" << path << "'";
	    }

	    // Get the .bz2 file info again, so that we can return the
	    // size below. This time the .bz2 file must exist,
	    // otherwise an exception is raised.
	    infoBZ2 = getFileInfo(path + ".bz2", true);
	}
    }

    return infoBZ2.size;
}

ByteSeq
IcePatch::RegularI::getBZ2(Ice::Int pos, Ice::Int num, const Ice::Current& current)
{
    IceUtil::RWRecMutex::WLock sync(_globalMutex); // TODO: RLock as soon as lock promotion works.
    string path = identityToPath(current.identity);

    FileInfo info = getFileInfo(path, true);
    FileInfo infoBZ2 = getFileInfo(path + ".bz2", false);
    if (infoBZ2.type != FileTypeRegular || infoBZ2.time < info.time)
    {
	//IceUtil::RWRecMutex::WLock sync(_globalMutex);
	infoBZ2 = getFileInfo(path + ".bz2", false);
	if (infoBZ2.type != FileTypeRegular || infoBZ2.time < info.time)
	{
	    createBZ2(path);

	    if (_traceLevel > 0)
	    {
		Trace out(_logger, "IcePatch");
		out << "created .bz2 file for `" << path << "'";
	    }
	}
    }

    return IcePatch::getBZ2(path, pos, num);
}
