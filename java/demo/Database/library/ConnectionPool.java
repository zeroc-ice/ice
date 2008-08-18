// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class ConnectionPool
{
    public
    ConnectionPool(Ice.Logger logger, String url, String username, String password, int numConnections)
        throws java.sql.SQLException
    {
        _logger = logger;
        _url = url;
        _username = username;
        _password = password;

        _nconnections = numConnections;
        while(numConnections-- > 0)
        {
            java.sql.Connection connection = java.sql.DriverManager.getConnection (url, username, password);
            connection.setAutoCommit(false);
            _connections.add(connection);
        }
    }

    public synchronized void
    destroy()
    {
        _destroyed = true;
        while(_connections.size() != _nconnections)
        {
            try
            {
                wait();
            }
            catch(InterruptedException e)
            {
            }
        }

        while(_connections.isEmpty())
        {
            java.sql.Connection conn = _connections.removeFirst();
            try
            {
                conn.close();
            }
            catch(java.sql.SQLException e)
            {
            }
        }
    }

    public synchronized java.sql.Connection
    acquire()
    {
        while(_connections.isEmpty() && !_destroyed)
        {
            try
            {
                wait();
            }
            catch(InterruptedException e)
            {
            }
        }
        if(_destroyed)
        {
            return null;
        }
        java.sql.Connection conn = _connections.removeFirst();

        try
        {
            boolean closed = conn.isClosed();
            if(closed)
            {
                _logger.warning("ConnectionPool: lost connection to database");
                conn = null;
            }
        }
        catch(java.sql.SQLException e)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            e.printStackTrace(pw);
            pw.flush();
            _logger.warning("ConnectionPool: lost connection to database:\n" + sw.toString());

            conn = null;
        }

        // If the connection has been closed, or is otherwise invalid
        // re-establish the connection.
        while(conn == null)
        {
            _logger.trace("ConnectionPool", "establishing new database connection");
            try
            {
                conn = java.sql.DriverManager.getConnection (_url, _username, _password);
                conn.setAutoCommit(false);
            }
            catch(java.sql.SQLException e)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                e.printStackTrace(pw);
                pw.flush();
                _logger.warning("ConnectionPool: database connection failed:\n" + sw.toString());
            }
        }
        return conn;
    }

    public synchronized void
    release(java.sql.Connection connection)
    {
        if(connection != null)
        {
            _connections.add(connection);
            notifyAll();
        }
    }

    Ice.Logger _logger;
    private String _url;
    private String _username;
    private String _password;
    private java.util.LinkedList<java.sql.Connection> _connections = new java.util.LinkedList<java.sql.Connection>();
    private boolean _destroyed = false;
    private int _nconnections;
}
