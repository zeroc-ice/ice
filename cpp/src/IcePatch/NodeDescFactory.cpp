// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IcePatch/NodeDescFactory.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

ObjectPtr
IcePatch::NodeDescFactory::create(const std::string& type)
{
    if (type == "::IcePatch::DirectoryDesc")
    {
	return new DirectoryDesc;
    }

    if (type == "::IcePatch::FileDesc")
    {
	return new FileDesc;
    }

    assert(false);
    return 0; // To keep the compiler from complaining.
}

void
IcePatch::NodeDescFactory::destroy()
{
    // Nothing to do.
}
