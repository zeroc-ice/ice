// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Freeze/Freeze.h>
#include <LibraryI.h>

using namespace std;

BookI::BookI(const LibraryIPtr& library, const Freeze::EvictorPtr& evictor) :
    _library(library),
    _evictor(evictor)
{
}

BookI::~BookI()
{
}

void
BookI::destroy(const Ice::Current&)
{
    IceUtil::RWRecMutex::RLock sync(*this);

    try
    {
	_library->remove(_description);

	//
	// This can throw EvictorDeactivatedException (which indicates
	// an internal error). The exception is currently ignored.
	//
	_evictor->destroyObject(createIdentity(_description.isbn));
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
BookI::getBookDescription(const Ice::Current&)
{
    // Immutable
    return _description;
}

::std::string
BookI::getRenterName(const Ice::Current&)
{
    IceUtil::RWRecMutex::RLock sync(*this);

    if (_rentalCustomerName.empty())
    {
	throw BookNotRentedException();
    }
    return _rentalCustomerName;
}

void
BookI::rentBook(const ::std::string& name, const Ice::Current&)
{
    IceUtil::RWRecMutex::WLock sync(*this);

    if (!_rentalCustomerName.empty())
    {
	throw BookRentedException();
    }
    _rentalCustomerName = name;
}

void
BookI::returnBook(const Ice::Current&)
{
    IceUtil::RWRecMutex::WLock sync(*this);

    if (_rentalCustomerName.empty())
    {
	throw BookNotRentedException();
    }
    _rentalCustomerName.clear();;
}

Ice::Identity
BookI::createIdentity(const string& isbn)
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
	Ice::Identity ident = BookI::createIdentity(isbn);
	return BookPrx::uncheckedCast(_adapter->createProxy(ident));
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
	book = IsbnToBook(_adapter)(description.isbn);
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

    BookPtr bookI = new BookI(this, _evictor);
    bookI->_description = description;

    Ice::Identity ident = BookI::createIdentity(description.isbn);
    _evictor->createObject(ident, bookI);

    Ice::StringSeq isbnSeq;

    StringIsbnSeqDict::iterator p =  _authors.find(description.authors);
    if (p != _authors.end())
    {
	isbnSeq = p->second;
    }

    isbnSeq.push_back(description.isbn);
    _authors.insert(make_pair(description.authors, isbnSeq));

    return book;
}

::BookPrx
LibraryI::findByIsbn(const string& isbn, const Ice::Current&)
{
    //
    // No locking is necessary since no internal mutable state is
    // accessed.
    //
    //IceUtil::RWRecMutex::RLock sync(*this);

    try
    {
	Ice::Identity ident = BookI::createIdentity(isbn);
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
LibraryI::findByAuthors(const string& authors, const Ice::Current&)
{
    IceUtil::RWRecMutex::RLock sync(*this);

    StringIsbnSeqDict::const_iterator p =  _authors.find(authors);

    BookPrxSeq books;

    if (p != _authors.end())
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
	if (p == _authors.end())
	{
	    throw Freeze::DBNotFoundException(__FILE__, __LINE__);
	}

	Ice::StringSeq isbnSeq  = p->second;
	isbnSeq.erase(remove_if(isbnSeq.begin(), isbnSeq.end(), bind2nd(equal_to<string>(), description.isbn)),
			 isbnSeq.end());
	
	if (isbnSeq.empty())
	{
	    _authors.erase(p);
	}
	else
	{
	    _authors.insert(make_pair(description.authors, isbnSeq));
	}
    }
    catch(const Freeze::DBException& ex)
    {
	DatabaseException e;
	e.message = ex.message;
	throw e;
    }
}

