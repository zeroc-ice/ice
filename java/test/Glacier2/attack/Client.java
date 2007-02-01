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
        // We want to check whether the client retries for evicted
        // proxies, even with regular retries disabled.
        //
        Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.RetryIntervals", "-1");

        AttackClient app = new AttackClient();
        int status = app.main("Client", argsH.value, initData);
        System.gc();
        System.exit(status);
    }
}
