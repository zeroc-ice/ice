// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef LIBRARY_ICE
#define LIBRARY_ICE

module Demo
{

/**
 *
 * This exception is raised in the case of a database failure.
 *
 **/
exception DatabaseException
{
    string message;
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
    string authors;
};

/**
 *
 * This class represents a book.
 *
 **/
class Book
{
    /**
     *
     * Get a description of the book.
     *
     * @return The book description.
     *
     **/
    ["cpp:const"] idempotent BookDescription getBookDescription();

    /**
     *
     * Destroy the book.
     *
     * @throws DatabaseException Raised if there is a problem with the
     * database system.
     *
     **/
    ["freeze:write"] void destroy()
        throws DatabaseException;

    /**
     *
     * Rent the book to the specified customer.
     *
     * @param name The name of the customer.
     *
     * @throws BookRentedException Raised if the book has already been
     * rented.
     *
     **/
    ["freeze:write"] void rentBook(string name)
        throws BookRentedException;

    /**
     *
     * Get the name of the renter.
     *
     * @return The name the current rental customer.
     *
     * @throws BookNotRentedException Raised if the book is not
     * currently rented.
     *
     **/
    ["cpp:const"] idempotent string getRenterName()
        throws BookNotRentedException;

    /**
     *
     * Return the book.
     *
     * @throws BookNotRentedException Raised if the book is not
     * currently rented.
     *
     **/
    ["freeze:write"] void returnBook()
        throws BookNotRentedException;

    /**
     *
     * The description for this book. Once a book has been created
     * this information is immutable.
     *
     **/
    BookDescription description;

    /**
     *
     * The name of the current rental customer, or the empty string if
     * the book is not currently rented.
     *
     **/
    string rentalCustomerName;
};

/**
 *
 * A sequence of book proxies.
 *
 **/
sequence<Book*> BookPrxSeq;

/**
 *
 * This interface represents a library.
 *
 **/
interface Library
{
    /**
     *
     * Create a book with the given description.
     *
     * @param description The book description.
     *
     * @return A proxy for the new book.
     *
     * @throws DatabaseException Raised if there is a problem with the
     * database.
     *
     * @throws BookExistsException Raised if the book already exists.
     *
     **/
    Book* createBook(BookDescription description)
        throws DatabaseException, BookExistsException;

    /**
     *
     * Find a book by its ISBN number.
     *
     * @param isbn The ISBN number.
     *
     * @return The result of the search, or an null proxy if the book
     * was not found.
     *
     * @throws DatabaseException Raised if there is a problem with the database.
     *
     **/
    ["cpp:const"] idempotent Book* findByIsbn(string isbn)
        throws DatabaseException;

    /**
     *
     * Find all books with a given authors.
     *
     * @param title The search parameters.
     *
     * @return The result of the search, or an empty sequence if no
     * books were found.
     *
     * @throws DatabaseException Raised if there is a problem with the database.
     *
     **/
    ["cpp:const"] idempotent BookPrxSeq findByAuthors(string authors)
        throws DatabaseException;

    /**
     *
     * Set the evictor size.
     *
     * @param size The new size for the evictor.
     *
     * @throws DatabaseException Raised if there is a problem with the
     * database.
     *
     **/
    idempotent void setEvictorSize(int size)
        throws DatabaseException;

    /**
     *
     * Shutdown the server.
     *
     **/
    void shutdown();
};

};

#endif
