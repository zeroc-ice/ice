// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class Server
{
    public static void
    main(String[] args)
    {
        NestedServer app = new NestedServer();
        int status = app.main("Server", args, "config.server");
        System.exit(status);
    }
}
