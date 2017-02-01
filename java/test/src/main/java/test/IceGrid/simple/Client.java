// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceGrid.simple;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();
        boolean withDeploy = false;
        for(String arg : args)
        {
            if(arg.equals("--with-deploy"))
            {
                withDeploy = true;
                break;
            }
        }

        if(!withDeploy)
        {
            AllTests.allTests(this);
        }
        else
        {
            AllTests.allTestsWithDeploy(communicator, getWriter());
        }

        return 0;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        initData.properties.setProperty("Ice.Package.Test", "test.IceGrid.simple");
        return initData;
    }

    public static void main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);

        System.gc();
        System.exit(status);
    }
}
