// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IcePatch/FileDescFactory.h>

using namespace std;
using namespace Ice;
using namespace IcePatch;

ObjectPtr
IcePatch::FileDescFactory::create(const std::string& type)
{
    if(type == "::IcePatch::DirectoryDesc")
    {
	return new DirectoryDesc;
    }

    if(type == "::IcePatch::RegularDesc")
    {
	return new RegularDesc;
    }

    assert(false);
    return 0; // To keep the compiler from complaining.
}

void
IcePatch::FileDescFactory::destroy()
{
    // Nothing to do.
}
