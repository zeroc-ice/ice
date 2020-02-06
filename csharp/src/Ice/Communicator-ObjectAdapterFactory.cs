//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    public sealed partial class Communicator
    {
        internal ObjectAdapter CreateObjectAdapter(string name, IRouterPrx? router)
        {
            lock (this)
            {
                if (_isShutdown)
                {
                    throw new CommunicatorDestroyedException();
                }

                if (name.Length > 0)
                {
                    if (_adapterNamesInUse.Contains(name))
                    {
                        throw new System.ArgumentException(
                            $"An object adapter with name `{name}' is already registered", nameof(name));
                    }
                    _adapterNamesInUse.Add(name);
                }
            }

            // Must be called outside the synchronization since the constructor can make client invocations
            // on the router if it's set.
            ObjectAdapter? adapter = null;
            try
            {
                adapter = new ObjectAdapter(this, name, router);
                lock (this)
                {
                    if (_isShutdown)
                    {
                        throw new CommunicatorDestroyedException();
                    }
                    _adapters.Add(adapter);
                    return adapter;
                }
            }
            catch (System.Exception)
            {
                if (adapter != null)
                {
                    adapter.Destroy();
                }

                if (name.Length > 0)
                {
                    lock (this)
                    {
                        _adapterNamesInUse.Remove(name);
                    }
                }
                throw;
            }
        }

        internal ObjectAdapter? FindObjectAdapter(IObjectPrx proxy)
        {
            List<ObjectAdapter> adapters;
            lock (this)
            {
                if (_isShutdown)
                {
                    return null;
                }

                adapters = new List<ObjectAdapter>(_adapters);
            }

            foreach (ObjectAdapter adapter in adapters)
            {
                try
                {
                    if (adapter.isLocal(proxy))
                    {
                        return adapter;
                    }
                }
                catch (ObjectAdapterDeactivatedException)
                {
                    // Ignore.
                }
            }

            return null;
        }

        internal void RemoveObjectAdapter(ObjectAdapter adapter)
        {
            lock (this)
            {
                if (_isShutdown)
                {
                    return;
                }

                _adapters.Remove(adapter);
                if (adapter.Name.Length > 0)
                {
                    _adapterNamesInUse.Remove(adapter.Name);
                }
            }
        }

        private readonly HashSet<string> _adapterNamesInUse = new HashSet<string>();
        private readonly List<ObjectAdapter> _adapters = new List<ObjectAdapter>();
        private bool _isShutdown = false;
    }

}
