// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

public class Client
{
    public static void
    main(String[] args)
    {
        CallbackClient app = new CallbackClient();
        int status = app.main("Client", args);
        System.exit(status);
    }
}
