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

using namespace std;
using namespace Ice;
using namespace IcePatch;


ObjectPtr
IcePatch::NodeLocator::locate(const ObjectAdapterPtr&, const Current& current, LocalObjectPtr&)
{
    assert(current.identity.category == "IcePack");

    //
    // Check whether the path (= identity.name) is valid.
    //
    string path = current.identity.name;

    if (path.empty())
    {
	return 0;
    }

    if (path[0] == '/') // Path must not start with '/'.
    {
	throw ObjectNotExistException(__FILE__, __LINE__);
    }

    if (path.find("..") != string::npos) // Path must not contain '..'.
    {
	throw ObjectNotExistException(__FILE__, __LINE__);
    }

    if (path.find(':') == 1) // Path cannot contain ':' as second character.
    {
	throw ObjectNotExistException(__FILE__, __LINE__);
    }

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
    // Nothing to do.
}
