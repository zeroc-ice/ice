// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef LIBRARY_I_H
#define LIBRARY_I_H

#include <IceUtil/RWRecMutex.h>

#include <Library.h>
#include <LibraryTypes.h>

class LibraryI : public Library, public IceUtil::RWRecMutex
{
public:

    LibraryI(const Ice::ObjectAdapterPtr&, const Freeze::DBPtr&, const Freeze::EvictorPtr&);
    virtual ~LibraryI();

    virtual ::BookPrx createBook(const ::BookDescription&, const Ice::Current&);
    virtual ::BookPrx findByIsbn(const ::std::string&, const Ice::Current&);
    virtual ::BookPrxSeq findByAuthors(const ::std::string&, const Ice::Current&);
    virtual void setEvictorSize(::Ice::Int, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    void remove(const BookDescription&);

private:

    Ice::ObjectAdapterPtr _adapter;
    Freeze::EvictorPtr _evictor;

    //
    // This is a dictionary of authors to a sequence of isbn numbers
    // for efficient lookup of books by authors.
    //
    StringIsbnSeqDict _authors;
};

typedef IceUtil::Handle<LibraryI> LibraryIPtr;

class BookI : public Book, public IceUtil::RWRecMutex
{
public:

    BookI(const LibraryIPtr&, const Freeze::EvictorPtr&);
    virtual ~BookI();

    virtual ::BookDescription getBookDescription(const Ice::Current&);
    virtual void destroy(const Ice::Current&);
    virtual void rentBook(const ::std::string&, const Ice::Current&);
    virtual ::std::string getRenterName(const Ice::Current&);
    virtual void returnBook(const Ice::Current&);

    static Ice::Identity createIdentity(const ::std::string&);

private:

    LibraryIPtr _library;
    Freeze::EvictorPtr _evictor;
};

#endif
