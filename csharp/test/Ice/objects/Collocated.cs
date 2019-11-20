//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.objects.Test;

namespace Ice
{
    namespace objects
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {
                var initData = new InitializationData();
                initData.typeIdNamespaces = new string[] { "Ice.objects.TypeId" };
                initData.properties = createTestProperties(ref args);
                initData.properties.setProperty("Ice.Warn.Dispatch", "0");
                using (var communicator = initialize(initData))
                {
                    communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                    adapter.Add(new InitialI(adapter), Ice.Util.stringToIdentity("initial"));
                    adapter.Add(new F2I(), Ice.Util.stringToIdentity("F21"));
                    var uoet = new UnexpectedObjectExceptionTestI();
                    adapter.Add((incoming, current) => uoet.Dispatch(incoming, current), Util.stringToIdentity("uoet"));
                    Test.AllTests.allTests(this);
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }
        }
    }
}
