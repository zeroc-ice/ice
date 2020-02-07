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
            ObjectAdapter? adapter = null;
            try
            {
                if (name.Length == 0)
                {
                    string uuid = System.Guid.NewGuid().ToString();
                    adapter = new ObjectAdapter(this, uuid, null, true);
                }
                else
                {
                    adapter = new ObjectAdapter(this, name, router, false);
                }

                lock (this)
                {
                    if (_isShutdown)
                    {
                        throw new CommunicatorDestroyedException();
                    }
                    _adapters.Add(adapter);
                }
            }
            catch (CommunicatorDestroyedException)
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
                    if (adapter.IsLocal(proxy))
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
                _adapterNamesInUse.Remove(adapter.GetName());
            }
        }

        private readonly HashSet<string> _adapterNamesInUse = new HashSet<string>();
        private readonly List<ObjectAdapter> _adapters = new List<ObjectAdapter>();
        private bool _isShutdown = false;
    }

}
