// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

class BookI extends Book
{
    //
    // No read/write mutexes in Java - hence use native
    // syncronization.
    //

    public BookDescription
    getBookDescription(Ice.Current current)
    {
	//
	// Immutable.
	//
	return _description;
    }

    synchronized public String
    getRenterName(Ice.Current current)
	throws BookNotRentedException
    {
	if (_rentalCustomerName.length() == 0)
	{
	    throw new BookNotRentedException();
	}
	return _rentalCustomerName;
    }

    synchronized public void
    rentBook(String name, Ice.Current current)
	throws BookRentedException
    {
	if (_rentalCustomerName.length() != 0)
	{
	    throw new BookRentedException();
	}
	_rentalCustomerName = name;
    }

    synchronized public void
    returnBook(Ice.Current current)
	throws BookNotRentedException
    {
	if (_rentalCustomerName.length() == 0)
	{
	    throw new BookNotRentedException();
	}
	_rentalCustomerName = new String();;
    }

    synchronized public void
    destroy(Ice.Current current)
	throws DatabaseException
    {
	try
	{
	    _library.remove(_description);

	    //
	    // This can throw EvictorDeactivatedException (which
	    // indicates an internal error). The exception is
	    // currently ignored.
	    //
	    _evictor.destroyObject(createIdentity(_description.isbn));
	}
	catch(Freeze.DBNotFoundException ex)
	{
	    //
	    // Raised by remove. Ignore.
	    //
	}
	catch(Freeze.DBException ex)
	{
	    DatabaseException e = new DatabaseException();
	    e.message = ex.message;
	    throw e;
	}
    }

    public static Ice.Identity
    createIdentity(String isbn)
    {
	//
	// Note that the identity category is important since the
	// locator was installed for the category 'book'.
	//
	Ice.Identity ident = new Ice.Identity();
	ident.category = "book";
	ident.name = isbn;
	
	return ident;
    }
    
    BookI(LibraryI library, Freeze.Evictor evictor)
    {
	_library = library;
	_evictor = evictor;

	//
	// This could be avoided by having two constructors (one for
	// new creation of a book, and the other for restoring a
	// previously saved book).
	//
	_rentalCustomerName = new String();
    }

    private LibraryI _library;
    private Freeze.Evictor _evictor;
}
