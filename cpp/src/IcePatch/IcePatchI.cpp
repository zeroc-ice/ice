// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IcePatch/IcePatchI.h>
#include <IcePatch/Util.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

static IceUtil::RWRecMutex globalMutex;

IcePatch::FileI::FileI(const ObjectAdapterPtr& adapter) :
    _adapter(adapter),
    _fileTraceLogger(adapter->getCommunicator()->getProperties()->getPropertyAsInt("IcePatch.Trace.Files") > 0 ?
		     adapter->getCommunicator()->getLogger() : LoggerPtr()),
    _busyTimeout(IceUtil::Time::seconds(adapter->getCommunicator()->getProperties()->
					getPropertyAsIntWithDefault("IcePatch.BusyTimeout", 10)))
{
}

ByteSeq
IcePatch::FileI::readMD5(const Current& current) const
{
    string path = identityToPath(current.id);

    if(path == ".")
    {
	//
	// We cannot create an MD5 file for the current directory.
	//
	return ByteSeq();
    }
    
    try
    {
	IceUtil::RWRecMutex::TryRLock sync(globalMutex, _busyTimeout);
	
	FileInfo info = getFileInfo(path, true, _fileTraceLogger);
	FileInfo infoMD5 = getFileInfo(path + ".md5", false, _fileTraceLogger);

	if(infoMD5.type != FileTypeRegular || infoMD5.time <= info.time)
	{
	    sync.timedUpgrade(_busyTimeout);

	    infoMD5 = getFileInfo(path + ".md5", false, _fileTraceLogger);
	    while(infoMD5.type != FileTypeRegular || infoMD5.time <= info.time)
	    {
		//
		// Timestamps have only a resolution of one second, so
		// we sleep if it's likely that the created MD5 file
		// would have the same time stamp as the source file.
		//
		IceUtil::Time diff = IceUtil::Time::seconds(info.time + 1) - IceUtil::Time::now();
		if(diff > IceUtil::Time())
		{
		    IceUtil::ThreadControl::sleep(diff);
		}

		createMD5(path, _fileTraceLogger);

		//
		// If the source file size has changed after we
		// created the MD5 file, we try again.
		//
		FileInfo oldInfo = info;
		info = getFileInfo(path, true, _fileTraceLogger);
		infoMD5 = getFileInfo(path + ".md5", true, _fileTraceLogger); // Must exist.
		if(info.size != oldInfo.size)
		{
		    info.time = infoMD5.time;
		}
	    }
	}
	
	return getMD5(path);
    }
    catch(const IceUtil::ThreadLockedException&)
    {
	throw BusyException();
    }
}

IcePatch::DirectoryI::DirectoryI(const ObjectAdapterPtr& adapter) :
    FileI(adapter)
{
}

FileDescPtr
IcePatch::DirectoryI::describe(const Current& current) const
{
    // No mutex lock necessary.
    DirectoryDescPtr desc = new DirectoryDesc;
    desc->md5 = readMD5(current);
    desc->dir = DirectoryPrx::uncheckedCast(_adapter->createProxy(current.id));
    return desc;
}

FileDescSeq
IcePatch::DirectoryI::getContents(const Current& current) const
{
    StringSeq filteredPaths;

    try
    {
	IceUtil::RWRecMutex::TryRLock sync(globalMutex, _busyTimeout);

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
			removeRecursive(*p, _fileTraceLogger);
		    }
		}
	    }
	    else
	    {
		filteredPaths.push_back(*p);
	    }
	}
    }
    catch(const IceUtil::ThreadLockedException&)
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
IcePatch::RegularI::describe(const Current& current) const
{
    // No mutex lock necessary.
    RegularDescPtr desc = new RegularDesc;
    desc->md5 = readMD5(current);
    desc->reg = RegularPrx::uncheckedCast(_adapter->createProxy(current.id));
    return desc;
}

