// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
