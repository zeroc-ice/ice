// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class Parser
{
    Parser(Ice.Communicator communicator, LibraryPrx library)
    {
        _library = library;
    }

    void
    usage()
    {
        System.err.print(
            "help                    Print this message.\n" +
            "exit, quit              Exit this program.\n" +
            "add isbn title authors  Create new book.\n" +
            "isbn NUMBER             Find the book with given ISBN number.\n" +
            "authors NAME            Find all books by the given authors.\n" +
            "next                    Set the current book to the next one that was found.\n" +
            "current                 Display the current book.\n" +
            "rent NAME               Rent the current book for customer NAME.\n" +
            "return                  Return the currently rented book.\n" +
            "remove                  Permanently remove the current book from the library.\n" +
            "size SIZE               Set the evictor size for books to SIZE.\n" +
            "shutdown                Shut the library server down.\n");
    }

    void
    addBook(java.util.List<String> args)
    {
        if(args.size() != 3)
        {
            error("`add' requires at exactly three arguments (type `help' for more info)");
            return;
        }

        try
        {
            BookDescription desc = new BookDescription();
            desc.isbn = args.get(0);
            desc.title = args.get(1);
            desc.authors = args.get(2);

            BookPrx book = _library.createBook(desc);
            System.out.println("added new book with isbn " + desc.isbn);
        }
        catch(DatabaseException ex)
        {
            error(ex.message);
        }
        catch(BookExistsException ex)
        {
            error("the book already exists.");
        }
        catch(Ice.LocalException ex)
        {
            error(ex.toString());
        }
    }

    void
    findIsbn(java.util.List<String> args)
    {
        if(args.size() != 1)
        {
            error("`isbn' requires exactly one argument (type `help' for more info)");
            return;
        }

        try
        {
            _foundBooks = null;
            _current = 0;

            BookPrx book = _library.findByIsbn(args.get(0));
            if(book == null)
            {
                System.out.println("no book with that ISBN number exists.");
            }
            else
            {
                _foundBooks = new BookPrx[1];
                _foundBooks[0] = book;
                printCurrent();
            }
        }
        catch(DatabaseException ex)
        {
            error(ex.message);
        }
        catch(Ice.LocalException ex)
        {
            error(ex.toString());
        }
    }

    void
    findAuthors(java.util.List<String> args)
    {
        if(args.size() != 1)
        {
            error("`authors' requires exactly one argument (type `help' for more info)");
            return;
        }

        try
        {
            _foundBooks = _library.findByAuthors(args.get(0));
            _current = 0;
            System.out.println("number of books found: " + _foundBooks.length);
            printCurrent();
        }
        catch(DatabaseException ex)
        {
            error(ex.message);
        }
        catch(Ice.LocalException ex)
        {
            error(ex.toString());
        }
    }

    void
    nextFoundBook()
    {
        if(_foundBooks != null && _current != _foundBooks.length)
        {
            ++_current;
        }
        printCurrent();
    }

    void
    printCurrent()
    {
        try
        {
            if(_foundBooks != null && _current != _foundBooks.length)
            {
                BookDescription desc = _foundBooks[_current].getBookDescription();
                String renter = null;
                try
                {
                    renter = _foundBooks[_current].getRenterName();
                }
                catch(BookNotRentedException ex)
                {
                }

                System.out.println("current book is:" );
                System.out.println("isbn: " + desc.isbn);
                System.out.println("title: " + desc.title);
                System.out.println("authors: " + desc.authors);
                if(renter != null)
                {
                    System.out.println("rented: " + renter);
                }
            }
            else
            {
                System.out.println("no current book");
            }
        }
        catch(Ice.ObjectNotExistException ex)
        {
            System.out.println("current book no longer exists");
        }
        catch(Ice.LocalException ex)
        {
            error(ex.toString());
        }
    }

    void
    rentCurrent(java.util.List<String> args)
    {
        if(args.size() != 1)
        {
            error("`rent' requires exactly one argument (type `help' for more info)");
            return;
        }

        try
        {
            if(_foundBooks != null && _current != _foundBooks.length)
            {
                _foundBooks[_current].rentBook(args.get(0));
                System.out.println("the book is now rented by `" + args.get(0) + "'");
            }
            else
            {
                System.out.println("no current book");
            }
        }
        catch(BookRentedException ex)
        {
            System.out.println("the book has already been rented.");
        }
        catch(Ice.ObjectNotExistException ex)
        {
            System.out.println("current book no longer exists");
        }
        catch(Ice.LocalException ex)
        {
            error(ex.toString());
        }
    }

    void
    returnCurrent()
    {
        try
        {
            if(_foundBooks != null && _current != _foundBooks.length)
            {
                _foundBooks[_current].returnBook();
                System.out.println( "the book has been returned.");
            }
            else
            {
                System.out.println("no current book");
            }
        }
        catch(BookNotRentedException ex)
        {
            System.out.println("the book is not currently rented.");
        }
        catch(Ice.ObjectNotExistException ex)
        {
            System.out.println("current book no longer exists");
        }
        catch(Ice.LocalException ex)
        {
            error(ex.toString());
        }
    }

    void
    removeCurrent()
    {
        try
        {
            if(_foundBooks != null && _current != _foundBooks.length)
            {
                _foundBooks[_current].destroy();
                System.out.println("removed current book" );
            }
            else
            {
                System.out.println("no current book" );
            }
        }
        catch(DatabaseException ex)
        {
            error(ex.message);
        }
        catch(Ice.ObjectNotExistException ex)
        {
            System.out.println("current book no longer exists");
        }
        catch(Ice.LocalException ex)
        {
            error(ex.toString());
        }
    }

    void
    setEvictorSize(java.util.List<String> args)
    {
        if(args.size() != 1)
        {
            error("`size' requires exactly one argument (type `help' for more info)");
            return;
        }

        String s = args.get(0);
        try
        {
            _library.setEvictorSize(Integer.parseInt(s));
        }
        catch(NumberFormatException ex)
        {
            error("not a number " + s);
        }
        catch(DatabaseException ex)
        {
            error(ex.message);
        }
        catch(Ice.LocalException ex)
        {
            error(ex.toString());
        }
    }

    void
    shutdown()
    {
        try
        {
            _library.shutdown();
        }
        catch(Ice.LocalException ex)
        {
            error(ex.toString());
        }
    }

    void
    error(String s)
    {
        System.err.println("error: " + s);
    }

    void
    warning(String s)
    {
        System.err.println("warning: " + s);
    }

    String
    getInput()
    {
        System.out.print(">>> ");
        System.out.flush();

        try
        {
            return _in.readLine();
        }
        catch(java.io.IOException e)
        {
            return null;
        }
    }

    int
    parse()
    {
        _foundBooks = new BookPrx[0];
        _current = 0;

        _in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        Grammar g = new Grammar(this);
        g.parse();

        return 0;
    }

    private BookPrx[] _foundBooks;
    private int _current;

    private LibraryPrx _library;

    private java.io.BufferedReader _in;
    private boolean _interactive;
}
