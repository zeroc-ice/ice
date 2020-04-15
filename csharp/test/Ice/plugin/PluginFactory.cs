//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

public class PluginFactory : Ice.IPluginFactory
{
    public Ice.IPlugin Create(Ice.Communicator communicator, string name, string[] args) => new Plugin(args);

    internal class Plugin : Ice.IPlugin
    {
        public Plugin(string[] args) => _args = args;

        public void Initialize()
        {
            _initialized = true;
            TestHelper.Assert(_args.Length == 3);
            TestHelper.Assert(_args[0] == "C:\\Program Files\\");
            TestHelper.Assert(_args[1] == "--DatabasePath");
            TestHelper.Assert(_args[2] == "C:\\Program Files\\Application\\db");
        }

        public void Destroy() => _destroyed = true;

        ~Plugin()
        {
            if (!_initialized)
            {
                Console.WriteLine("Plugin not initialized");
            }
            if (!_destroyed)
            {
                Console.WriteLine("Plugin not destroyed");
            }
        }

        private bool _initialized = false;
        private bool _destroyed = false;
        private string[] _args;
    }
}
