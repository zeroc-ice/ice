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
#include <fstream>

using namespace std;
using namespace Ice;
using namespace IcePatch;

static IceUtil::RWRecMutex globalMutex;

IcePatch::FileI::FileI(const ObjectAdapterPtr& adapter, const string& dir) :
    _adapter(adapter),
    _fileTraceLogger(adapter->getCommunicator()->getProperties()->getPropertyAsInt("IcePatch.Trace.Files") > 0 ?
		     adapter->getCommunicator()->getLogger() : LoggerPtr()),
    _busyTimeout(IceUtil::Time::seconds(adapter->getCommunicator()->getProperties()->
					getPropertyAsIntWithDefault("IcePatch.BusyTimeout", 10))),
    _dir(dir)
{
    assert(!_dir.empty());
    if(_dir[_dir.length() - 1] != '/')
    {
	const_cast<string&>(_dir) += '/';
    }
}

ByteSeq
IcePatch::FileI::readMD5(const Current& current) const
{
    string path = identityToPath(current.id);

    if(path == ".")
    {
        path = _dir.substr(0, _dir.size() - 1); // Remove trailing '/'.
    }
    else
    {
        path = _dir + path;
    }

    string pathMD5 = path + ".md5";
    
    IceUtil::RWRecMutex::TryRLock sync(globalMutex, _busyTimeout);
    if(!sync.acquired())
    {
	throw BusyException();
    }

    FileInfo info = getFileInfo(path, true, _fileTraceLogger);
    FileInfo infoMD5 = getFileInfo(pathMD5, false, _fileTraceLogger);

    if(infoMD5.type != FileTypeRegular || infoMD5.time <= info.time)
    {
	if(!sync.timedUpgrade(_busyTimeout))
	{
	    throw BusyException();
	}
	
	infoMD5 = getFileInfo(pathMD5, false, _fileTraceLogger);
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
	    infoMD5 = getFileInfo(pathMD5, true, _fileTraceLogger); // Must exist.
	    if(info.size != oldInfo.size)
	    {
		info.time = infoMD5.time;
	    }
	}
    }
	
    return getMD5(path);
}

IcePatch::DirectoryI::DirectoryI(const ObjectAdapterPtr& adapter, const string& dir) :
    FileI(adapter, dir)
{
}

FileDescPtr
IcePatch::DirectoryI::describe(const Current& current) const
{
    // No mutex lock necessary.
    DirectoryDescPtr desc = new DirectoryDesc;
    desc->md5 = readMD5(current);
    
    //
    // We want compression for directories, to compress directory
    // listings on the fly.
    //
    desc->dir = DirectoryPrx::uncheckedCast(_adapter->createProxy(current.id));//->ice_compress(true));
    return desc;
}

