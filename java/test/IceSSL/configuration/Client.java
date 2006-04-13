// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Client
{
    private static int
    run(String[] args, Ice.Communicator communicator)
    {
	if(args.length < 1)
	{
	    System.out.println("Usage: client testdir");
	    return 1;
	}

	AllTests.allTests(communicator, args[0]);

	return 0;
    }

    public static void main(String[] args)
    {
	int status = 0;
	Ice.Communicator communicator = null;

	try
	{
	    Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
	    communicator = Ice.Util.initialize(argsH);
	    status = run(argsH.value, communicator);
	}
	catch(Ice.LocalException ex)
	{
	    ex.printStackTrace();
	    status = 1;
	}

	if(communicator != null)
	{
	    try
	    {
		communicator.destroy();
	    }
	    catch(Ice.LocalException ex)
	    {
		ex.printStackTrace();
		status = 1;
	    }
	}

	System.gc();
	System.exit(status);
    }
}
