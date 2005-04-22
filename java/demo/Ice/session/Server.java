// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Server
{
    public static void
    main(String[] args)
    {
        SessionServer app = new SessionServer();
        int status = app.main("Server", args, "config");
        System.exit(status);
    }
}
