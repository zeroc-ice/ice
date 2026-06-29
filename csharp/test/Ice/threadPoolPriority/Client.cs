// Copyright (c) ZeroC, Inc.

namespace Ice.threadPoolPriority;

public class Client : global::Test.TestHelper
{
    public override void run(string[] args)
    {
        using Communicator communicator = initialize(ref args);
        TextWriter output = getWriter();

        output.Write("testing thread priority property parsing... ");
        Properties properties = new Properties();
        properties.setProperty("Ice.ThreadPriority", "ThreadPriority.AboveNormal");
        test(Ice.Internal.Util.getThreadPriorityProperty(properties, "Ice") ==
            System.Threading.ThreadPriority.AboveNormal);
        properties.setProperty("Ice.ThreadPriority", "AboveNormal");
        test(Ice.Internal.Util.getThreadPriorityProperty(properties, "Ice") ==
            System.Threading.ThreadPriority.AboveNormal);
        properties.setProperty("Ice.ThreadPriority", "");
        test(Ice.Internal.Util.getThreadPriorityProperty(properties, "Ice") ==
            System.Threading.ThreadPriority.Normal);
        properties.setProperty("Ice.ThreadPriority", "BogusPriority");
        try
        {
            Ice.Internal.Util.getThreadPriorityProperty(properties, "Ice");
            test(false);
        }
        catch (PropertyException)
        {
        }
        output.WriteLine("ok");

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
