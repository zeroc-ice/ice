// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

//
// This servant is a default servant. The book identity is retrieved
// from the Ice.Current object.
//
class BookI extends _BookDisp
{
    public void
    ice_ping(Ice.Current current)
    {
        SQLRequestContext context = SQLRequestContext.getCurrentContext();
        assert context != null;
        Integer id = new Integer(current.id.name);

        try
        {
            java.sql.PreparedStatement stmt = context.prepareStatement("SELECT * FROM books WHERE id = ?");
            stmt.setInt(1, id);
            java.sql.ResultSet rs = stmt.executeQuery();
            if(!rs.next())
            {
                throw new Ice.ObjectNotExistException();
            }
        }
        catch(java.sql.SQLException e)
        {
            JDBCException ex = new JDBCException();
            ex.initCause(e);
            throw ex;
        }
    }

    public BookDescription
    describe(Ice.Current current)
    {
        SQLRequestContext context = SQLRequestContext.getCurrentContext();
        assert context != null;
        Integer id = new Integer(current.id.name);

        try
        {
            java.sql.PreparedStatement stmt = context.prepareStatement("SELECT * FROM books WHERE id = ?");
            stmt.setInt(1, id);
            java.sql.ResultSet rs = stmt.executeQuery();
            if(!rs.next())
            {
                throw new Ice.ObjectNotExistException();
            }
            return extractDescription(context, rs, current.adapter);
        }
        catch(java.sql.SQLException e)
        {
            JDBCException ex = new JDBCException();
            ex.initCause(e);
            throw ex;
        }
    }

    public void
    setTitle(String title, Ice.Current current)
    {
        SQLRequestContext context = SQLRequestContext.getCurrentContext();
        assert context != null;
        Integer id = new Integer(current.id.name);

        try
        {
            java.sql.PreparedStatement stmt = context.prepareStatement("UPDATE books SET title = ? WHERE id = ?");
            stmt.setString(1, title);
            stmt.setInt(2, id);
            int count = stmt.executeUpdate();
            assert count == 1;
        }
        catch(java.sql.SQLException e)
        {
            JDBCException ex = new JDBCException();
            ex.initCause(e);
            throw ex;
        }
    }

    public void
    setAuthors(java.util.List<String> authors, Ice.Current current)
    {
        SQLRequestContext context = SQLRequestContext.getCurrentContext();
        assert context != null;
        Integer id = new Integer(current.id.name);

        try
        {
            // First destroy each of the authors_books records.
            java.sql.PreparedStatement stmt = context.prepareStatement("DELETE FROM authors_books WHERE book_id = ?");
            stmt.setInt(1, id);
            stmt.executeUpdate();

            //
            // Convert the authors string to an id set.
            //
            java.util.List<Integer> authIds = new java.util.LinkedList<Integer>();
            for(String author : authors)
            {
                Integer authid;
                stmt = context.prepareStatement("SELECT * FROM authors WHERE name = ?");
                stmt.setString(1, author);
                java.sql.ResultSet rs = stmt.executeQuery();
                if(rs.next())
                {
                    // If there is a result, then the database
                    // already contains this author.
                    authid = rs.getInt(1);
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
                    authid = rs.getInt(1);
                }

                // Add the new id to the list of ids.
                authIds.add(authid);
            }

            // Create new authors_books records.
            for(Integer i : authIds)
            {
                stmt = context.prepareStatement("INSERT INTO authors_books (book_id, author_id) VALUES(?, ?)");
                stmt.setInt(1, id);
                stmt.setInt(2, i);
                int count = stmt.executeUpdate();
                assert count == 1;
            }
        }
        catch(java.sql.SQLException e)
        {
            JDBCException ex = new JDBCException();
            ex.initCause(e);
            throw ex;
        }
    }

    public void
    destroy(Ice.Current current)
    {
        SQLRequestContext context = SQLRequestContext.getCurrentContext();
        assert context != null;
        Integer id = new Integer(current.id.name);

        try
        {
            java.sql.PreparedStatement stmt = context.prepareStatement("DELETE FROM books WHERE id = ?");
            stmt.setInt(1, id);
            int count = stmt.executeUpdate();
            if(count == 0)
            {
                throw new Ice.ObjectNotExistException();
            }
        }
        catch(java.sql.SQLException e)
        {
            JDBCException ex = new JDBCException();
            ex.initCause(e);
            throw ex;
        }
    }

