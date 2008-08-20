// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef LIBRARY_ICE
#define LIBRARY_ICE

#include <Ice/BuiltinSequences.ice>

module Demo
{

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
};

/**
 *
 * This exception is raised if the book has not been rented.
 *
 **/
exception BookNotRentedException
{
};

/**
 *
 * This exception is raised if a query has no results.
 *
 **/
exception NoResultsException
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
     * Rent the book to the specified customer.
     *
     * @param customer The customer.
     *
     * @throws BookRentedException Raised if the book has already been
     * rented.
     *
     **/
    void rentBook(string name)
        throws BookRentedException;

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
     * Query based on isbn number. The query is a partial match of the
     * isbn number.
     *
     * @param isbn The ISBN number.
     *
     * @param first The first book description.
     *
     * @param result The remainder of the results. If there are no
     * further results, a null proxy is returned.

     * @throws NoResultsException Raised if there are no results.
     *
     **/
    void queryByIsbn(string isbn, out BookDescription first, out BookQueryResult* result)
        throws NoResultsException;

    /**
     *
     * Query based on the author name. The query is a partial match of
     * the author's name.
     *
     * @param author The authors name.
     *
     * @param first The first book description.
     *
     * @param result The remainder of the results. If there are no
     * further results, a null proxy is returned.

     * @throws NoResultsException Raised if there are no results.
     *
     **/
    void queryByAuthor(string author, out BookDescription first, out BookQueryResult* result)
        throws NoResultsException;

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

#endif
