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

#include <Freeze/Freeze.h>
#include <LibraryI.h>

using namespace std;

BookI::BookI(const LibraryIPtr& library) :
    _library(library), _destroyed(false)
{
}

BookI::~BookI()
{
}

void
BookI::destroy(const Ice::Current&)
{
    IceUtil::RWRecMutex::RLock sync(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    _destroyed = true;

    try
    {
	_library->remove(description);
    }
    catch(const Freeze::DBNotFoundException&)
    {
	//
	// Raised by remove. Ignore.
	//
    }
    catch(const Freeze::DBException& ex)
    {
	DatabaseException e;
	e.message = ex.message;
	throw e;
    }
}

::BookDescription
BookI::getBookDescription(const Ice::Current&) const
{
    IceUtil::RWRecMutex::RLock sync(*this);

    if(_destroyed)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    // Immutable
    return description;
}

::std::string
BookI::getRenterName(const Ice::Current&) const
{
    IceUtil::RWRecMutex::RLock sync(*this);

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
BookI::rentBook(const ::std::string& name, const Ice::Current&)
{
    IceUtil::RWRecMutex::WLock sync(*this);

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
    IceUtil::RWRecMutex::WLock sync(*this);

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

static Ice::Identity
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

    Ice::ObjectAdapterPtr _adapter;
};

LibraryI::LibraryI(const Ice::ObjectAdapterPtr& adapter, const Freeze::DBPtr& db, const Freeze::EvictorPtr& evictor) :
    _adapter(adapter),
    _evictor(evictor),
    _authors(db)
{
}

LibraryI::~LibraryI()
{
}

::BookPrx
LibraryI::createBook(const ::BookDescription& description, const Ice::Current&)
{
    IceUtil::RWRecMutex::WLock sync(*this);

    BookPrx book = IsbnToBook(_adapter)(description.isbn);
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
    _evictor->createObject(ident, bookI);

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
    _authors.insert(make_pair(description.authors, isbnSeq));

    return book;
}

::BookPrx
LibraryI::findByIsbn(const string& isbn, const Ice::Current&) const
{
    //
    // No locking is necessary since no internal mutable state is
    // accessed.
    //
    //IceUtil::RWRecMutex::RLock sync(*this);

    try
    {
	BookPrx book = IsbnToBook(_adapter)(isbn);
	book->ice_ping();
	return book;
    }
    catch(const Ice::ObjectNotExistException&)
    {
	//
	// Book doesn't exist, return a null proxy.
	//
	return BookPrx();
    }
}

::BookPrxSeq
LibraryI::findByAuthors(const string& authors, const Ice::Current&) const
{
    IceUtil::RWRecMutex::RLock sync(*this);

    //
    // Lookup all books that match the given authors, and return them
    // to the caller.
    //
    StringIsbnSeqDict::const_iterator p =  _authors.find(authors);

    BookPrxSeq books;

    if(p != _authors.end())
    {
	books.reserve(p->second.size());
	transform(p->second.begin(), p->second.end(), back_inserter(books), IsbnToBook(_adapter));
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
    //
    // No synchronization necessary, _adapter is immutable.
    //
    _adapter->getCommunicator()->shutdown();
}

void
LibraryI::remove(const BookDescription& description)
{
    IceUtil::RWRecMutex::WLock sync(*this);

    try
    {
	StringIsbnSeqDict::iterator p = _authors.find(description.authors);
	
	//
	// If the title isn't found then raise a record not found
	// exception.
	//
	if(p == _authors.end())
	{
	    throw Freeze::DBNotFoundException(__FILE__, __LINE__);
	}

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
	    _authors.insert(make_pair(description.authors, isbnSeq));
	}

	//
	// This can throw EvictorDeactivatedException (which indicates
	// an internal error). The exception is currently ignored.
	//
	_evictor->destroyObject(createBookIdentity(description.isbn));
    }
    catch(const Freeze::DBException& ex)
    {
	DatabaseException e;
	e.message = ex.message;
	throw e;
    }
}

