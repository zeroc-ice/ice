// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
        catch(com.zeroc.Ice.AdapterAlreadyActiveException ex)
        {
            ex.printStackTrace();
            throw new RuntimeException();
        }
        catch(com.zeroc.Ice.AdapterNotFoundException ex)
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
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.location");
        initData.properties.setProperty("Ice.Default.Locator", "locator:" + getTestEndpoint(initData.properties, 0));
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
