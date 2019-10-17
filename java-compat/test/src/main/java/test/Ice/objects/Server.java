//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.objects;

public class Server extends test.TestHelper
{
    private static class MyValueFactory implements Ice.ValueFactory
    {
        @Override
        public Ice.Object create(String type)
        {
            if(type.equals("::Test::I"))
            {
                return new II();
            }
            else if(type.equals("::Test::J"))
            {
                return new JI();
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
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.objects");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.MessageSizeMax", "2048"); // Needed on some Android versions

        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ValueFactory factory = new MyValueFactory();
            communicator.getValueFactoryManager().add(factory, "::Test::I");
            communicator.getValueFactoryManager().add(factory, "::Test::J");
            communicator.getValueFactoryManager().add(factory, "::Test::H");

            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.Object object = new InitialI(adapter);
            adapter.add(object, Ice.Util.stringToIdentity("initial"));
            adapter.add(new F2I(), Ice.Util.stringToIdentity("F21"));
            object = new UnexpectedObjectExceptionTestI();
            adapter.add(object, Ice.Util.stringToIdentity("uoet"));
            adapter.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