    public String
    getRenter(Ice.Current current)
        throws BookNotRentedException
    {
        SQLRequestContext context = SQLRequestContext.getCurrentContext();
        assert context != null;
        Integer id = new Integer(current.id.name);

        try
        {
            java.sql.PreparedStatement stmt = context.prepareStatement("SELECT * FROM books WHERE id = ?");
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

            stmt = context.prepareStatement("SELECT * FROM customers WHERE id = ?");
            stmt.setInt(1, renterId);
            rs = stmt.executeQuery();
            boolean next = rs.next();
            assert next;
            return rs.getString("name");
        }
        catch(java.sql.SQLException e)
        {
            JDBCException ex = new JDBCException();
            ex.initCause(e);
            throw ex;
        }
    }

    public void
    rentBook(String name, Ice.Current current)
        throws InvalidCustomerException, BookRentedException
    {
        SQLRequestContext context = SQLRequestContext.getCurrentContext();
        assert context != null;
        Integer id = new Integer(current.id.name);
        name = name.trim();
        if(name.length() == 0)
        {
            throw new InvalidCustomerException();
        }

        try
        {
            java.sql.PreparedStatement stmt = context.prepareStatement("SELECT * FROM books WHERE id = ?");
            stmt.setInt(1, id);
            java.sql.ResultSet rs = stmt.executeQuery();
            if(!rs.next())
            {
                throw new Ice.ObjectNotExistException();
            }

            Integer renterId = rs.getInt("renter_id");
            if(!rs.wasNull())
            {
                stmt = context.prepareStatement("SELECT * FROM customers WHERE id = ?");
                stmt.setInt(1, renterId);
                rs = stmt.executeQuery();
                boolean next = rs.next();
                assert next;
                throw new BookRentedException(rs.getString("name"));
            }

            stmt = context.prepareStatement("SELECT * FROM customers WHERE name = ?");
            stmt.setString(1, name);
            rs = stmt.executeQuery();

            if(rs.next())
            {
                renterId = rs.getInt("id");
                assert !rs.next();
            }
            else
            {
                stmt = context.prepareStatement("INSERT into customers (name) VALUES(?)",
                                                java.sql.Statement.RETURN_GENERATED_KEYS);
                stmt.setString(1, name);
                int count = stmt.executeUpdate();
                assert count == 1;
                rs = stmt.getGeneratedKeys();
                boolean next = rs.next();
                assert next;
                renterId = rs.getInt(1);
            }

            stmt = context.prepareStatement("UPDATE books SET renter_id = ? WHERE id = ?");
            stmt.setInt(1, renterId);
            stmt.setInt(2, id);
            int count = stmt.executeUpdate();
            assert count == 1;
        }
        catch(java.sql.SQLException e)
        {
            JDBCException ex = new JDBCException();
            ex.initCause(e);
            throw ex;
        }
    }

    public void
    returnBook(Ice.Current current)
        throws BookNotRentedException
    {
        SQLRequestContext context = SQLRequestContext.getCurrentContext();
        assert context != null;
        Integer id = new Integer(current.id.name);
        try
        {
            java.sql.PreparedStatement stmt = context.prepareStatement("SELECT * FROM books WHERE id = ?");
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

            stmt = context.prepareStatement("UPDATE books SET renter_id = NULL WHERE id = ?");
            stmt.setInt(1, id);
            int count = stmt.executeUpdate();
            assert count == 1;
        }
        catch(java.sql.SQLException e)
        {
            JDBCException ex = new JDBCException();
            ex.initCause(e);
            throw ex;
        }
    }

    BookI()
    {
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
    extractDescription(SQLRequestContext context, java.sql.ResultSet rs, Ice.ObjectAdapter adapter)
        throws java.sql.SQLException
    {
        Integer id = rs.getInt("id");

        BookDescription desc = new BookDescription();
        desc.isbn = rs.getString("isbn");
        desc.title = rs.getString("title");
        desc.authors = new java.util.LinkedList<String>();
        desc.proxy = BookPrxHelper.uncheckedCast(adapter.createProxy(createIdentity(id)));

        java.sql.PreparedStatement stmt = null;
        // Query for the rentedBy.
        Integer renterId = rs.getInt("renter_id");
        if(!rs.wasNull())
        {
            stmt = context.prepareStatement("SELECT * FROM customers WHERE id = ?");
            stmt.setInt(1, renterId);
            java.sql.ResultSet customerRS = stmt.executeQuery();
            boolean next = customerRS.next();
            assert next;
            desc.rentedBy = customerRS.getString(2);
        }

        // Query for the authors.
        stmt = context.prepareStatement("SELECT * FROM authors INNER JOIN authors_books ON " +
                                        "authors.id=authors_books.author_id AND authors_books.book_id = ?");
        stmt.setInt(1, id);
        java.sql.ResultSet authorRS = stmt.executeQuery();
        while(authorRS.next())
        {
            desc.authors.add(authorRS.getString("name"));
        }

        return desc;
    }
}
