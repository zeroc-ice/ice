// **********************************************************************
//
// Copyright (c) 2002
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
