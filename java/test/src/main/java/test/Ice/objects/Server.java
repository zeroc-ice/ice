// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.objects;

public class Server extends test.TestHelper
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

    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.objects");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));

            com.zeroc.Ice.ValueFactory factory = new MyValueFactory();
            communicator.getValueFactoryManager().add(factory, "::Test::I");
            communicator.getValueFactoryManager().add(factory, "::Test::J");
            communicator.getValueFactoryManager().add(factory, "::Test::H");

            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new InitialI(adapter), com.zeroc.Ice.Util.stringToIdentity("initial"));
            adapter.add(new UnexpectedObjectExceptionTestI(), com.zeroc.Ice.Util.stringToIdentity("uoet"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
