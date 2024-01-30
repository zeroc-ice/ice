//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace Ice
{
    namespace operations
    {
        public class AllTests : global::Test.AllTests
        {
            public static async Task<Test.MyClassPrx> allTestsAsync(global::Test.TestHelper helper)
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
                await TwowaysAMI.twowaysAMIAsync(helper, cl);
                await TwowaysAMI.twowaysAMIAsync(helper, derivedProxy);
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
                await BatchOnewaysAMI.batchOnewaysAsync(cl);
                await BatchOnewaysAMI.batchOnewaysAsync(derivedProxy);
                output.WriteLine("ok");
                return cl;
            }
        }
    }
}
