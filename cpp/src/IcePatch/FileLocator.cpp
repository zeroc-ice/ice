// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#include <IcePatch/FileLocator.h>
#include <IcePatch/Util.h>
#include <IcePatch/IcePatchI.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

IcePatch::FileLocator::FileLocator(const Ice::ObjectAdapterPtr& adapter, const string& dir) :
    _directory(new DirectoryI(adapter, dir)),
    _regular(new RegularI(adapter, dir)),
    _logger(adapter->getCommunicator()->getLogger()),
    _fileTraceLogger(adapter->getCommunicator()->getProperties()->getPropertyAsInt("IcePatch.Trace.Files") > 0 ?
		     _logger : LoggerPtr()),
    _dir(dir)
{
    assert(!_dir.empty());
    if(_dir[_dir.length() - 1] != '/')
    {
	const_cast<string&>(_dir) += '/';
    }
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
	info = getFileInfo(_dir + path, true, _fileTraceLogger);
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
IcePatch::FileLocator::deactivate(const std::string&)
{
    //
    // Break cyclic dependencies.
    //
    _directory = 0;
    _regular = 0;
}
