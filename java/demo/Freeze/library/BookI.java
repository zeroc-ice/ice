// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

class BookI extends Book
{
    //
    // No read/write mutexes in Java - hence use native
    // synchronization.
    //

    public BookDescription
    getBookDescription(Ice.Current current)
    {
	//
	// Immutable.
	//
	return description;
    }

    synchronized public String
    getRenterName(Ice.Current current)
	throws BookNotRentedException
    {
	if(rentalCustomerName.length() == 0)
	{
	    throw new BookNotRentedException();
	}
	return rentalCustomerName;
    }

    synchronized public void
    rentBook(String name, Ice.Current current)
	throws BookRentedException
    {
	if(rentalCustomerName.length() != 0)
	{
	    throw new BookRentedException();
	}
	rentalCustomerName = name;
    }

    synchronized public void
    returnBook(Ice.Current current)
	throws BookNotRentedException
    {
	if(rentalCustomerName.length() == 0)
	{
	    throw new BookNotRentedException();
	}
	rentalCustomerName = new String();;
    }

    synchronized public void
    destroy(Ice.Current current)
	throws DatabaseException
    {
	try
	{
	    _library.remove(description);
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

    BookI(LibraryI library)
    {
	_library = library;

	//
	// This could be avoided by having two constructors (one for
	// new creation of a book, and the other for restoring a
	// previously saved book).
	//
	rentalCustomerName = new String();
    }

    private LibraryI _library;
}
