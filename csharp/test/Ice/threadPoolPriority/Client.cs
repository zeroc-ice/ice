// Copyright (c) ZeroC, Inc.

namespace Ice.threadPoolPriority;

public class Client : global::Test.TestHelper
{
    public override void run(string[] args)
    {
        using Communicator communicator = initialize(ref args);
        TextWriter output = getWriter();
        output.Write("testing server priority... ");
        output.Flush();
        ObjectPrx obj = communicator.stringToProxy("test:" + getTestEndpoint(0) + " -t 10000");
        Test.PriorityPrx priority = Test.PriorityPrxHelper.checkedCast(obj);
        test(priority.getPriority() == "AboveNormal");
        output.WriteLine("ok");
        priority.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        global::Test.TestDriver.runTestAsync<Client>(args);
}
