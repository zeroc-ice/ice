// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IcePatch/NodeLocator.h>
#include <IcePatch/Util.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

IcePatch::NodeLocator::NodeLocator(const Ice::ObjectAdapterPtr& adapter) :
    _directory(new DirectoryI(adapter)),
    _file(new FileI(adapter))
{
}

ObjectPtr
IcePatch::NodeLocator::locate(const ObjectAdapterPtr& adapter, const Current& current, LocalObjectPtr&)
{
    //
    // Check whether the path is valid.
    //
    string path = identityToPath(current.identity);

    if (path.empty())
    {
	return 0;
    }

    if (path[0] == '/') // Path must not start with '/'.
    {
	return 0;
    }

    if (path.find("..") != string::npos) // Path must not contain '..'.
    {
	return 0;
    }

    if (path.find(':') == 1) // Path cannot contain ':' as second character.
    {
	return 0;
    }

    FileInfo info;
    try
    {
	info = getFileInfo(path);
    }
    catch (const NodeAccessException& ex)
    {
	Warning out(adapter->getCommunicator()->getLogger());
	out << ex << ":\n" << ex.reason;
	return 0;
    }

    switch (info)
    {
	case FileInfoDirectory:
	{
	    return _directory;
	}

	case FileInfoRegular:
	{
	    return _file;
	}

	default:
	{
	    return 0;
	}
    }
}

void
IcePatch::NodeLocator::finished(const ObjectAdapterPtr&, const Current& current, const ObjectPtr&,
				const LocalObjectPtr&)
{
    // Nothing to do.
}

void
IcePatch::NodeLocator::deactivate()
{
    //
    // Break cyclic dependencies.
    //
    _directory = 0;
    _file = 0;
}
