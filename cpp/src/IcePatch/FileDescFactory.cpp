// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
