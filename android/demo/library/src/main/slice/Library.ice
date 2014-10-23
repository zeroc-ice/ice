// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Ice/BuiltinSequences.ice>

module Demo
{

/**
 *
 * This local exception is used internally if a java.sql.SQLException
 * is raised.
 *
 **/
local exception JDBCException
{
};

/**
 *
 * This exception is raised if the book already exists.
 *
 **/
exception BookExistsException
{
};

/**
 *
 * This exception is raised if a book has already been rented.
 *
 **/
exception BookRentedException
{
    string renter;
};

/**
 *
 * This exception is raised if a customer name is invalid.
 *
 **/
exception InvalidCustomerException
{
};

/**
 *
 * This exception is raised if the book has not been rented.
 *
 **/
exception BookNotRentedException
{
};

/** Forward declaration for the interface Book. */
interface Book;

/**
 *
 * A description of a book.
 *
 **/
struct BookDescription
{
    /** The ISBN number of the book.  */
    string isbn;

    /** The title of the book. */
    string title;

    /** The authors of the book. */
    ["java:type:java.util.LinkedList<String>:java.util.List<String>"] Ice::StringSeq authors;

    /** The customer name of the renter. */
    string rentedBy;

    /** A proxy to the associated book. */
    Book* proxy;
};

/** A sequence of book descriptions. */
["java:type:java.util.LinkedList<BookDescription>:java.util.List<BookDescription>"]
sequence<BookDescription> BookDescriptionSeq;

/**
 *
 * This interface represents a book.
 *
 **/
interface Book
{
    /**
     *
     * Get a description of the book.
     *
     * @return The book description.
     *
     **/
    idempotent BookDescription describe();

    /**
     *
     * Set the title of a book.
     *
     * @param title The book title.
     *
     **/
    void setTitle(string title);

    /**
     *
     * Set the book authors.
     *
     * @param authors The book authors.
     *
     **/
    void setAuthors(["java:type:java.util.LinkedList<String>:java.util.List<String>"] Ice::StringSeq authors);

    /**
     *
     * Rent the book to the specified customer.
     *
     * @param customer The customer.
     *
     * @throws BookRentedException Raised if the book has already been
     * rented.
     *
     * @throws InvalidCustomerException Raised if the customer is invalid.
     *
     **/
    void rentBook(string name)
        throws InvalidCustomerException, BookRentedException;

    /**
     *
     * Get the renter.
     *
     * @return The current rental customer.
     *
     * @throws BookNotRentedException Raised if the book is not
     * currently rented.
     *
     **/
    idempotent string getRenter()
        throws BookNotRentedException;

    /**
     *
     * Return the book.
     *
     * @throws BookNotRentedException Raised if the book is not
     * currently rented.
     *
     **/
    void returnBook()
        throws BookNotRentedException;

    /**
     *
     * Destroy the book.
     *
     **/
    void destroy();
};

/**
 *
 * Interface to get query results.
 *
 **/
interface BookQueryResult
{
    /**
     *
     * Get more query results.
     *
     * @param n The maximum number of results to return.
     *
     * @param destroyed There are no more results, and the query has
     * been destroyed.
     *
     * @returns A sequence of up to n results.
     *
     **/
    BookDescriptionSeq next(int n, out bool destroyed);

    /**
     *
     * Destroy the query result.
     *
     **/
    void destroy();
};

/**
 *
 * An interface to the library.
 *
 **/
interface Library
{
    /**
     *
     * Query based on isbn number. The query is a partial match at the
     * start of the isbn number.
     *
     * @param isbn The ISBN number.
     *
     * @param n The number of rows to retrieve in the initial request.

     * @param first The first set of results, up to n results.
     *
     * @param nrows The total number of rows.
     *
     * @param result The remainder of the results. If there are no
     * further results, a null proxy is returned.
     *
     **/
    void queryByIsbn(string isbn, int n, out BookDescriptionSeq first, out int nrows, out BookQueryResult* result);

    /**
     *
     * Query based on the author name. The query is a partial match of
     * the author's name.
     *
     * @param author The authors name.
     *
     * @param n The number of rows to retrieve in the initial request.

     * @param first The first set of results, up to n results.
     *
     * @param nrows The total number of rows.
     *
     * @param result The remainder of the results. If there are no
     * further results, a null proxy is returned.
     *
     **/
    void queryByAuthor(string author, int n, out BookDescriptionSeq first, out int nrows, out BookQueryResult* result);

    /**
     *
     * Query based on the book title. The query is a partial match of
     * the book title.
     *
     * @param author The authors name.
     *
     * @param n The number of rows to retrieve in the initial request.

     * @param first The first set of results, up to n results.
     *
     * @param nrows The total number of rows.
     *
     * @param result The remainder of the results. If there are no
     * further results, a null proxy is returned.
     *
     **/
    void queryByTitle(string title, int n, out BookDescriptionSeq first, out int nrows, out BookQueryResult* result);

    /**
     *
     * Create a book with the given description.
     *
     * @param description The book description.
     *
     * @return A proxy for the new book.
     *
     * @throws BookExistsException Raised if a book with the same ISBN
     * number already exists.
     *
     **/
    Book* createBook(string isbn, string title,
                     ["java:type:java.util.LinkedList<String>:java.util.List<String>"] Ice::StringSeq authors)
        throws BookExistsException;
};

};
