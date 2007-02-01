// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class LibraryI extends _LibraryDisp
{
    public synchronized BookPrx
    createBook(BookDescription description, Ice.Current current)
        throws DatabaseException, BookExistsException
    {
        BookPrx book = isbnToBook(description.isbn, current.adapter);

        try
        {
            book.ice_ping();

            //
            // The book already exists.
            //
            throw new BookExistsException();
        }
        catch(Ice.ObjectNotExistException e)
        {
            //
            // Book doesn't exist, ignore the exception.
            //
        }

        //
        // Create a new book Servant.
        //
        BookI bookI = new BookI(this);
        bookI.description = description;

        Ice.Identity ident = createBookIdentity(description.isbn);

        //
        // Create a new Ice Object in the evictor, using the new
        // identity and the new Servant.
        //
        // This can throw EvictorDeactivatedException (which indicates
        // an internal error). The exception is currently ignored.
        //
        _evictor.add(bookI, ident);

        try
        {
            //
            // Add the isbn number to the authors map.
            //
            String[] isbnSeq = (String[])_authors.get(description.authors);
            int length = (isbnSeq == null) ? 0 : isbnSeq.length;
            String[] newIsbnSeq = new String[length+1];

            if(isbnSeq != null)
            {
                System.arraycopy(isbnSeq, 0, newIsbnSeq, 0, length);
            }
            newIsbnSeq[length] = description.isbn;

            _authors.fastPut(description.authors, newIsbnSeq);
        
            return book;
        }
        catch(Freeze.DatabaseException ex)
        {
            DatabaseException e = new DatabaseException();
            e.message = ex.message;
            throw e;
        }
    }

    //
    // No locking is necessary since no internal mutable state is
    // accessed.
    //
    public BookPrx
    findByIsbn(String isbn, Ice.Current current)
        throws DatabaseException
    {
        try
        {
            BookPrx book = isbnToBook(isbn, current.adapter);
            book.ice_ping();

            return book;
        }
        catch(Ice.ObjectNotExistException ex)
        {
            //
            // Book doesn't exist, return a null proxy.
            //
            return null;
        }
    }

    public synchronized BookPrx[]
    findByAuthors(String authors, Ice.Current current)
        throws DatabaseException
    {
        try
        {
            //
            // Lookup all books that match the given authors, and
            // return them to the caller.
            //
            String[] isbnSeq = (String[])_authors.get(authors);

            int length = (isbnSeq == null) ? 0 : isbnSeq.length;
            BookPrx[] books = new BookPrx[length];

            if(isbnSeq != null)
            {
                for(int i = 0; i < length; ++i)
                {
                    books[i] = isbnToBook(isbnSeq[i], current.adapter);
                }
            }

            return books;
        }
        catch(Freeze.DatabaseException ex)
        {
            DatabaseException e = new DatabaseException();
            e.message = ex.message;
            throw e;
        }
    }

    public void
    setEvictorSize(int size, Ice.Current current)
        throws DatabaseException
    {
        //
        // No synchronization necessary, _evictor is immutable.
        //
        _evictor.setSize(size);
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    protected synchronized void
    remove(BookDescription description)
        throws DatabaseException
    {
        try
        {
            String[] isbnSeq = (String[])_authors.get(description.authors);

            assert isbnSeq != null;
            

            int i;
            for(i = 0; i < isbnSeq.length; ++i)
            {
                if(isbnSeq[i].equals(description.isbn))
                {
                    break;
                }
            }

            assert i < isbnSeq.length;

            if(isbnSeq.length == 1)
            {
                //
                // If there are no further associated isbn numbers then remove
                // the record.
                //
                _authors.fastRemove(description.authors);
            }
            else
            {
                //
                // Remove the isbn number from the sequence and write
                // back the new record.
                //
                String[] newIsbnSeq = new String[isbnSeq.length-1];
                System.arraycopy(isbnSeq, 0, newIsbnSeq, 0, i);
                if(i < isbnSeq.length - 1)
                {
                    System.arraycopy(isbnSeq, i+1, newIsbnSeq, i, isbnSeq.length - i - 1);
                }
            
                _authors.fastPut(description.authors, newIsbnSeq);
            }

            //
            // This can throw EvictorDeactivatedException (which
            // indicates an internal error). The exception is
            // currently ignored.
            //
            _evictor.remove(createBookIdentity(description.isbn));
        }
        catch(Freeze.DatabaseException ex)
        {
            DatabaseException e = new DatabaseException();
            e.message = ex.message;
            throw e;
        }
    }

    LibraryI(Ice.Communicator communicator, String envName, String dbName, Freeze.Evictor evictor)
    {
        _evictor = evictor;
        _connection = Freeze.Util.createConnection(communicator, envName);
        _authors = new StringIsbnSeqDict(_connection, dbName, true);
    }

    void
    close()
    {
        _authors.close();
        _connection.close();
    }

    private Ice.Identity
    createBookIdentity(String isbn)
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

    private BookPrx
    isbnToBook(String isbn, Ice.ObjectAdapter adapter)
    {
        return BookPrxHelper.uncheckedCast(adapter.createProxy(createBookIdentity(isbn)));
    }

    private Freeze.Evictor _evictor;
    private Freeze.Connection _connection;
    private StringIsbnSeqDict _authors;
}
