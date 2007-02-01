// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Server
{
    public static void
    main(String[] args)
    {
        CallbackServer app = new CallbackServer();
        int status = app.main("Server", args);
        System.gc();
        System.exit(status);
    }
}
