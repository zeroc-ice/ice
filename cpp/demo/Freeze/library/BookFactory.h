// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef BOOK_FACTORY_H
#define BOOK_FACTORY_H

#include <LibraryI.h>

class BookFactory : public Ice::ObjectFactory
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
