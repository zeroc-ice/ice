// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
