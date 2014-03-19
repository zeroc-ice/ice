// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.library.controller;

import java.util.ArrayList;
import java.util.List;

import android.os.Handler;

public class QueryController
{
    public interface Listener
    {
        void onDataChange(QueryModel data, boolean saved);
        void onError();
    }

    public static final int NO_BOOK = -1;
    public static final int NEW_BOOK = -2;

    public enum QueryType
    {
        ISBN, TITLE, AUTHOR
    }

    private ArrayList<Demo.BookDescription> _books = new ArrayList<Demo.BookDescription>();
    private int _nrows = 0;
    private int _rowsQueried = 0;
    private Demo.BookQueryResultPrx _query = null;
    private Listener _listener;
    private Handler _handler;
    private Demo.LibraryPrx _library;
    private int _currentBook = NO_BOOK;
    private String _lastError;

    synchronized protected void postDataChanged(final boolean saved)
    {
        if(_listener != null)
        {
            _handler.post(new Runnable()
            {
                public void run()
                {
                    _listener.onDataChange(getQueryModel(), saved);
                }
            });
        }
    }

    synchronized protected void postError(final String string)
    {
        _lastError = string;
        if(_listener != null)
        {
            _handler.post(new Runnable()
            {
                public void run()
                {
                    _listener.onError();
                }
            });
        }
    }

    synchronized private void queryResponse(List<Demo.BookDescription> first, int nrows, Demo.BookQueryResultPrx result)
    {
        _books.clear();
        _nrows = nrows;
        _books.addAll(first);
        _query = result;
        if(_listener != null)
        {
            postDataChanged(false);
        }
    }

    synchronized private QueryModel getQueryModel()
    {
        QueryModel data = new QueryModel();
        data.books = new ArrayList<Demo.BookDescription>();
        for(Demo.BookDescription book : _books)
        {
            data.books.add((Demo.BookDescription)book.clone());
        }
        data.nrows = _nrows;
        if(_currentBook == NEW_BOOK)
        {
            data.currentBook = new Demo.BookDescription();
            data.currentBook.proxy = null;
            data.currentBook.isbn = "";
            data.currentBook.title = "";
            data.currentBook.rentedBy = "";
            data.currentBook.authors = new ArrayList<String>();
        }
        else if(_currentBook != NO_BOOK)
        {
            data.currentBook = (Demo.BookDescription)_books.get(_currentBook).clone();
        }
        return data;
    }

    // An empty query
    QueryController(Handler handler, Demo.LibraryPrx library)
    {
        _handler = handler;
        _library = library;
    }

    QueryController(Handler handler, Demo.LibraryPrx library, final Listener listener, final QueryType _type, final String _queryString)
    {
        _handler = handler;
        _listener = listener;
        _library = library;

        // Send the initial data change notification.
        _listener.onDataChange(getQueryModel(), false);
        _rowsQueried = 10;

        if(_type == QueryType.ISBN)
        {
            Demo.Callback_Library_queryByIsbn cb = new Demo.Callback_Library_queryByIsbn()
            {
                @Override
                public void exception(Ice.LocalException ex)
                {
                    postError(ex.toString());
                }

                @Override
                public void response(List<Demo.BookDescription> first, int nrows, Demo.BookQueryResultPrx result)
                {
                    queryResponse(first, nrows, result);
                }
            };

            _library.begin_queryByIsbn(_queryString, 10, cb);
        }
        else if(_type == QueryType.AUTHOR)
        {
            Demo.Callback_Library_queryByAuthor cb = new Demo.Callback_Library_queryByAuthor()
            {
                @Override
                public void exception(Ice.LocalException ex)
                {
                    postError(ex.toString());
                }

                @Override
                public void response(List<Demo.BookDescription> first, int nrows, Demo.BookQueryResultPrx result)
                {
                    queryResponse(first, nrows, result);
                }
            };

            _library.begin_queryByAuthor(_queryString, 10, cb);
        }
        else
        {
            Demo.Callback_Library_queryByTitle cb = new Demo.Callback_Library_queryByTitle()
            {
                @Override
                public void exception(Ice.LocalException ex)
                {
                    postError(ex.toString());
                }

                @Override
                public void response(List<Demo.BookDescription> first, int nrows, Demo.BookQueryResultPrx result)
                {
                    queryResponse(first, nrows, result);
                }
            };
            _library.begin_queryByTitle(_queryString, 10, cb);
        }
    }

    protected void destroy()
    {
        if(_query != null)
        {
            _query.begin_destroy(new Demo.Callback_BookQueryResult_destroy()
            {
                @Override
                public void exception(Ice.LocalException ex)
                {
                }

                @Override
                public void response()
                {
                }
            });
            _query = null;
        }
    }

    synchronized public void setListener(Listener cb)
    {
        _listener = cb;
        _listener.onDataChange(getQueryModel(), false);
        if(_lastError != null)
        {
            _listener.onError();
        }
    }

    synchronized public String getLastError()
    {
        return _lastError;
    }

    synchronized public void clearLastError()
    {
        _lastError = null;
    }

