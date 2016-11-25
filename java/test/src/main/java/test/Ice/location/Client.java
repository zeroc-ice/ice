// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.location");
        r.initData.properties.setProperty("Ice.Default.Locator", "locator:" + getTestEndpoint(r.initData.properties, 0));
        return r;
    }

    public static void main(String[] args)
    {
        Client app = new Client();
        int result = app.main("Client", args);
        System.gc();
        System.exit(result);
    }
}
