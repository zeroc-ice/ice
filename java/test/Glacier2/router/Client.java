// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Warn.Connections", "0");

        CallbackClient app = new CallbackClient();
        int status = app.main("Client", argsH.value, initData);
        System.gc();
        System.exit(status);
    }
}
