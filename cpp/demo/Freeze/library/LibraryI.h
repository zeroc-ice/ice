// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef LIBRARY_I_H
#define LIBRARY_I_H

#include <IceUtil/Mutex.h>
#include <IceUtil/AbstractMutex.h>
#include <Freeze/Evictor.h>
#include <Library.h>
#include <LibraryTypes.h>

class LibraryI : public Demo::Library, public IceUtil::Mutex
{
public:

    LibraryI(const Ice::CommunicatorPtr& communicator,
             const std::string& envName, const std::string& dbName,
             const Freeze::EvictorPtr& evictor);

    virtual Demo::BookPrx createBook(const Demo::BookDescription&, const Ice::Current&);
    virtual Demo::BookPrx findByIsbn(const std::string&, const Ice::Current&) const;
    virtual Demo::BookPrxSeq findByAuthors(const std::string&, const Ice::Current&) const;
    virtual void setEvictorSize(::Ice::Int, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

    void remove(const Demo::BookDescription&);

private:

    const Freeze::EvictorPtr _evictor;
    const Freeze::ConnectionPtr _connection;

    //
    // This is a dictionary of authors to a sequence of isbn numbers
    // for efficient lookup of books by authors.
    //
    StringIsbnSeqDict _authors;
};

typedef IceUtil::Handle<LibraryI> LibraryIPtr;

class BookI : public Demo::Book, public IceUtil::AbstractMutexI<IceUtil::Mutex>
{
public:

    BookI(const LibraryIPtr&);

    virtual Demo::BookDescription getBookDescription(const Ice::Current&) const;
    virtual void destroy(const Ice::Current&);
    virtual void rentBook(const std::string&, const Ice::Current&);
    virtual std::string getRenterName(const Ice::Current&) const;
    virtual void returnBook(const Ice::Current&);

private:

    const LibraryIPtr _library;
    bool _destroyed;
};

#endif
