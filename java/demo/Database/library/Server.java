// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class Server extends Ice.Application
{
    static class LocatorI implements Ice.ServantLocator
    {
        public Ice.Object
        locate(Ice.Current c, Ice.LocalObjectHolder cookie)
        {
            assert c.id.category.equals("book");
            return _servant;
        }

        public void
        finished(Ice.Current c, Ice.Object servant, Object cookie)
        {
        }

        public void
        deactivate(String category)
        {
        }

        LocatorI(Ice.Object servant)
        {
            _servant = new DispatchInterceptorI(servant);
        }

        private Ice.Object _servant;
    }

    public int
    run(String[] args)
    {
        args = communicator().getProperties().parseCommandLineOptions("JDBC", args);

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
            Class.forName("com.mysql.jdbc.Driver").newInstance();
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

        long timeout = properties.getPropertyAsIntWithDefault("SessionTimeout", 30);

        ReapThread reaper = new ReapThread(logger, timeout);
        reaper.start();

        //
        // Create an object adapter
        //
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("SessionFactory");

        SQLRequestContext.initialize(logger, pool);
        adapter.addServantLocator(new LocatorI(new BookI()), "book");
        
        adapter.add(new SessionFactoryI(logger, reaper, timeout), communicator().stringToIdentity("SessionFactory"));
        adapter.add(new Glacier2SessionManagerI(logger, reaper),
                    communicator().stringToIdentity("LibrarySessionManager"));

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

    static public void
    main(String[] args)
    {
        Server app = new Server();
        int status = app.main("demo.Database.library.Server", args, "config.server");
        System.exit(status);
    }
}
