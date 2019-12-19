//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System.Collections.Generic;

    public sealed class ObjectAdapterFactory
    {
        public void shutdown()
        {
            List<Ice.ObjectAdapter> adapters;
            lock (this)
            {
                //
                // Ignore shutdown requests if the object adapter factory has
                // already been shut down.
                //
                if (_communicator == null)
                {
                    return;
                }

                adapters = new List<Ice.ObjectAdapter>(_adapters);

                _communicator = null;

                System.Threading.Monitor.PulseAll(this);
            }

            //
            // Deactivate outside the thread synchronization, to avoid
            // deadlocks.
            //
            foreach (Ice.ObjectAdapter adapter in adapters)
            {
                adapter.Deactivate();
            }
        }

        public void waitForShutdown()
        {
            List<Ice.ObjectAdapter> adapters;
            lock (this)
            {
                //
                // First we wait for the shutdown of the factory itself.
                //
                while (_communicator != null)
                {
                    System.Threading.Monitor.Wait(this);
                }

                adapters = new List<Ice.ObjectAdapter>(_adapters);
            }

            //
            // Now we wait for deactivation of each object adapter.
            //
            foreach (Ice.ObjectAdapter adapter in adapters)
            {
                adapter.WaitForDeactivate();
            }
        }

        public bool isShutdown()
        {
            lock (this)
            {
                return _communicator == null;
            }
        }

        public void destroy()
        {
            //
            // First wait for shutdown to finish.
            //
            waitForShutdown();

            List<Ice.ObjectAdapter> adapters;
            lock (this)
            {
                adapters = new List<Ice.ObjectAdapter>(_adapters);
            }

            foreach (Ice.ObjectAdapter adapter in adapters)
            {
                adapter.Destroy();
            }

            lock (this)
            {
                _adapters.Clear();
            }
        }

        public void
        updateConnectionObservers()
        {
            List<Ice.ObjectAdapter> adapters;
            lock (this)
            {
                adapters = new List<Ice.ObjectAdapter>(_adapters);
            }

            foreach (Ice.ObjectAdapter adapter in adapters)
            {
                adapter.updateConnectionObservers();
            }
        }

        public void
        updateThreadObservers()
        {
            List<Ice.ObjectAdapter> adapters;
            lock (this)
            {
                adapters = new List<Ice.ObjectAdapter>(_adapters);
            }

            foreach (Ice.ObjectAdapter adapter in adapters)
            {
                adapter.updateThreadObservers();
            }
        }

        public Ice.ObjectAdapter createObjectAdapter(string name, Ice.RouterPrx? router)
        {
            lock (this)
            {
                if (_communicator == null)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }

                if (name.Length > 0)
                {
                    if (_adapterNamesInUse.Contains(name))
                    {
                        throw new System.ArgumentException($"An object adapter with name `{name}' is already registered",
                            nameof(name));
                    }
                    _adapterNamesInUse.Add(name);
                }
            }

            //
            // Must be called outside the synchronization since initialize can make client invocations
            // on the router if it's set.
            //
            Ice.ObjectAdapter? adapter = null;
            try
            {
                if (name.Length == 0)
                {
                    string uuid = System.Guid.NewGuid().ToString();
                    adapter = new Ice.ObjectAdapter(_communicator, this, uuid, null, true);
                }
                else
                {
                    adapter = new Ice.ObjectAdapter(_communicator, this, name, router, false);
                }

                lock (this)
                {
                    if (_communicator == null)
                    {
                        throw new Ice.CommunicatorDestroyedException();
                    }
                    _adapters.Add(adapter);
                }
            }
            catch (Ice.CommunicatorDestroyedException)
            {
                if (adapter != null)
                {
                    adapter.Destroy();
                }
                throw;
            }
            catch (System.Exception)
            {
                if (name.Length > 0)
                {
                    lock (this)
                    {
                        _adapterNamesInUse.Remove(name);
                    }
                }
                throw;
            }

            return adapter;
        }

        public Ice.ObjectAdapter findObjectAdapter(Ice.IObjectPrx proxy)
        {
            List<Ice.ObjectAdapter> adapters;
            lock (this)
            {
                if (_communicator == null)
                {
                    return null;
                }

                adapters = new List<Ice.ObjectAdapter>(_adapters);
            }

            foreach (Ice.ObjectAdapter adapter in adapters)
            {
                try
                {
                    if (adapter.isLocal(proxy))
                    {
                        return adapter;
                    }
                }
                catch (Ice.ObjectAdapterDeactivatedException)
                {
                    // Ignore.
                }
            }

            return null;
        }

        public void removeObjectAdapter(Ice.ObjectAdapter adapter)
        {
            lock (this)
            {
                if (_communicator == null)
                {
                    return;
                }

                _adapters.Remove(adapter);
                _adapterNamesInUse.Remove(adapter.GetName());
            }
        }

        //
        // Only for use by Instance.
        //
        internal ObjectAdapterFactory(Ice.Communicator communicator)
        {
            _communicator = communicator;
            _adapterNamesInUse = new HashSet<string>();
            _adapters = new List<Ice.ObjectAdapter>();
        }

        private Ice.Communicator _communicator;
        private HashSet<string> _adapterNamesInUse;
        private List<Ice.ObjectAdapter> _adapters;
    }

}
