// **********************************************************************
//
// Copyright (c) 2001
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

#ifndef BOOK_FACTORY_H
#define BOOK_FACTORY_H

#include <LibraryI.h>

class BookFactory : virtual public Ice::ObjectFactory
{
public:

    BookFactory(const LibraryIPtr&);

    //
    // Operations from ObjectFactory
    //
    virtual Ice::ObjectPtr create(const std::string&);
    virtual void destroy();

private:

    LibraryIPtr _library;
};

#endif
