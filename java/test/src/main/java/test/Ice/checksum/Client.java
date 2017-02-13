// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.checksum;

import test.Ice.checksum.Test.ChecksumPrx;

public class Client extends test.Util.Application
{
    @Override
    public int
    run(String[] args)
    {
        Ice.Communicator communicator = communicator();
        ChecksumPrx checksum = AllTests.allTests(communicator, false, getWriter());
        checksum.shutdown();
        return 0;
    }


    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.checksum");
        return initData;
    }

    public static void
    main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);
        
        System.gc();
        System.exit(status);
    }
}
