// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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
        NestedClient app = new NestedClient();
        int status = app.main("Client", args, "config");
        System.exit(status);
    }
}