FileDescSeq
IcePatch::DirectoryI::getContents(const Current& current) const
{
    StringSeq filteredPaths;

    {
	IceUtil::RWRecMutex::TryRLock sync(globalMutex, _busyTimeout);
	if(!sync.acquired())
	{
	    throw BusyException();
	}

	bool syncUpgraded = false;
	string prependPath = identityToPath(current.id);
	string realPath = _dir + prependPath;
	StringSeq paths = readDirectory(realPath, prependPath);
	filteredPaths.reserve(paths.size() / 3);
	for(StringSeq::const_iterator p = paths.begin(); p != paths.end(); ++p)
	{
	    if(ignoreSuffix(*p))
	    {
		pair<StringSeq::iterator, StringSeq::iterator> r =
		    equal_range(paths.begin(), paths.end(), removeSuffix(*p));
		if(r.first == r.second)
		{
		    if(!syncUpgraded)
		    {
			syncUpgraded = sync.timedUpgrade(_busyTimeout);
			if(!syncUpgraded)
			{
			    throw BusyException();
			}
		    }
		    StringSeq paths2 = readDirectory(realPath, prependPath);
		    pair<StringSeq::iterator, StringSeq::iterator> r2 =
			equal_range(paths2.begin(), paths2.end(), removeSuffix(*p));
		    if(r2.first == r2.second)
		    {
			removeRecursive(_dir + *p, _fileTraceLogger);
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

Long
IcePatch::DirectoryI::getTotal(const Ice::ByteSeq& md5, const Ice::Current& current) const
{
    string path = identityToPath(current.id);

    if(path == ".")
    {
        path = _dir.substr(0, _dir.size() - 1); // Remove trailing '/'.
    }
    else
    {
        path = _dir + path;
    }

    IceUtil::RWRecMutex::TryRLock sync(globalMutex, _busyTimeout);
    if(!sync.acquired())
    {
        throw BusyException();
    }

    TotalMap totals = getTotalMap(_adapter->getCommunicator(), path);
    TotalMap::const_iterator p;
    ByteSeq zeroMD5(16, 0);
    if(md5.empty() || md5 == zeroMD5)
    {
        p = totals.find(ByteSeq());
    }
    else
    {
        p = totals.find(md5);
    }
    if(p != totals.end())
    {
        return p->second;
    }
    return -1;
}

IcePatch::RegularI::RegularI(const ObjectAdapterPtr& adapter, const string& dir) :
    FileI(adapter, dir)
{
}

FileDescPtr
IcePatch::RegularI::describe(const Current& current) const
{
    // No mutex lock necessary.
    RegularDescPtr desc = new RegularDesc;
    desc->md5 = readMD5(current);

    //
    // We do not want compression for regular files, because we
    // download pre-compressed files.
    //
    desc->reg = RegularPrx::uncheckedCast(_adapter->createProxy(current.id));//->ice_compress(false));

    return desc;
}

Int
IcePatch::RegularI::getBZ2Size(const Current& current) const
{
    IceUtil::RWRecMutex::TryRLock sync(globalMutex, _busyTimeout);
    if(!sync.acquired())
    {
	throw BusyException();
    }
    
    string path = _dir + identityToPath(current.id);
    
    FileInfo info = getFileInfo(path, true, _fileTraceLogger);
    assert(info.type == FileTypeRegular);
    FileInfo infoBZ2 = getFileInfo(path + ".bz2", false, _fileTraceLogger);
    
    if(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
    {
	if(!sync.timedUpgrade(_busyTimeout))
	{
	    throw BusyException();
	}
	
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
    
    return static_cast<Int>(infoBZ2.size);
}

ByteSeq
IcePatch::RegularI::getBZ2(Int pos, Int num, const Current& current) const
{
    IceUtil::RWRecMutex::TryRLock sync(globalMutex, _busyTimeout);
    if(!sync.acquired())
    {
	throw BusyException();
    }

    string path = _dir + identityToPath(current.id);
    
    FileInfo info = getFileInfo(path, true, _fileTraceLogger);
    assert(info.type == FileTypeRegular);
    FileInfo infoBZ2 = getFileInfo(path + ".bz2", false, _fileTraceLogger);
    
    if(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
    {
	if(!sync.timedUpgrade(_busyTimeout))
	{
	    throw BusyException();
	}
	
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

ByteSeq
IcePatch::RegularI::getBZ2MD5(Int size, const Current& current) const
{
    IceUtil::RWRecMutex::TryRLock sync(globalMutex, _busyTimeout);
    if(!sync.acquired())
    {
	throw BusyException();
    }

    string path = _dir + identityToPath(current.id);

    FileInfo info = getFileInfo(path, true, _fileTraceLogger);
    assert(info.type == FileTypeRegular);
    FileInfo infoBZ2 = getFileInfo(path + ".bz2", false, _fileTraceLogger);
    
    if(infoBZ2.type != FileTypeRegular || infoBZ2.time <= info.time)
    {
	if(!sync.timedUpgrade(_busyTimeout))
	{
	    throw BusyException();
	}
	
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
    
    return calcPartialMD5(path + ".bz2", size, _fileTraceLogger);
}
