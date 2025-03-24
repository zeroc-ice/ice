// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        Console.Out.Write("testing preloading assemblies... ");
        Console.Out.Flush();
        var info = new User.UserInfo();

        Ice.Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.PreloadAssemblies", "0");
#pragma warning disable SYSLIB0012 // Type or member is obsolete
        string assembly =
            string.Format("{0}/core.dll",
                          Path.GetFileName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().CodeBase)));

        using (var communicator = initialize(properties))
        {
            test(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault((e) => e.CodeBase.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase)) == null);
        }
        properties.setProperty("Ice.PreloadAssemblies", "1");
        using (var communicator = initialize(properties))
        {
            test(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault((e) => e.CodeBase.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase)) != null);
        }
        Console.Out.WriteLine("ok");
#pragma warning restore SYSLIB0012 // Type or member is obsolete
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
