// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
	// We must disable connection warnings, because we attempt to
	// ping the router before session establishment, as well as
	// after session destruction. Both will cause a
	// ConnectionLostException.
	//
	Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
	Ice.Properties properties = Ice.Util.getDefaultProperties(argsH);
	properties.setProperty("Ice.Warn.Connections", "0");

        CallbackClient app = new CallbackClient();
        int status = app.main("Client", argsH.value);
	System.gc();
        System.exit(status);
    }
}
