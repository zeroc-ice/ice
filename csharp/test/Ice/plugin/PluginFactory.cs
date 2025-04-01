// Copyright (c) ZeroC, Inc.

public class PluginFactory : Ice.PluginFactory
{
    public Ice.Plugin create(Ice.Communicator communicator, string name, string[] args) =>
        new Plugin(communicator, args);

    internal class Plugin : Ice.Plugin
    {
        public Plugin(Ice.Communicator communicator, string[] args) => _args = args;

        public void initialize()
        {
            test(!_initialized);
            _initialized = true;
            test(_args.Length == 3);
            test(_args[0] == "C:\\Program Files\\");
            test(_args[1] == "--DatabasePath");
            test(_args[2] == "C:\\Program Files\\Application\\db");
        }

        public void destroy() => test(_initialized);

        private static void test(bool b) => global::Test.TestHelper.test(b);

        private bool _initialized = false;
        private readonly string[] _args;
    }
}
