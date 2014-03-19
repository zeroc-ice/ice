// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;
import FilesystemDB.*;

public class Server extends Ice.Application
{
    public
    Server(String envName)
    {
        _envName = envName;
    }

    public int
    run(String[] args)
    {
        //
        // Create an object adapter
        //
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("MapFilesystem");

        Freeze.Connection connection = null;
        try
        {
            //
            // Open a connection to the files and directories
            // database. This should remain open for the duration of the
            // application for performance reasons.
            //
            connection = Freeze.Util.createConnection(communicator(), _envName);
            IdentityFileEntryMap fileDB = new IdentityFileEntryMap(connection, FileI.filesDB(), true);
            IdentityDirectoryEntryMap dirDB = new IdentityDirectoryEntryMap(
                connection, DirectoryI.directoriesDB(), true);

            //
            // Add default servants for the file and directory.
            //
            adapter.addDefaultServant(new FileI(communicator(), _envName), "file");
            adapter.addDefaultServant(new DirectoryI(communicator(), _envName), "");

            //
            // Ready to accept requests now
            //
            adapter.activate();

            //
            // Wait until we are done
            //
            communicator().waitForShutdown();
        }
        finally
        {
            //
            // Close the connection gracefully.
            //
            if(connection != null)
            {
                connection.close();
            }
        }

        return 0;
    }

    public static void
    main(String[] args)
    {
        Server app = new Server("db");
        app.main("demo.book.map_filesystem.Server", args, "config.server");
        System.exit(0);
    }

    private String _envName;
}
