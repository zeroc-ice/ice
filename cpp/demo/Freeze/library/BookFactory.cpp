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

#include <BookFactory.h>

using namespace std;
using namespace Ice;

BookFactory::BookFactory(const LibraryIPtr& library) :
    _library(library)
{
}

ObjectPtr
BookFactory::create(const string& type)
{
    assert(type == "::Book");
    return new BookI(_library);
}

void
BookFactory::destroy()
{
    //
    // Break cyclic object dependencies
    //
    _library = 0;
}
