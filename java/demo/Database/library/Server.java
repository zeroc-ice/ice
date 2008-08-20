// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class LibraryServer extends Ice.Application
{
    static class LocatorI implements Ice.ServantLocator
    {
        LocatorI(Ice.Logger logger, ConnectionPool pool, Ice.Object bookServant, Ice.Object libraryServant)
        {
            _logger = logger;
            _pool = pool;
            _bookServant = bookServant;
            _libraryServant = libraryServant;
        }

        public Ice.Object
        locate(Ice.Current c, Ice.LocalObjectHolder cookie)
        {
            assert c.id.category.equals("library") || c.id.category.equals("book");

            // Setup the new SQL request context.
            SQLRequestContext context = new SQLRequestContext(_logger, _pool);

            if(c.id.category.equals("library"))
            {
                return _libraryServant;
            }
            else //if(c.id.category.equals("book"))
            {
                return _bookServant;
            }
        }

        public void
        finished(Ice.Current c, Ice.Object servant, Object cookie)
        {
            // If a SQL request context is still associated with this
            // request, then destroy it (it will not be associated if
            // obtain was called).
            SQLRequestContext context = SQLRequestContext.getCurrentContext();
            if(context != null)
            {
                context.destroyFromLocator();
            }
        }

        public void
        deactivate(String category)
        {
        }

        private Ice.Logger _logger;
        private ConnectionPool _pool;

        private Ice.Object _bookServant;
        private Ice.Object _libraryServant;
    }

    public int
    run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        Ice.Properties properties = communicator().getProperties();

        String username = properties.getProperty("JDBC.Username");
        String password = properties.getProperty("JDBC.Password");
        String url = properties.getProperty("JDBC.Url");
        int nConnections = properties.getPropertyAsIntWithDefault("JDBC.NumConnections", 5);
        if(nConnections < 1)
        {
            nConnections = 1;
        }
        ConnectionPool pool = null;
        Ice.Logger logger = communicator().getLogger();

        try
        {
            Class.forName ("com.mysql.jdbc.Driver").newInstance();
        }
        catch(Exception e)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            e.printStackTrace(pw);
            pw.flush();
            System.err.println("failed to initialize mysql driver:\n" + sw.toString());
            return 1;
        }

        try
        {
            pool = new ConnectionPool(logger, url, username, password, nConnections);
        }
        catch(java.sql.SQLException e)
        {
            java.io.StringWriter sw = new java.io.StringWriter();
            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
            e.printStackTrace(pw);
            pw.flush();
            System.err.println("failed to create connection pool: SQLException:\n" + sw.toString());
            return 1;
        }

        ReapThread reaper = new ReapThread(logger);
        reaper.start();

        //
        // Create an object adapter
        //
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("SessionFactory");

        LocatorI locator = new LocatorI(logger, pool, new BookI(logger), new LibraryI(logger));

        adapter.add(new SessionFactoryI(logger, reaper), communicator().stringToIdentity("SessionFactory"));
        adapter.add(new Glacier2SessionManagerI(logger, reaper),
                    communicator().stringToIdentity("LibrarySessionManager"));

        adapter.addServantLocator(locator, "book");
        adapter.addServantLocator(locator, "library");

        //
        // Everything ok, let's go.
        //
        adapter.activate();

        shutdownOnInterrupt();
        communicator().waitForShutdown();
        defaultInterrupt();

        reaper.terminate();
        try
        {
            reaper.join();
        }
        catch(InterruptedException e)
        {
        }

        pool.destroy();

        return 0;
    }
}

public class Server
{
    static public void
    main(String[] args)
    {
        LibraryServer app = new LibraryServer();
        app.main("demo.Database.library.Server", args, "config.server");
    }
}
