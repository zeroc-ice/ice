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
            "isbn NUMBER             Find all books that start with the given ISBN number.\n" +
            "authors NAME            Find all books by the given authors.\n" +
            "title NAME              Find all books which have the given title.\n" +
            "next                    Set the current book to the next one that was found.\n" +
            "current                 Display the current book.\n" +
            "rent NAME               Rent the current book for customer NAME.\n" +
            "return                  Return the currently rented book.\n" +
            "remove                  Permanently remove the current book from the library.\n");
    }

    void
    addBook(java.util.List<String> args)
    {
        if(args.size() != 3)
        {
            error("`add' requires exactly three arguments (type `help' for more info)");
            return;
        }

        try
        {
            String isbn = args.get(0);
            String title = args.get(1);

            java.util.List<String> authors = new java.util.LinkedList<String>();
            java.util.StringTokenizer st = new java.util.StringTokenizer(args.get(2), ",");
            while(st.hasMoreTokens())
            {
                authors.add(st.nextToken().trim());
            }

            BookPrx book = _library.createBook(isbn, title, authors);
            System.out.println("added new book with isbn " + isbn);
        }
        catch(BookExistsException ex)
        {
            error("the book already exists");
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
            if(_query != null)
            {
                try
                {
                    _query.destroy();
                }
                catch(Exception e)
                {
                    // Ignore
                }
                _query = null;
                _current = null;
            }

            BookDescriptionSeqHolder first = new BookDescriptionSeqHolder();
            Ice.IntHolder nrows = new Ice.IntHolder();
            BookQueryResultPrxHolder result = new BookQueryResultPrxHolder();
            _library.queryByIsbn(args.get(0), 1, first, nrows, result);

            System.out.println(nrows.value + " results");
            if(nrows.value == 0)
            {   
                return;
            }

            _current = first.value.get(0);
            _query = result.value;
            printCurrent();
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
            if(_query != null)
            {
                try
                {
                    _query.destroy();
                }
                catch(Exception e)
                {
                    // Ignore
                }
                _query = null;
                _current = null;
            }

            BookDescriptionSeqHolder first = new BookDescriptionSeqHolder();
            Ice.IntHolder nrows = new Ice.IntHolder();
            BookQueryResultPrxHolder result = new BookQueryResultPrxHolder();
            _library.queryByAuthor(args.get(0), 1, first, nrows, result);

            System.out.println(nrows.value + " results");
            if(nrows.value == 0)
            {   
                return;
            }

            _current = first.value.get(0);
            _query = result.value;
            printCurrent();
        }
        catch(Ice.LocalException ex)
        {
            error(ex.toString());
        }
    }

    void
    findTitle(java.util.List<String> args)
    {
        if(args.size() != 1)
        {
            error("`title' requires exactly one argument (type `help' for more info)");
            return;
        }

        try
        {
            if(_query != null)
            {
                try
                {
                    _query.destroy();
                }
                catch(Exception e)
                {
                    // Ignore
                }
                _query = null;
                _current = null;
            }

            BookDescriptionSeqHolder first = new BookDescriptionSeqHolder();
            Ice.IntHolder nrows = new Ice.IntHolder();
            BookQueryResultPrxHolder result = new BookQueryResultPrxHolder();
            _library.queryByTitle(args.get(0), 1, first, nrows, result);

            System.out.println(nrows.value + " results");
            if(nrows.value == 0)
            {   
                return;
            }

            _current = first.value.get(0);
            _query = result.value;
            printCurrent();
        }
        catch(Ice.LocalException ex)
        {
            error(ex.toString());
        }
    }

    void
    nextFoundBook()
    {
        if(_query == null)
        {
            System.out.println("no next book");
            return;
        }

        try
        {
            Ice.BooleanHolder destroyed = new Ice.BooleanHolder();
            java.util.List<BookDescription> next = _query.next(1, destroyed);
            if(next.size() > 0)
            {
                _current = next.get(0);
            }
            else
            {
                assert destroyed.value;
                _current = null;
            }
            if(destroyed.value)
            {
                _query = null;
            }
            printCurrent();
        }
        catch(Ice.ObjectNotExistException ex)
        {
            System.out.println("the query object no longer exists");
        }
        catch(Ice.LocalException ex)
        {
            error(ex.toString());
        }
    }

    void
    printCurrent()
    {
        if(_current != null)
        {
            System.out.println("current book is:");
            System.out.println("isbn: " + _current.isbn);
            System.out.println("title: " + _current.title);
            System.out.println("authors: " + _current.authors);
            if(_current.rentedBy.length() > 0)
            {
                System.out.println("rented: " + _current.rentedBy);
            }
        }
        else
        {
            System.out.println("no current book");
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
            if(_current != null)
            {
                _current.proxy.rentBook(args.get(0));
                System.out.println("the book is now rented by `" + (String)args.get(0) + "'");
                _current = _current.proxy.describe();
            }
            else
            {
                System.out.println("no current book");
            }
        }
        catch(BookRentedException ex)
        {
            System.out.println("the book has already been rented");
        }
        catch(InvalidCustomerException ex)
        {
            System.out.println("the customer name is invalid");
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
            if(_current != null)
            {
                _current.proxy.returnBook();
                System.out.println( "the book has been returned");
                _current = _current.proxy.describe();
            }
            else
            {
                System.out.println("no current book");
            }
        }
        catch(BookNotRentedException ex)
        {
            System.out.println("the book is not currently rented");
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
            if(_current != null)
            {
                _current.proxy.destroy();
                _current = null;
                System.out.println("removed current book" );
            }
            else
            {
                System.out.println("no current book" );
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
        _query = null;
        _current = null;

        _in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));

        Grammar g = new Grammar(this);
        g.parse();

        return 0;
    }

    int
    parse(String file)
    {
        _query = null;
        _current = null;

        try
        {
            _in = new java.io.BufferedReader(new java.io.FileReader(file));
        }
        catch(java.io.FileNotFoundException ex)
        {
            error(ex.getMessage());
            return 1;
        }

        Grammar g = new Grammar(this);
        g.parse();

        System.out.println();

        try
        {
            _in.close();
        }
        catch(java.io.IOException ex)
        {
        }

        return 0;
    }

    private BookQueryResultPrx _query;
    private BookDescription _current;

    private LibraryPrx _library;

    private java.io.BufferedReader _in;
    private boolean _interactive;
}
