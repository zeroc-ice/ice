// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Client extends Ice.Application
{
    public int
    run(String[] args)
    {
	return RunParser.runParser(appName(), args, communicator());
    }

    static public void
    main(String[] args)
    {
	Client app = new Client();
	app.main("demo.Freeze.library.Client", args, "config");
    }
}
