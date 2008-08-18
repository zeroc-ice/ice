// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class BookQueryResultI extends _BookQueryResultDisp
{
    // The query result owns the java.sql.Connection object until
    // destroyed.
    BookQueryResultI(Ice.Logger logger, SQLRequestContext context, java.sql.ResultSet rs)
    {
        _logger = logger;
        _context = context;
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
                l.add(BookI.extractDescription(_context, _rs, current.adapter));
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
        _context.destroy();

        current.adapter.remove(current.id);
    }

    // Called on application shutdown by the Library.
    synchronized public void
    shutdown()
    {
        if(!_destroyed)
        {
            _destroyed = true;
            _context.destroy();
        }
    }

    private void
    error(Exception ex)
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        ex.printStackTrace(pw);
        pw.flush();
        _logger.error("BookQueryResultI: error:\n" + sw.toString());
    }

    private Ice.Logger _logger;
    private SQLRequestContext _context;
    private java.sql.ResultSet _rs;
    private boolean _destroyed = false;
}

