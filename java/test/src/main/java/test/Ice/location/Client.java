// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.location;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        try
        {
            AllTests.allTests(this);
        }
        catch(Ice.AdapterAlreadyActiveException ex)
        {
            ex.printStackTrace();
            throw new RuntimeException();
        }
        catch(Ice.AdapterNotFoundException ex)
        {
            ex.printStackTrace();
            throw new RuntimeException();
        }
        catch(InterruptedException ex)
        {
            ex.printStackTrace();
            throw new RuntimeException();
        }

        return 0;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.location");
        initData.properties.setProperty("Ice.Default.Locator", "locator:default -p 12010");
        return initData;
    }

    public static void main(String[] args)
    {
        Client app = new Client();
        int result = app.main("Client", args);
        System.gc();
        System.exit(result);
    }
}
