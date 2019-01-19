//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.dispatcher;

public class Collocated extends test.TestHelper
{
    public void run(String[] args)
    {
        Dispatcher dispatcher = new Dispatcher();
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = createTestProperties(args);
            initData.properties.setProperty("Ice.Package.Test", "test.Ice.dispatcher");
            initData.dispatcher = dispatcher;
            try(Ice.Communicator communicator = initialize(initData))
            {
                communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1, "tcp"));
                communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");
                Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

                adapter.add(new TestI(dispatcher), Ice.Util.stringToIdentity("test"));
                //adapter.activate(); // Don't activate OA to ensure collocation is used.
                adapter2.add(new TestControllerI(adapter), Ice.Util.stringToIdentity("testController"));
                //adapter2.activate(); // Don't activate OA to ensure collocation is used.
                AllTests.allTests(this, dispatcher);
            }
        }
        finally
        {
            dispatcher.terminate();
        }
    }
}
