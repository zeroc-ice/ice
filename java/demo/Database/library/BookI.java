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
// from Ice.Current object.
class BookI extends _BookDisp
{
    public void
    ice_ping(Ice.Current current)
    {
        Integer id = new Integer(current.id.name);

        java.sql.Connection conn = _pool.acquire();
        try
        {
            java.sql.PreparedStatement stmt = null;
            try
            {
                stmt = conn.prepareStatement("SELECT * FROM books WHERE id = ?");
                stmt.setInt(1, id);
                java.sql.ResultSet rs = stmt.executeQuery();
                if(!rs.next())
                {
                    throw new Ice.ObjectNotExistException();
                }
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
            _pool.release(conn);
        }
    }

    public BookDescription
    describe(Ice.Current current)
    {
        Integer id = new Integer(current.id.name);

        java.sql.Connection conn = _pool.acquire();
        try
        {
            java.sql.PreparedStatement stmt = null;
            try
            {
                stmt = conn.prepareStatement("SELECT * FROM books WHERE id = ?");
                stmt.setInt(1, id);
                java.sql.ResultSet rs = stmt.executeQuery();
                if(!rs.next())
                {
                    throw new Ice.ObjectNotExistException();
                }
                return extractDescription(conn, rs, current.adapter);
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
            _pool.release(conn);
        }
    }

    public void
    destroy(Ice.Current current)
    {
        Integer id = new Integer(current.id.name);
        java.sql.Connection conn = _pool.acquire();
        try
        {
            java.sql.PreparedStatement stmt = null;
            try
            {
                stmt = conn.prepareStatement("DELETE FROM books WHERE id = ?");
                stmt.setInt(1, id);
                int count = stmt.executeUpdate();
                if(count == 0)
                {
                    throw new Ice.ObjectNotExistException();
                }
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
            _pool.release(conn);
        }
    }

    public String
    getRenter(Ice.Current current)
        throws BookNotRentedException
    {
        Integer id = new Integer(current.id.name);
        java.sql.Connection conn = _pool.acquire();
        try
        {
            java.sql.PreparedStatement stmt = null;
            try
            {
                stmt = conn.prepareStatement("SELECT * FROM books WHERE id = ?");
                stmt.setInt(1, id);
                java.sql.ResultSet rs = stmt.executeQuery();
                if(!rs.next())
                {
                    throw new Ice.ObjectNotExistException();
                }

                int renterId = rs.getInt("renter_id");
                if(rs.wasNull())
                {
                    throw new BookNotRentedException();
                }
                stmt.close();
                stmt = null;
                
                stmt = conn.prepareStatement("SELECT * FROM customers WHERE id = ?");
                stmt.setInt(1, renterId);
                rs = stmt.executeQuery();
                boolean next = rs.next();
                assert next;
                return rs.getString("name");
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
            _pool.release(conn);
        }
    }

    public void
    rentBook(String name, Ice.Current current)
        throws BookRentedException
    {
        java.sql.Connection conn = _pool.acquire();
        Integer id = new Integer(current.id.name);
        try
        {
            conn.setAutoCommit(false);
            java.sql.PreparedStatement stmt = null;
            try
            {
                stmt = conn.prepareStatement("SELECT * FROM books WHERE id = ?");
                stmt.setInt(1, id);
                java.sql.ResultSet rs = stmt.executeQuery();
                if(!rs.next())
                {
                    throw new Ice.ObjectNotExistException();
                }

                rs.getInt("renter_id");
                if(!rs.wasNull())
                {
                    throw new BookRentedException();
                }
                stmt.close();
                stmt = null;

                stmt = conn.prepareStatement("SELECT * FROM customers WHERE name = ?");
                stmt.setString(1, name);
                rs = stmt.executeQuery();

                Integer renterId = null;
                if(rs.next())
                {
                    renterId = rs.getInt("id");
                    assert !rs.next();
                }
                else
                {
                    stmt.close();
                    stmt = null;

                    stmt = conn.prepareStatement("INSERT into customers (name) VALUES(?)",
                                                        java.sql.Statement.RETURN_GENERATED_KEYS);
                    stmt.setString(1, name);
                    int count = stmt.executeUpdate();
                    assert count == 1;
                    rs = stmt.getGeneratedKeys();
                    if(!rs.next())
                    {
                        // ERROR:
                    }
                    renterId = rs.getInt(1);
                }
                stmt.close();
                stmt = null;

                stmt = conn.prepareStatement("UPDATE books SET renter_id = ? WHERE id = ?");
                stmt.setInt(1, renterId);
                stmt.setInt(2, id);
                int count = stmt.executeUpdate();
                assert count == 1;

                // Commit the transaction.
                conn.commit();
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

    public void
    returnBook(Ice.Current current)
        throws BookNotRentedException
    {
        Integer id = new Integer(current.id.name);
        java.sql.Connection conn = _pool.acquire();
        try
        {
            java.sql.PreparedStatement stmt = null;
            try
            {
                stmt = conn.prepareStatement("SELECT * FROM books WHERE id = ?");
                stmt.setInt(1, id);
                java.sql.ResultSet rs = stmt.executeQuery();
                if(!rs.next())
                {
                    throw new Ice.ObjectNotExistException();
                }
                Integer renterId = rs.getInt("renter_id");
                if(rs.wasNull())
                {
                    throw new BookNotRentedException();
                }
                stmt.close();
                stmt = null;

                stmt = conn.prepareStatement("UPDATE books SET renter_id = NULL WHERE id = ?");
                stmt.setInt(1, id);
                int count = stmt.executeUpdate();
                assert count == 1;
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
            _pool.release(conn);
        }
    }

    BookI(Ice.Logger logger, ConnectionPool pool)
    {
        _logger = logger;
        _pool = pool;
    }

    static Ice.Identity
    createIdentity(Integer bookId)
    {
        Ice.Identity id = new Ice.Identity();
        id.category = "book";
        id.name = bookId.toString();
        return id;
    }

    static BookDescription
    extractDescription(java.sql.Connection conn, java.sql.ResultSet rs, Ice.ObjectAdapter adapter)
        throws java.sql.SQLException
    {
        Integer id = rs.getInt("id");

        BookDescription desc = new BookDescription();
        desc.isbn = rs.getString("isbn");
        desc.title = rs.getString("title");
        desc.authors = new java.util.LinkedList<String>();
        desc.proxy = BookPrxHelper.uncheckedCast(adapter.createProxy(createIdentity(id)));

        java.sql.PreparedStatement stmt = null;
        try
        {
            // Query for the rentedBy.
            Integer renterId = rs.getInt("renter_id");
            if(!rs.wasNull())
            {
                stmt = conn.prepareStatement("SELECT * FROM customers WHERE id = ?");
                stmt.setInt(1, renterId);
                java.sql.ResultSet customerRS = stmt.executeQuery();
                boolean next = customerRS.next();
                assert next;
                desc.rentedBy = customerRS.getString(2);

                stmt.close();
                stmt = null;
            }

            // Query for the authors.
            stmt = conn.prepareStatement("SELECT * FROM authors INNER JOIN authors_books ON " +
                                         "authors.id=authors_books.author_id AND authors_books.book_id = ?");
            stmt.setInt(1, id);
            java.sql.ResultSet authorRS = stmt.executeQuery();
            while(authorRS.next())
            {
                desc.authors.add(authorRS.getString("name"));
            }
        }
        finally
        {
            if(stmt != null)
            {
                stmt.close();
            }
        }

        return desc;
    }

    private void
    error(Exception ex)
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        ex.printStackTrace(pw);
        pw.flush();
        _logger.error("BookI: error:\n" + sw.toString());
    }

    private Ice.Logger _logger;
    private ConnectionPool _pool;
}
