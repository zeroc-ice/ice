// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/Freeze.h>
#include <LibraryI.h>

using namespace std;
using namespace Demo;

BookI::BookI(const LibraryIPtr& library) :
    _library(library), _destroyed(false)
{
}

void
BookI::destroy(const Ice::Current&)
{
    IceUtil::Mutex::Lock lock(*this);
    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    _destroyed = true;

    try
    {
        _library->remove(description);
    }
    catch(const Freeze::DatabaseException& ex)
    {
        DatabaseException e;
        e.message = ex.message;
        throw e;
    }
}

Demo::BookDescription
BookI::getBookDescription(const Ice::Current&) const
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    //
    // Returns a copy
    //
    return description;
}

string
BookI::getRenterName(const Ice::Current&) const
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    if(rentalCustomerName.empty())
    {
        throw BookNotRentedException();
    }
    return rentalCustomerName;
}

void
BookI::rentBook(const string& name, const Ice::Current&)
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    if(!rentalCustomerName.empty())
    {
        throw BookRentedException();
    }
    rentalCustomerName = name;
}

void
BookI::returnBook(const Ice::Current&)
{
    IceUtil::Mutex::Lock lock(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    if(rentalCustomerName.empty())
    {
        throw BookNotRentedException();
    }
    rentalCustomerName.clear();;
}

// Needs to be extern for the Sun C++ 5.4 compiler
//
extern Ice::Identity
createBookIdentity(const string& isbn)
{
    //
    // Note that the identity category is important since the locator
    // was installed for the category 'book'.
    //
    Ice::Identity ident;
    ident.category = "book";
    ident.name = isbn;

    return ident;
}

class IsbnToBook
{
public:

    IsbnToBook(const Ice::ObjectAdapterPtr& adapter) :
        _adapter(adapter)
    {
    }

    BookPrx
    operator()(const string& isbn)
    {
        return BookPrx::uncheckedCast(_adapter->createProxy(createBookIdentity(isbn)));
    }

private:

    const Ice::ObjectAdapterPtr _adapter;
};

LibraryI::LibraryI(const Ice::CommunicatorPtr& communicator, 
                   const string& envName, const string& dbName,
                   const Freeze::EvictorPtr& evictor) :
    _evictor(evictor),
    _connection(Freeze::createConnection(communicator, envName)),
    _authors(_connection, dbName)
{
}

Demo::BookPrx
LibraryI::createBook(const Demo::BookDescription& description, const Ice::Current& c)
{
    IceUtil::Mutex::Lock lock(*this);

#if defined(__SUNPRO_CC)
    //
    // Strange CC bug (only when optimizing and raising BookExistsException)
    //
    BookPrx book;
    {
        book = IsbnToBook(c.adapter)(description.isbn);
    }
#else
    BookPrx book = IsbnToBook(c.adapter)(description.isbn);
#endif
    try
    {
        book->ice_ping();

        //
        // The book already exists.
        //
        throw BookExistsException();
    }
    catch(const Ice::ObjectNotExistException&)
    {
        //
        // Book doesn't exist, ignore the exception.
        //
    }

    BookPtr bookI = new BookI(this);
    bookI->description = description;

    //
    // Create a new Ice Object in the evictor, using the new identity
    // and the new Servant.
    //
    // This can throw EvictorDeactivatedException (which indicates an
    // internal error). The exception is currently ignored.
    //
    Ice::Identity ident = createBookIdentity(description.isbn);
    _evictor->add(bookI, ident);

    //
    // Add the isbn number to the authors map.
    //
    Ice::StringSeq isbnSeq;

    StringIsbnSeqDict::iterator p =  _authors.find(description.authors);
    if(p != _authors.end())
    {
        isbnSeq = p->second;
    }

    isbnSeq.push_back(description.isbn);
    _authors.put(StringIsbnSeqDict::value_type(description.authors, isbnSeq));

    return book;
}

Demo::BookPrx
LibraryI::findByIsbn(const string& isbn, const Ice::Current& c) const
{
    //
    // No locking is necessary since no internal mutable state is
    // accessed.
    //

    try
    {
        BookPrx book = IsbnToBook(c.adapter)(isbn);
        book->ice_ping();
        return book;
    }
    catch(const Ice::ObjectNotExistException&)
    {
        //
        // Book doesn't exist, return a null proxy.
        //
        return 0;
    }
}

Demo::BookPrxSeq
LibraryI::findByAuthors(const string& authors, const Ice::Current& c) const
{
    IceUtil::Mutex::Lock lock(*this);

    //
    // Lookup all books that match the given authors, and return them
    // to the caller.
    //
    StringIsbnSeqDict::const_iterator p =  _authors.find(authors);

    BookPrxSeq books;

    if(p != _authors.end())
    {
        books.reserve(p->second.size());
        transform(p->second.begin(), p->second.end(), back_inserter(books), IsbnToBook(c.adapter));
    }

    return books;
}

void
LibraryI::setEvictorSize(::Ice::Int size, const Ice::Current&)
{
    //
    // No synchronization necessary, _evictor is immutable.
    //
    _evictor->setSize(size);
}

void
LibraryI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

void
LibraryI::remove(const BookDescription& description)
{
    IceUtil::Mutex::Lock lock(*this);
    
    //
    // Note: no need to catch and retry on deadlock since all access to
    // _authors is serialized.
    //

    try
    {
        StringIsbnSeqDict::iterator p = _authors.find(description.authors);
        
        assert(p != _authors.end());

        //
        // Remove the isbn number from the sequence.
        //
        Ice::StringSeq isbnSeq  = p->second;
        isbnSeq.erase(remove_if(isbnSeq.begin(), isbnSeq.end(), bind2nd(equal_to<string>(), description.isbn)),
                         isbnSeq.end());
        
        if(isbnSeq.empty())
        {
            //
            // If there are no further associated isbn numbers then remove
            // the record.
            //
            _authors.erase(p);
        }
        else
        {
            //
            // Otherwise, write back the new record.
            //
            p.set(isbnSeq);
        }

        //
        // This can throw EvictorDeactivatedException (which indicates
        // an internal error). The exception is currently ignored.
        //
        _evictor->remove(createBookIdentity(description.isbn));
    }
    catch(const Freeze::DatabaseException& ex)
    {
        DatabaseException e;
        e.message = ex.message;
        throw e;
    }
}