Int
IcePatch::RegularI::getBZ2Size(const Current& current) const
{
    try
    {
	IceUtil::RWRecMutex::TryRLock sync(globalMutex, _busyTimeout);
	
	string path = identityToPath(current.id);

	FileInfo info = getFileInfo(path, true, _fileTraceLogger);
	assert(info.type == FileTypeRegular);
	FileInfo infoBZ2 = getFileInfo(path + ".bz2", false, _fileTraceLogger);

	if(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
	{
	    sync.timedUpgrade(_busyTimeout);

	    infoBZ2 = getFileInfo(path + ".bz2", false, _fileTraceLogger);
	    while(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
	    {
		createBZ2(path, _fileTraceLogger);
		
		//
		// If the source file size has changed after we
		// created the BZ2 file, we try again.
		//
		FileInfo oldInfo = info;
		info = getFileInfo(path, true, _fileTraceLogger);
		infoBZ2 = getFileInfo(path + ".bz2", true, _fileTraceLogger); // Must exist.
		if(info.size != oldInfo.size)
		{
		    info.time = infoBZ2.time;
		}
	    }
	}
	
	return infoBZ2.size;
    }
    catch(const IceUtil::ThreadLockedException&)
    {
	throw BusyException();
    }
}

ByteSeq
IcePatch::RegularI::getBZ2(Int pos, Int num, const Current& current) const
{
    try
    {
	IceUtil::RWRecMutex::TryRLock sync(globalMutex, _busyTimeout);
	
	string path = identityToPath(current.id);

	FileInfo info = getFileInfo(path, true, _fileTraceLogger);
	assert(info.type == FileTypeRegular);
	FileInfo infoBZ2 = getFileInfo(path + ".bz2", false, _fileTraceLogger);

	if(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
	{
	    sync.timedUpgrade(_busyTimeout);

	    infoBZ2 = getFileInfo(path + ".bz2", false, _fileTraceLogger);
	    while(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
	    {
		createBZ2(path, _fileTraceLogger);

		//
		// If the source file size has changed after we
		// created the BZ2 file, we try again.
		//
		FileInfo oldInfo = info;
		info = getFileInfo(path, true, _fileTraceLogger);
		infoBZ2 = getFileInfo(path + ".bz2", true, _fileTraceLogger); // Must exist.
		if(info.size != oldInfo.size)
		{
		    info.time = infoBZ2.time;
		}
	    }
	}
	
	return IcePatch::getBZ2(path, pos, num);
    }
    catch(const IceUtil::ThreadLockedException&)
    {
	throw BusyException();
    }
}

ByteSeq
IcePatch::RegularI::getBZ2MD5(Int size, const Current& current) const
{
    try
    {
	IceUtil::RWRecMutex::TryRLock sync(globalMutex, _busyTimeout);
	
	string path = identityToPath(current.id);

	FileInfo info = getFileInfo(path, true, _fileTraceLogger);
	assert(info.type == FileTypeRegular);
	FileInfo infoBZ2 = getFileInfo(path + ".bz2", false, _fileTraceLogger);

	if(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
	{
	    sync.timedUpgrade(_busyTimeout);

	    infoBZ2 = getFileInfo(path + ".bz2", false, _fileTraceLogger);
	    while(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
	    {
		createBZ2(path, _fileTraceLogger);

		//
		// If the source file size has changed after we
		// created the BZ2 file, we try again.
		//
		FileInfo oldInfo = info;
		info = getFileInfo(path, true, _fileTraceLogger);
		infoBZ2 = getFileInfo(path + ".bz2", true, _fileTraceLogger); // Must exist.
		if(info.size != oldInfo.size)
		{
		    info.time = infoBZ2.time;
		}
	    }
	}
	
	return IcePatch::calcPartialMD5(path + ".bz2", size, _fileTraceLogger);
    }
    catch(const IceUtil::ThreadLockedException&)
    {
	throw BusyException();
    }
}
