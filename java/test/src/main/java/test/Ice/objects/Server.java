// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.objects;

public class Server extends test.Util.Application
{
    private static class MyValueFactory implements com.zeroc.Ice.ValueFactory
    {
        @Override
        public com.zeroc.Ice.Value create(String type)
        {
            if(type.equals("::Test::I"))
            {
                return new HI();
            }
            else if(type.equals("::Test::J"))
            {
                return new HI();
            }
            else if(type.equals("::Test::H"))
            {
                return new HI();
            }

            assert (false); // Should never be reached
            return null;
        }
    }

    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();
        com.zeroc.Ice.ValueFactory factory = new MyValueFactory();
        communicator.getValueFactoryManager().add(factory, "::Test::I");
        communicator.getValueFactoryManager().add(factory, "::Test::J");
        communicator.getValueFactoryManager().add(factory, "::Test::H");

        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        com.zeroc.Ice.Object object = new InitialI(adapter);
        adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("initial"));
        object = new UnexpectedObjectExceptionTestI();
        adapter.add(object, com.zeroc.Ice.Util.stringToIdentity("uoet"));
        adapter.activate();

        return WAIT;
    }

    @Override
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.objects");
        r.initData.properties.setProperty("TestAdapter.Endpoints", "default -p 12010");
        return r;
    }

    public static void main(String[] args)
    {
        Server app = new Server();
        int result = app.main("Server", args);
        System.gc();
        System.exit(result);
    }
}
