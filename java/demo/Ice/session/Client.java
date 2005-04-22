// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class Client
{
    public static void
    main(String[] args)
    {
        SessionClient app = new SessionClient();
        int status = app.main("Client", args, "config");
        System.exit(status);
    }
}
