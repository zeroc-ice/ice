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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

IcePatch::NodeLocator::NodeLocator(const Ice::ObjectAdapterPtr& adapter) :
    _directory(new DirectoryI(adapter)),
    _file(new FileI(adapter))
{
}

ObjectPtr
IcePatch::NodeLocator::locate(const ObjectAdapterPtr&, const Current& current, LocalObjectPtr&)
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

    struct stat buf;
    if (stat(path.c_str(), &buf) == -1)
    {
	//
	// Can't really throw any sensible exception here if stat()
	// fails, so I return 0.
	//
	return 0;
    }

    if (S_ISDIR(buf.st_mode))
    {
	return _directory;
    }

    if (S_ISREG(buf.st_mode))
    {
	return _file;
    }

    //
    // Neither a regular file nor a directory, so we return 0.
    //
    return 0;
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
