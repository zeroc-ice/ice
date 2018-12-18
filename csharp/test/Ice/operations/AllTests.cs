// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    namespace operations
    {
        public class AllTests : global::Test.AllTests
        {
            public static Test.MyClassPrx allTests(global::Test.TestHelper helper)
            {
                Ice.Communicator communicator = helper.communicator();
                var output = helper.getWriter();
                output.Flush();
                string rf = "test:" + helper.getTestEndpoint(0);
                Ice.ObjectPrx baseProxy = communicator.stringToProxy(rf);
                var cl = Test.MyClassPrxHelper.checkedCast(baseProxy);
                var derivedProxy = Test.MyDerivedClassPrxHelper.checkedCast(cl);

                output.Write("testing twoway operations... ");
                output.Flush();
                Twoways.twoways(helper, cl);
                Twoways.twoways(helper, derivedProxy);
                derivedProxy.opDerived();
                output.WriteLine("ok");

                output.Write("testing oneway operations... ");
                output.Flush();
                Oneways.oneways(helper, cl);
                output.WriteLine("ok");

                output.Write("testing twoway operations with AMI... ");
                output.Flush();
                TwowaysAMI.twowaysAMI(helper, cl);
                TwowaysAMI.twowaysAMI(helper, derivedProxy);
                output.WriteLine("ok");

                output.Write("testing oneway operations with AMI... ");
                output.Flush();
                OnewaysAMI.onewaysAMI(helper, cl);
                output.WriteLine("ok");

                output.Write("testing batch oneway operations... ");
                output.Flush();
                BatchOneways.batchOneways(helper, cl);
                BatchOneways.batchOneways(helper, derivedProxy);
                output.WriteLine("ok");

                output.Write("testing batch AMI oneway operations... ");
                output.Flush();
                BatchOnewaysAMI.batchOneways(cl);
                BatchOnewaysAMI.batchOneways(derivedProxy);
                output.WriteLine("ok");
                return cl;
            }
        }
    }
}
