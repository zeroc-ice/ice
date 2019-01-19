//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using Test;

namespace Ice
{
    namespace objects
    {
        public class Server : TestHelper
        {
            public static Ice.Value MyValueFactory(string type)
            {
                if(type.Equals("::Test::I"))
                {
                    return new II();
                }
                else if(type.Equals("::Test::J"))
                {
                    return new JI();
                }
                else if(type.Equals("::Test::H"))
                {
                    return new HI();
                }
                Debug.Assert(false); // Should never be reached
                return null;
            }

            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[]{"Ice.objects.TypeId"};
                initData.properties = createTestProperties(ref args);
                initData.properties.setProperty("Ice.Warn.Dispatch", "0");
                using(var communicator = initialize(initData))
                {
                    communicator.getValueFactoryManager().add(MyValueFactory, "::Test::I");
                    communicator.getValueFactoryManager().add(MyValueFactory, "::Test::J");
                    communicator.getValueFactoryManager().add(MyValueFactory, "::Test::H");

                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    Ice.Object @object = new InitialI(adapter);
                    adapter.add(@object, Ice.Util.stringToIdentity("initial"));
                    @object = new UnexpectedObjectExceptionTestI();
                    adapter.add(@object, Ice.Util.stringToIdentity("uoet"));
                    adapter.activate();
                    serverReady();
                    communicator.waitForShutdown();
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Server>(args);
            }
        }
    }
}
