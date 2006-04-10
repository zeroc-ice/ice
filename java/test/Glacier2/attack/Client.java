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
    public static void
    main(String[] args)
    {
	//
	// We want to check whether the client retries for evicted
	// proxies, even with regular retries disabled.
	//
	Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
	Ice.Properties properties = Ice.Util.getDefaultProperties(argsH);
	properties.setProperty("Ice.RetryIntervals", "-1");

        AttackClient app = new AttackClient();
        int status = app.main("Client", argsH.value);
	System.gc();
        System.exit(status);
    }
}
