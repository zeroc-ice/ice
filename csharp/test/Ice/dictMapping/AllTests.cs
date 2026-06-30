// Copyright (c) ZeroC, Inc.

namespace Ice.dictMapping;

public class AllTests : global::Test.AllTests
{
    public static async Task<Test.MyInterfacePrx> allTests(global::Test.TestHelper helper, bool collocated)
    {
        Communicator communicator = helper.communicator();
        TextWriter output = helper.getWriter();
        output.Flush();
        string rf = "test:" + helper.getTestEndpoint(0);
        ObjectPrx baseProxy = communicator.stringToProxy(rf);
        Test.MyInterfacePrx cl = Test.MyInterfacePrxHelper.checkedCast(baseProxy);

        output.Write("testing twoway operations... ");
        output.Flush();
        Twoways.twoways(communicator, cl);
        output.WriteLine("ok");

        if (!collocated)
        {
            output.Write("testing twoway operations with AMI... ");
            output.Flush();
            await TwowaysAMI.twowaysAMI(cl);
            output.WriteLine("ok");
        }
        return cl;
    }
}
