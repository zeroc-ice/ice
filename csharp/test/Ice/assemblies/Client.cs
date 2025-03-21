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
        User.UserInfo info = new User.UserInfo();

        Ice.Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.PreloadAssemblies", "0");

        string assembly =
            string.Format("{0}/core.dll",
#pragma warning disable SYSLIB0012 // Type or member is obsolete
                          Path.GetFileName(Path.GetDirectoryName(Assembly.GetExecutingAssembly().CodeBase)));
#pragma warning restore SYSLIB0012 // Type or member is obsolete
        using (var communicator = initialize(properties))
        {
            test(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault((e) =>
                    {
#pragma warning disable SYSLIB0012 // Type or member is obsolete
                        return e.CodeBase.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase);
#pragma warning restore SYSLIB0012 // Type or member is obsolete
                    }) == null);
        }
        properties.setProperty("Ice.PreloadAssemblies", "1");
        using (var communicator = initialize(properties))
        {
            test(AppDomain.CurrentDomain.GetAssemblies().FirstOrDefault((e) =>
                    {
#pragma warning disable SYSLIB0012 // Type or member is obsolete
                        return e.CodeBase.EndsWith(assembly, StringComparison.InvariantCultureIgnoreCase);
#pragma warning restore SYSLIB0012 // Type or member is obsolete
                    }) != null);
        }

        Console.Out.WriteLine("ok");
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
