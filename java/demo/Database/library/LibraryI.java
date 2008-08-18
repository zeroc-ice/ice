// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

// This servant is a default servant. The book identity is retreived
// from Ice.Current object. The session object associated with the
// library is also retrieved using the Libraries identity.
class LibraryI extends _LibraryDisp
{
    public void
    queryByIsbn(String isbn, BookDescriptionHolder first, BookQueryResultPrxHolder result, Ice.Current current)
        throws NoResultsException
    {
        SessionI session = SessionI.getSession(current.id);
        if(session == null)
        {
            // No associated session.
            throw new Ice.ObjectNotExistException();
        }
        session.reapQueries();

        RequestContext context = RequestContext.getCurrentContext();
        assert context != null;

        try
        {
            java.sql.PreparedStatement stmt = context.prepareStatement("SELECT * FROM books WHERE isbn LIKE ?");
            stmt.setString(1, "%" + isbn + "%");
            java.sql.ResultSet rs = stmt.executeQuery();
            if(!rs.next())
            {
                throw new NoResultsException();
            }

            first.value = BookI.extractDescription(context, rs, current.adapter);
            if(rs.next())
            {
                // The RequestContext is now owned by the query
                // implementation.
                context.obtain();
                BookQueryResultI impl = new BookQueryResultI(_logger, context, rs);
                result.value = BookQueryResultPrxHelper.uncheckedCast(current.adapter.addWithUUID(impl));
                session.add(result.value, impl);
            }
        }
        catch(java.sql.SQLException e)
        {
            // Log the error, and raise an UnknownException.
            error(e);
            Ice.UnknownException ex = new Ice.UnknownException();
            ex.initCause(e);
            throw ex;
        }
    }

    public void
    queryByAuthor(String author, BookDescriptionHolder first, BookQueryResultPrxHolder result, Ice.Current current)
        throws NoResultsException
    {
        SessionI session = SessionI.getSession(current.id);
        if(session == null)
        {
            // No associated session.
            throw new Ice.ObjectNotExistException();
        }
        session.reapQueries();

        RequestContext context = RequestContext.getCurrentContext();
        assert context != null;

        try
        {
                // Find each of the authors.
            java.sql.PreparedStatement stmt = context.prepareStatement("SELECT * FROM authors WHERE name LIKE ?");
            stmt.setString(1, "%" + author + "%");
            java.sql.ResultSet rs = stmt.executeQuery();
            if(!rs.next())
            {
                throw new NoResultsException();
            }

            // Build a query that finds all books by these set of
            // authors.
            StringBuffer sb = new StringBuffer("SELECT * FROM books INNER JOIN authors_books ON " +
                                               "books.id=authors_books.book_id AND (");
            boolean front = true;
            do
            {
                if(!front)
                {
                    sb.append(" OR ");
                }
                front = false;
                sb.append("authors_books.author_id=");
                sb.append(rs.getInt("id"));
            }
            while(rs.next());
            sb.append(")");

            // Execute the query.
            stmt = context.prepareStatement(sb.toString());
            rs = stmt.executeQuery();
            if(!rs.next())
            {
                throw new NoResultsException();
            }

            first.value = BookI.extractDescription(context, rs, current.adapter);
            if(rs.next())
            {
                // The RequestContext is now owned by the query
                // implementation.
                context.obtain();
                BookQueryResultI impl = new BookQueryResultI(_logger, context, rs);
                result.value = BookQueryResultPrxHelper.uncheckedCast(current.adapter.addWithUUID(impl));
                session.add(result.value, impl);
            }
        }
        catch(java.sql.SQLException e)
        {
            // Log the error, and raise an UnknownException.
            error(e);
            Ice.UnknownException ex = new Ice.UnknownException();
            ex.initCause(e);
            throw ex;
        }
    }

    public BookPrx
    createBook(String isbn, String title, java.util.List<String> authors, Ice.Current current)
        throws BookExistsException
    {
        RequestContext context = RequestContext.getCurrentContext();
        assert context != null;
        try
        {
            java.sql.PreparedStatement stmt = context.prepareStatement("SELECT * FROM books WHERE isbn = ?");
            stmt.setString(1, isbn);
            java.sql.ResultSet rs = stmt.executeQuery();
            if(rs.next())
            {
                throw new BookExistsException();
            }

            //
            // First convert the authors string to an id set.
            //
            java.util.List<Integer> authIds = new java.util.LinkedList<Integer>();
            java.util.Iterator<String> p = authors.iterator();
            while(p.hasNext())
            {
                String author = p.next();

                Integer id;
                stmt = context.prepareStatement("SELECT * FROM authors WHERE name = ?");
                stmt.setString(1, author);
                rs = stmt.executeQuery();
                if(rs.next())
                {
                    // If there is a result, then the database
                    // already contains this author.
                    id = rs.getInt(1);
                    assert !rs.next();
                }
                else
                {
                    // Otherwise, create a new author record.
                    stmt = context.prepareStatement("INSERT INTO authors (name) VALUES(?)",
                                                 java.sql.Statement.RETURN_GENERATED_KEYS);
                    stmt.setString(1, author);
                    int count = stmt.executeUpdate();
                    assert count == 1;
                    rs = stmt.getGeneratedKeys();
                    boolean next = rs.next();
                    assert next;
                    id = rs.getInt(1);
                }

                // Add the new id to the list of ids.
                authIds.add(id);
            }

            // Create the new book.
            stmt = context.prepareStatement("INSERT INTO books (isbn, title) VALUES(?, ?)",
                                         java.sql.Statement.RETURN_GENERATED_KEYS);
            stmt.setString(1, isbn);
            stmt.setString(2, title);
            int count = stmt.executeUpdate();
            assert count == 1;

            rs = stmt.getGeneratedKeys();
            boolean next = rs.next();
            assert next;
            Integer bookId = rs.getInt(1);

            // Create new authors_books records.
            java.util.Iterator<Integer> q = authIds.iterator();
            while(q.hasNext())
            {
                stmt = context.prepareStatement("INSERT INTO authors_books (book_id, author_id) VALUES(?, ?)");
                stmt.setInt(1, bookId);
                stmt.setInt(2, q.next());
                count = stmt.executeUpdate();
                assert count == 1;
            }

            // Commit the transaction.
            context.commit();

            return BookPrxHelper.uncheckedCast(current.adapter.createProxy(BookI.createIdentity(bookId)));
        }
        catch(java.sql.SQLException e)
        {
            // Log the error, and raise an UnknownException.
            error(e);
            Ice.UnknownException ex = new Ice.UnknownException();
            ex.initCause(e);
            throw ex;
        }
    }

    LibraryI(Ice.Logger logger)
    {
        _logger = logger;
    }

    private void
    error(Exception ex)
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        ex.printStackTrace(pw);
        pw.flush();
        _logger.error("LibraryI: error:\n" + sw.toString());
    }

    private Ice.Logger _logger;
}