    synchronized public void getMore(int position)
    {
        assert position < _nrows;
        if(position < _rowsQueried)
        {
            return;
        }

        Demo.Callback_BookQueryResult_next cb = new Demo.Callback_BookQueryResult_next()
        {
            @Override
            public void exception(Ice.LocalException ex)
            {
                postError(ex.toString());
            }

            @Override
            public void response(final List<Demo.BookDescription> ret, final boolean destroyed)
            {
                synchronized(QueryController.this)
                {
                    _books.addAll(ret);
                    postDataChanged(false);
                }
            }
        };
        _query.begin_next(10, cb);
        _rowsQueried += 10;
    }

    synchronized public boolean setCurrentBook(int row)
    {
        if(row < _books.size())
        {
            _currentBook = row;
            return true;
        }
        return false;
    }

    synchronized public void returnBook()
    {
        assert _currentBook != NO_BOOK;
        final Demo.BookDescription desc = _books.get(_currentBook);
        Demo.Callback_Book_returnBook returnBookCB = new Demo.Callback_Book_returnBook()
        {
            @Override
            public void exception(Ice.LocalException ex)
            {
                postError(ex.toString());
            }

            @Override
            public void exception(Ice.UserException ex)
            {
                final String error;
                if(ex instanceof Demo.BookNotRentedException)
                {
                    error = "The book is no longer rented.";
                }
                else
                {
                    error = "An error occurred: " + ex.toString();
                }
                postError(error);
            }

            @Override
            public void response()
            {
                synchronized(QueryController.this)
                {
                    desc.rentedBy = "";
                    postDataChanged(false);
                }
            }
        };
        desc.proxy.begin_returnBook(returnBookCB);
    }

    synchronized public void rentBook(final String r)
    {
        assert _currentBook != NO_BOOK;
        final Demo.BookDescription desc = _books.get(_currentBook);
        Demo.Callback_Book_rentBook rentBookCB = new Demo.Callback_Book_rentBook()
        {
            @Override
            public void exception(final Ice.LocalException ex)
            {
                postError(ex.toString());
            }

            @Override
            public void exception(Ice.UserException ex)
            {
                final String error;
                if(ex instanceof Demo.InvalidCustomerException)
                {
                    error = "The customer name is invalid.";
                }
                else if(ex instanceof Demo.BookRentedException)
                {
                    error = "That book is already rented.";
                }
                else
                {
                    error = "An error occurred: " + ex.toString();
                }
                postError(error);
            }

            @Override
            public void response()
            {
                synchronized(QueryController.this)
                {
                    desc.rentedBy = r;
                    postDataChanged(false);
                }
            }
        };
        desc.proxy.begin_rentBook(r, rentBookCB);
    }

    synchronized public void deleteBook()
    {
        assert _currentBook != NO_BOOK;
        final Demo.BookDescription desc = _books.get(_currentBook);
        desc.proxy.begin_destroy(new Demo.Callback_Book_destroy()
        {
            @Override
            public void exception(Ice.LocalException ex)
            {
                postError(ex.toString());
            }

            @Override
            public void response()
            {
                synchronized(QueryController.this)
                {
                    _books.remove(_currentBook);
                    _currentBook = NO_BOOK;
                    --_nrows;
                    postDataChanged(false);
                }
            }
        });
    }

    synchronized public boolean saveBook(final Demo.BookDescription newDesc)
    {
        assert _currentBook != NO_BOOK;
        if(_currentBook == NEW_BOOK)
        {
            _library.begin_createBook(newDesc.isbn, newDesc.title, newDesc.authors,
                                      new Demo.Callback_Library_createBook()
            {
                @Override
                public void exception(Ice.LocalException ex)
                {
                    postError(ex.toString());
                }

                @Override
                public void exception(Ice.UserException ex)
                {
                    if(ex instanceof Demo.BookExistsException)
                    {
                        postError("That ISBN is already in the library.");
                    }
                    else
                    {
                        postError("Unknown error: " + ex.toString());
                    }
                }

                @Override
                public void response(Demo.BookPrx ret)
                {
                    synchronized(QueryController.this)
                    {
                        _currentBook = NO_BOOK;
                        postDataChanged(true);
                    }
                }

            });

            return true;
        }

        final Demo.BookDescription desc = _books.get(_currentBook);

        final boolean saveTitle = !newDesc.title.equals(desc.title);
        final boolean saveAuthors = !newDesc.authors.equals(desc.authors);

        // If nothing changed we're done.
        if(!saveTitle && !saveAuthors)
        {
            return false;
        }

        Runnable r = new Runnable()
        {
            public void run()
            {
                try
                {
                    if(saveTitle)
                    {
                        desc.proxy.setTitle(newDesc.title);
                        desc.title = newDesc.title;
                    }
                    if(saveAuthors)
                    {
                        desc.proxy.setAuthors(newDesc.authors);
                        desc.authors = newDesc.authors;
                    }
                    postDataChanged(true);
                }
                catch(Ice.LocalException ex)
                {
                    postError(ex.toString());
                }
            }
        };
        new Thread(r).start();
        return true;
    }
}
