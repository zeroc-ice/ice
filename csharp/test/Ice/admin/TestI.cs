//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Collections.Generic;
using Ice.admin.Test;

namespace Ice
{
    namespace admin
    {
        public class TestFacetI : Test.TestFacet
        {
            public void op(Ice.Current current)
            {
            }
        }

        public class RemoteCommunicatorI : Test.RemoteCommunicator
        {
            public RemoteCommunicatorI(Ice.Communicator communicator)
            {
                _communicator = communicator;
            }

            public Ice.IObjectPrx getAdmin(Ice.Current current)
            {
                return _communicator.getAdmin();
            }

            public Dictionary<string, string> getChanges(Ice.Current current)
            {
                lock (this)
                {
                    return _changes;
                }
            }

            public void print(string message, Ice.Current current)
            {
                _communicator.getLogger().print(message);
            }

            public void trace(string category, string message, Ice.Current current)
            {
                _communicator.getLogger().trace(category, message);
            }

            public void warning(string message, Ice.Current current)
            {
                _communicator.getLogger().warning(message);
            }

            public void error(string message, Ice.Current current)
            {
                _communicator.getLogger().error(message);
            }

            public void shutdown(Ice.Current current)
            {
                _communicator.shutdown();
            }

            public void waitForShutdown(Ice.Current current)
            {
                //
                // Note that we are executing in a thread of the *main* communicator,
                // not the one that is being shut down.
                //
                _communicator.waitForShutdown();
            }

            public void destroy(Ice.Current current)
            {
                _communicator.destroy();
            }

            public void updated(Dictionary<string, string> changes)
            {
                lock (this)
                {
                    _changes = changes;
                }
            }

            private Ice.Communicator _communicator;
            private Dictionary<string, string> _changes;
        }

        public class RemoteCommunicatorFactoryI : Test.RemoteCommunicatorFactory
        {
            public Test.RemoteCommunicatorPrx createCommunicator(Dictionary<string, string> props, Ice.Current current)
            {
                //
                // Prepare the property set using the given properties.
                //
                Ice.InitializationData init = new Ice.InitializationData();
                init.properties = Ice.Util.createProperties();
                foreach (KeyValuePair<string, string> e in props)
                {
                    init.properties.setProperty(e.Key, e.Value);
                }

                if (init.properties.getPropertyAsInt("NullLogger") > 0)
                {
                    init.logger = new NullLogger();
                }

                //
                // Initialize a new communicator.
                //
                Ice.Communicator communicator = Ice.Util.initialize(init);

                //
                // Install a custom admin facet.
                //
                try
                {
                    communicator.addAdminFacet<TestFacet, TestFacetTraits>(new TestFacetI(), "TestFacet");
                }
                catch (System.ArgumentException ex)
                {
                }

                //
                // The RemoteCommunicator servant also implements PropertiesAdminUpdateCallback.
                // Set the callback on the admin facet.
                //
                RemoteCommunicatorI servant = new RemoteCommunicatorI(communicator);
                var propFacet = communicator.findAdminFacet("Properties").servant;

                if (propFacet != null)
                {
                    Ice.NativePropertiesAdmin admin = (Ice.NativePropertiesAdmin)propFacet;
                    Debug.Assert(admin != null);
                    admin.addUpdateCallback(servant.updated);
                }

                return current.adapter.Add(servant);
            }

            public void shutdown(Ice.Current current)
            {
                current.adapter.GetCommunicator().shutdown();
            }

            private class NullLogger : Ice.Logger
            {
                public void print(string message)
                {
                }

                public void trace(string category, string message)
                {
                }

                public void warning(string message)
                {
                }

                public void error(string message)
                {
                }

                public string getPrefix()
                {
                    return "NullLogger";
                }

                public Ice.Logger cloneWithPrefix(string prefix)
                {
                    return this;
                }
            }
        }
    }
}
