// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <BookFactory.h>

using namespace std;
using namespace Ice;

BookFactory::BookFactory(const LibraryIPtr& library, const Freeze::EvictorPtr& evictor) :
    _library(library),
    _evictor(evictor)
{
}

ObjectPtr
BookFactory::create(const string& type)
{
    assert(type == "::Book");
    return new BookI(_library, _evictor);
}

void
BookFactory::destroy()
{
    //
    // Break cyclic object dependencies
    //
    _library = 0;
    _evictor = 0;
}
