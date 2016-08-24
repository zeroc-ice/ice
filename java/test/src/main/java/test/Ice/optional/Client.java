// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.optional;

import test.Ice.optional.Test.InitialPrx;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        java.io.PrintWriter out = getWriter();
        InitialPrx initial = AllTests.allTests(this, false, out);
        initial.shutdown();
        return 0;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.optional");
        return r;
    }

    public static void main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);

        System.gc();
        System.exit(status);
    }
}
