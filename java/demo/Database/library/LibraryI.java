// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

// Per-session library object.
class LibraryI extends _LibraryDisp
{
    class BookQueryResultI extends _BookQueryResultDisp
    {
        // The query result owns the java.sql.Connection object until
        // destroyed.
        BookQueryResultI(java.sql.Connection conn, java.sql.PreparedStatement stmt, java.sql.ResultSet rs)
        {
            _conn = conn;
            _stmt = stmt;
            _rs = rs;
        }

        synchronized public java.util.List<BookDescription>
        next(int n, Ice.BooleanHolder destroyed, Ice.Current current)
        {
            if(_destroyed)
            {
                throw new Ice.ObjectNotExistException();
            }
            destroyed.value = false;
            java.util.List<BookDescription> l = new java.util.LinkedList<BookDescription>();
            if(n <= 0)
            {
                return l;
            }
            boolean next = true;
            try
            {
                for(int i = 0; i < n && next; ++i)
                {
                    l.add(BookI.extractDescription(_conn, _rs, current.adapter));
                    next = _rs.next();
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

            if(!next)
            {
                try
                {
                    destroyed.value = true;
                    destroy(current);
                }
                catch(Exception e)
                {
                    // Ignore.
                }
            }

            return l;
        }

        synchronized public void
        destroy(Ice.Current current)
        {
            if(_destroyed)
            {
                throw new Ice.ObjectNotExistException();
            }
            _destroyed = true;

            try
            {
                // Closing a statement closes the associated
                // java.sql.ResultSet.
                _stmt.close();
            }
            catch(java.sql.SQLException e)
            {
                // Log the error, but otherwise ignore the exception.
                error(e);
            }
            _pool.release(_conn);

            current.adapter.remove(current.id);
        }

        // Called on application shutdown by the Library.
        synchronized private void
        shutdown()
        {
            if(!_destroyed)
            {
                _destroyed = true;

                try
                {
                    // Closing a statement closes the associated
                    // java.sql.ResultSet.
                    _stmt.close();
                }
                catch(java.sql.SQLException e)
                {
                    // Log the error, but otherwise ignore the
                    // exception.
                    error(e);
                }
                _pool.release(_conn);
            }
        }

        private java.sql.Connection _conn;
        private java.sql.PreparedStatement _stmt;
        private java.sql.ResultSet _rs;
        private boolean _destroyed = false;
    }

    synchronized public void
    queryByIsbn(String isbn, BookDescriptionHolder first, BookQueryResultPrxHolder result, Ice.Current current)
        throws QueryActiveException, NoResultsException
    {
        reapQuery();

        java.sql.Connection conn = _pool.acquire();
        try
        {
            java.sql.PreparedStatement stmt = null;
            try
            {
                stmt = conn.prepareStatement("SELECT * FROM books WHERE isbn LIKE ?");
                stmt.setString(1, "%" + isbn + "%");
                java.sql.ResultSet rs = stmt.executeQuery();
                if(!rs.next())
                {
                    throw new NoResultsException();
                }

                first.value = BookI.extractDescription(conn, rs, current.adapter);
                if(rs.next())
                {
                    _queryImpl = new BookQueryResultI(conn, stmt, rs);
                    result.value = BookQueryResultPrxHelper.uncheckedCast(current.adapter.addWithUUID(_queryImpl));
                    _query = result.value;
                    // The java.sql.Connection, result set and
                    // statement are now owned by the book query. Set
                    // to null so they are not prematurely released.
                    conn = null;
                    stmt = null;
                }
            }
            finally
            {
                if(stmt != null)
                {
                    // Closing a statement closes the associated
                    // java.sql.ResultSet.
                    stmt.close();
                }
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
        finally
        {
            _pool.release(conn);
        }
    }

    synchronized public void
    queryByAuthor(String author, BookDescriptionHolder first, BookQueryResultPrxHolder result, Ice.Current current)
        throws QueryActiveException, NoResultsException
    {
        reapQuery();

        java.sql.Connection conn = _pool.acquire();
        try
        {
            java.sql.PreparedStatement stmt = null;
            try
            {
                // Find each of the authors.
                stmt = conn.prepareStatement("SELECT * FROM authors WHERE name LIKE ?");
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

                stmt.close();
                stmt = null;

                // Execute the query.
                stmt = conn.prepareStatement(sb.toString());
                rs = stmt.executeQuery();
                if(!rs.next())
                {
                    throw new NoResultsException();
                }

                first.value = BookI.extractDescription(conn, rs, current.adapter);
                if(rs.next())
                {
                    _queryImpl = new BookQueryResultI(conn, stmt, rs);
                    result.value = BookQueryResultPrxHelper.uncheckedCast(current.adapter.addWithUUID(_queryImpl));
                    _query = result.value;
                    // The java.sql.Connection, result set and
                    // statement are now owned by the book query. Set
                    // to null so they are not prematurely released.
                    stmt = null;
                    conn = null;
                }
            }
            finally
            {
                // Closing a statement closes the associated
                // java.sql.ResultSet.
                if(stmt != null)
                {
                    stmt.close();
                }
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
        finally
        {
            _pool.release(conn);
        }
    }

    synchronized public BookPrx
    createBook(String isbn, String title, java.util.List<String> authors, Ice.Current current)
        throws BookExistsException
    {
        java.sql.Connection conn = _pool.acquire();
        try
        {
            conn.setAutoCommit(false);
            java.sql.PreparedStatement stmt = null;
            try
            {
                stmt = conn.prepareStatement("SELECT * FROM books WHERE isbn = ?");
                stmt.setString(1, isbn);
                java.sql.ResultSet rs = stmt.executeQuery();
                if(rs.next())
                {
                    throw new BookExistsException();
                }

                stmt.close();
                stmt = null;

                //
                // First convert the authors string to an id set.
                //
                java.util.List<Integer> authIds = new java.util.LinkedList<Integer>();
                java.util.Iterator<String> p = authors.iterator();
                while(p.hasNext())
                {
                    String author = p.next();

                    Integer id;
                    stmt = conn.prepareStatement("SELECT * FROM authors WHERE name = ?");
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
                        stmt.close();
                        stmt = null;

                        // Otherwise, create a new author record.
                        stmt = conn.prepareStatement("INSERT INTO authors (name) VALUES(?)",
                                                     java.sql.Statement.RETURN_GENERATED_KEYS);
                        stmt.setString(1, author);
                        int count = stmt.executeUpdate();
                        assert count == 1;
                        rs = stmt.getGeneratedKeys();
                        boolean next = rs.next();
                        assert next;
                        id = rs.getInt(1);
                    }

                    stmt.close();
                    stmt = null;

                    // Add the new id to the list of ids.
                    authIds.add(id);
                }

                // Create the new book.
                stmt = conn.prepareStatement("INSERT INTO books (isbn, title) VALUES(?, ?)",
                                             java.sql.Statement.RETURN_GENERATED_KEYS);
                stmt.setString(1, isbn);
                stmt.setString(2, title);
                int count = stmt.executeUpdate();
                assert count == 1;

                rs = stmt.getGeneratedKeys();
                boolean next = rs.next();
                assert next;
                Integer bookId = rs.getInt(1);

                stmt.close();
                stmt = null;

                // Create new authors_books records.
                java.util.Iterator<Integer> q = authIds.iterator();
                while(q.hasNext())
                {
                    stmt = conn.prepareStatement("INSERT INTO authors_books (book_id, author_id) VALUES(?, ?)");
                    stmt.setInt(1, bookId);
                    stmt.setInt(2, q.next());
                    count = stmt.executeUpdate();
                    assert count == 1;

                    stmt.close();
                    stmt = null;
                }

                // Commit the transaction.
                conn.commit();
                
                return BookPrxHelper.uncheckedCast(current.adapter.createProxy(BookI.createIdentity(bookId)));
            }
            catch(RuntimeException e)
            {
                // Rollback any updates.
                conn.rollback();
                throw e;
            }
            catch(java.sql.SQLException e)
            {
                // Rollback any updates.
                conn.rollback();
                throw e;
            }
            finally
            {
                if(stmt != null)
                {
                    stmt.close();
                }
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
        finally
        {
            try
            {
                conn.setAutoCommit(true);
            }
            catch(java.sql.SQLException e)
            {
                // Ignore
            }
            _pool.release(conn);
        }
    }

    LibraryI(Ice.Logger logger, ConnectionPool pool)
    {
        _logger = logger;
        _pool = pool;
    }

    // Called when the session is destroyed.
    synchronized void
    destroy()
    {
        if(_query != null)
        {
            try
            {
                _query.destroy();
            }
            catch(Ice.ObjectNotExistException e)
            {
            }
            _query = null;
            _queryImpl = null;
        }
    }

    // Called on application shutdown.
    synchronized void
    shutdown()
    {
        if(_queryImpl != null)
        {
            _queryImpl.shutdown();
            _queryImpl = null;
            _query = null;
        }
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

    private void
    reapQuery()
        throws QueryActiveException
    {
        if(_query != null)
        {
            try
            {
                _query.ice_ping();
            }
            catch(Ice.ObjectNotExistException e)
            {
                _query = null;
                _queryImpl = null;
            }

            if(_query != null)
            {
                throw new QueryActiveException();
            }
        }
    }

    private Ice.Logger _logger;
    private ConnectionPool _pool;
    private BookQueryResultPrx _query = null;
    private BookQueryResultI _queryImpl = null;
}
