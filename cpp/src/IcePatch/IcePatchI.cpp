// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
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
    PropertiesPtr properties = adapter->getCommunicator()->getProperties();
    _traceLevel = properties->getPropertyAsInt("IcePatch.Trace.Files");
    _busyTimeout = IceUtil::Time::seconds(properties->getPropertyAsIntWithDefault("IcePatch.BusyTimeout", 10));
}

IcePatch::DirectoryI::DirectoryI(const ObjectAdapterPtr& adapter) :
    FileI(adapter)
{
}

FileDescPtr
IcePatch::DirectoryI::describe(const Current& current)
{
    // No mutex lock necessary.
    DirectoryDescPtr desc = new DirectoryDesc;
    desc->directory = DirectoryPrx::uncheckedCast(_adapter->createProxy(current.id));
    return desc;
}

FileDescSeq
IcePatch::DirectoryI::getContents(const Current& current)
{
    StringSeq filteredPaths;

    try
    {
	IceUtil::RWRecMutex::TryRLock sync(_globalMutex, _busyTimeout);

	bool syncUpgraded = false;
	string path = identityToPath(current.id);
	StringSeq paths = readDirectory(path);
	filteredPaths.reserve(paths.size() / 3);
	for(StringSeq::const_iterator p = paths.begin(); p != paths.end(); ++p)
	{
	    if(ignoreSuffix(*p))
	    {
		pair<StringSeq::const_iterator, StringSeq::const_iterator> r =
		    equal_range(paths.begin(), paths.end(), removeSuffix(*p));
		if(r.first == r.second)
		{
		    if(!syncUpgraded)
		    {
			sync.timedUpgrade(_busyTimeout);
			syncUpgraded = true;
		    }
		    StringSeq paths2 = readDirectory(path);
		    pair<StringSeq::const_iterator, StringSeq::const_iterator> r2 =
			equal_range(paths2.begin(), paths2.end(), removeSuffix(*p));
		    if(r2.first == r2.second)
		    {
			removeRecursive(*p);

			if(_traceLevel > 0)
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
    catch(const IceUtil::LockedException&)
    {
	throw BusyException();
    }

    //
    // Call describe() outside the thread synchronization, to avoid
    // deadlocks.
    //
    FileDescSeq result;
    result.reserve(filteredPaths.size());
    for(StringSeq::const_iterator p = filteredPaths.begin(); p != filteredPaths.end(); ++p)
    {
	FilePrx file = FilePrx::uncheckedCast(_adapter->createProxy(pathToIdentity(*p)));
	try
	{
	    result.push_back(file->describe());
	}
	catch(const ObjectNotExistException&)
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
IcePatch::RegularI::describe(const Current& current)
{
    try
    {
	IceUtil::RWRecMutex::TryRLock sync(_globalMutex, _busyTimeout);
	
	string path = identityToPath(current.id);
	FileInfo info = getFileInfo(path, true);
	assert(info.type == FileTypeRegular);

	FileInfo infoMD5 = getFileInfo(path + ".md5", false);
	if(infoMD5.type != FileTypeRegular || infoMD5.time <= info.time)
	{
	    sync.timedUpgrade(_busyTimeout);

	    infoMD5 = getFileInfo(path + ".md5", false);
	    if(infoMD5.type != FileTypeRegular || infoMD5.time <= info.time)
	    {
		createMD5(path);
		
		if(_traceLevel > 0)
		{
		    Trace out(_logger, "IcePatch");
		    out << "created MD5 file for file `" << path << "'";
		}
	    }
	}
	
	RegularDescPtr desc = new RegularDesc;
	desc->md5 = getMD5(path);
	desc->regular = RegularPrx::uncheckedCast(_adapter->createProxy(current.id));
	return desc;
    }
    catch(const IceUtil::LockedException&)
    {
	throw BusyException();
    }
}

Int
IcePatch::RegularI::getBZ2Size(const Current& current)
{
    try
    {
	IceUtil::RWRecMutex::TryRLock sync(_globalMutex, _busyTimeout);
	
	string path = identityToPath(current.id);
	FileInfo info = getFileInfo(path, true);
	assert(info.type == FileTypeRegular);

	FileInfo infoBZ2 = getFileInfo(path + ".bz2", false);
	if(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
	{
	    sync.timedUpgrade(_busyTimeout);

	    infoBZ2 = getFileInfo(path + ".bz2", false);
	    if(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
	    {
		createBZ2(path);
		
		if(_traceLevel > 0)
		{
		    Trace out(_logger, "IcePatch");
		    out << "created BZ2 file for `" << path << "'";
		}
		
		//
		// Get the .bz2 file info again, so that we can return the
		// size below. This time the .bz2 file must exist,
		// otherwise an exception is raised.
		//
		infoBZ2 = getFileInfo(path + ".bz2", true);
	    }
	}
	
	return infoBZ2.size;
    }
    catch(const IceUtil::LockedException&)
    {
	throw BusyException();
    }
}

ByteSeq
IcePatch::RegularI::getBZ2(Int pos, Int num, const Current& current)
{
    try
    {
	IceUtil::RWRecMutex::TryRLock sync(_globalMutex, _busyTimeout);
	
	string path = identityToPath(current.id);
	FileInfo info = getFileInfo(path, true);
	assert(info.type == FileTypeRegular);

	FileInfo infoBZ2 = getFileInfo(path + ".bz2", false);
	if(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
	{
	    sync.timedUpgrade(_busyTimeout);

	    infoBZ2 = getFileInfo(path + ".bz2", false);
	    if(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
	    {
		createBZ2(path);
		
		if(_traceLevel > 0)
		{
		    Trace out(_logger, "IcePatch");
		    out << "created .bz2 file for `" << path << "'";
		}
	    }
	}
	
	return IcePatch::getBZ2(path, pos, num);
    }
    catch(const IceUtil::LockedException&)
    {
	throw BusyException();
    }
}

ByteSeq
IcePatch::RegularI::getBZ2MD5(Int size, const Current& current)
{
    try
    {
	IceUtil::RWRecMutex::TryRLock sync(_globalMutex, _busyTimeout);
	
	string path = identityToPath(current.id);
	FileInfo info = getFileInfo(path, true);
	assert(info.type == FileTypeRegular);

	FileInfo infoBZ2 = getFileInfo(path + ".bz2", false);
	if(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
	{
	    sync.timedUpgrade(_busyTimeout);

	    infoBZ2 = getFileInfo(path + ".bz2", false);
	    if(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
	    {
		createBZ2(path);
		
		if(_traceLevel > 0)
		{
		    Trace out(_logger, "IcePatch");
		    out << "created .bz2 file for `" << path << "'";
		}
	    }
	}
	
	return IcePatch::calcPartialMD5(path + ".bz2", size);
    }
    catch(const IceUtil::LockedException&)
    {
	throw BusyException();
    }
}
