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

#include <IcePatch/FileLocator.h>
#include <IcePatch/Util.h>
#include <IcePatch/IcePatchI.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

IcePatch::FileLocator::FileLocator(const Ice::ObjectAdapterPtr& adapter) :
    _directory(new DirectoryI(adapter)),
    _regular(new RegularI(adapter)),
    _logger(adapter->getCommunicator()->getLogger()),
    _fileTraceLogger(adapter->getCommunicator()->getProperties()->getPropertyAsInt("IcePatch.Trace.Files") > 0 ?
		     _logger : LoggerPtr())
{
}

ObjectPtr
IcePatch::FileLocator::locate(const Current& current, LocalObjectPtr&)
{
    //
    // Check whether the path is valid.
    //
    string path = identityToPath(current.id);

    if(path.empty())
    {
	return 0;
    }
    
    if(path[0] == '/') // Example: /usr/mail/foo
    {
	return 0;
    }

    //
    // Note: We could make the following rule more selective, to allow
    // names such as "foo..bar". But since such names are rather
    // uncommon, we disallow ".." altogether, to be on the safe side.
    //
    if(path.find("..") != string::npos) // Example: foo/../..
    {
	return 0;
    }

    if(path.size() >= 2 &&
       ::tolower(path[0]) >= 'a' && ::tolower(path[0]) <= 'z' && path[1] == ':') // Example: c:\blah
    {
	return 0;
    }

    if(ignoreSuffix(path)) // Example: foo.md5
    {
	return 0;
    }

    FileInfo info;
    try
    {
	info = getFileInfo(path, true, _fileTraceLogger);
    }
    catch(const FileAccessException& ex)
    {
	Warning out(_logger);
	out << ex << ":\n" << ex.reason;
	return 0;
    }

    switch(info.type)
    {
	case FileTypeDirectory:
	{
	    return _directory;
	}

	case FileTypeRegular:
	{
	    return _regular;
	}

	default:
	{
	    return 0;
	}
    }
}

void
IcePatch::FileLocator::finished(const Current& current, const ObjectPtr&,
				const LocalObjectPtr&)
{
    // Nothing to do.
}

void
IcePatch::FileLocator::deactivate()
{
    //
    // Break cyclic dependencies.
    //
    _directory = 0;
    _regular = 0;
}
