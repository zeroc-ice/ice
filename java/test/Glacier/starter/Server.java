// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
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
        CallbackServer app = new CallbackServer();
        int status = app.main("Server", args);
        System.exit(status);
    }
}
