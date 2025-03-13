// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Text;

namespace Test;

public interface ControllerHelper
{
    void serverReady();
    void communicatorInitialized(Ice.Communicator communicator);
}

public interface PlatformAdapter
{
    bool isEmulator();

    string processControllerRegistryHost();

    string processControllerIdentity();
}

public abstract class TestHelper
{
    // A custom trace listener that always aborts the application upon failure.
    internal class TestTraceListener : DefaultTraceListener
    {
        public override void Fail(string message)
        {
            TestHelper.fail(message, null);
        }

        public override void Fail(string message, string detailMessage)
        {
            TestHelper.fail(message, detailMessage);
        }
    }

    static TestHelper()
    {
        // Replace the default trace listener that is responsible of displaying the retry/abort dialog
        // with our custom trace listener that always aborts upon failure.
        // see: https://docs.microsoft.com/en-us/dotnet/api/system.diagnostics.defaulttracelistener?view=net-5.0#remarks
        Trace.Listeners.Clear();
        Trace.Listeners.Add(new TestTraceListener());
    }

    public virtual void run(string[] args)
    {
        throw new NotImplementedException();
    }

    public virtual Task runAsync(string[] args)
    {
        run(args);
        return Task.CompletedTask;
    }

    public string getTestEndpoint(int num = 0, string protocol = "")
    {
        return getTestEndpoint(_communicator.getProperties(), num, protocol);
    }

    public static string getTestEndpoint(Ice.Properties properties, int num = 0, string protocol = "")
    {
        StringBuilder sb = new StringBuilder();
        sb.Append(protocol.Length == 0 ? properties.getIceProperty("Ice.Default.Protocol") :
                                   protocol);
        sb.Append(" -p ");
        sb.Append(properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num);
        return sb.ToString();
    }

    public string getTestHost()
    {
        return getTestHost(_communicator.getProperties());
    }

    public static string getTestHost(Ice.Properties properties)
    {
        return properties.getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");
    }

    public String getTestProtocol()
    {
        return getTestProtocol(_communicator.getProperties());
    }

    public static String getTestProtocol(Ice.Properties properties)
    {
        return properties.getIceProperty("Ice.Default.Protocol");
    }

    public int getTestPort(int num)
    {
        return getTestPort(_communicator.getProperties(), num);
    }

    public static int getTestPort(Ice.Properties properties, int num)
    {
        return properties.getPropertyAsIntWithDefault("Test.BasePort", 12010) + num;
    }

    public TextWriter getWriter()
    {
        if (_writer == null)
        {
            return Console.Out;
        }
        else
        {
            return _writer;
        }
    }

    public void setWriter(TextWriter writer)
    {
        _writer = writer;
    }

    public Ice.Properties createTestProperties(ref string[] args)
    {
        Ice.Properties properties = new Ice.Properties(ref args);
        args = properties.parseCommandLineOptions("Test", args);
        return properties;
    }

    public Ice.Communicator initialize(ref string[] args)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = createTestProperties(ref args);
        return initialize(initData);
    }

    public Ice.Communicator initialize(Ice.Properties properties)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = properties;
        return initialize(initData);
    }

    public Ice.Communicator initialize(Ice.InitializationData initData)
    {
        Ice.Communicator communicator = Ice.Util.initialize(initData);
        if (_communicator == null)
        {
            _communicator = communicator;
        }
        if (_controllerHelper != null)
        {
            _controllerHelper.communicatorInitialized(communicator);
        }
        return communicator;
    }

    public Ice.Communicator communicator()
    {
        return _communicator;
    }

    public static void test(bool b)
    {
        if (!b)
        {
            fail(null, null);
        }
    }

    internal static void fail(string message, string detailMessage)
    {
        var sb = new StringBuilder();
        sb.Append("failed:\n");
        if (message != null && message.Length > 0)
        {
            sb.Append("message: ").Append(message).Append('\n');
        }
        if (detailMessage != null && detailMessage.Length > 0)
        {
            sb.Append("details: ").Append(detailMessage).Append('\n');
        }
        try
        {
            sb.Append(new StackTrace(fNeedFileInfo: true).ToString()).Append('\n');
        }
        catch
        {
        }

        Console.WriteLine(sb.ToString());
        Environment.Exit(1);
    }

    public void setControllerHelper(ControllerHelper controllerHelper)
    {
        _controllerHelper = controllerHelper;
    }

    public void serverReady()
    {
        if (_controllerHelper != null)
        {
            _controllerHelper.serverReady();
        }
    }

    private Ice.Communicator _communicator;
    private ControllerHelper _controllerHelper;
    private TextWriter _writer;
}

public abstract class AllTests
{
    protected static void test(bool b)
    {
        if (!b)
        {
            Debug.Assert(false);
            throw new Exception();
        }
    }
}

public static class TestDriver
{
    public static async Task<int> runTestAsync<T>(string[] args)
        where T : TestHelper, new()
    {
        int status = 0;
        try
        {
            var testHelper = new T();
            await testHelper.runAsync(args);
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
            status = 1;
        }
        return status;
    }
}
