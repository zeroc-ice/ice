// Copyright (c) ZeroC, Inc.

using System.Net.Security;

namespace Ice.Internal;

public sealed class ObjectAdapterFactory
{
    public void shutdown()
    {
        List<Ice.ObjectAdapter> adapters;
        lock (_mutex)
        {
            //
            // Ignore shutdown requests if the object adapter factory has
            // already been shut down.
            //
            if (_instance == null)
            {
                return;
            }

            adapters = new List<Ice.ObjectAdapter>(_adapters);

            _instance = null;
            _communicator = null;

            System.Threading.Monitor.PulseAll(_mutex);
        }

        //
        // Deactivate outside the thread synchronization, to avoid
        // deadlocks.
        //
        foreach (Ice.ObjectAdapter adapter in adapters)
        {
            adapter.deactivate();
        }
    }

    public void waitForShutdown()
    {
        List<Ice.ObjectAdapter> adapters;
        lock (_mutex)
        {
            //
            // First we wait for the shutdown of the factory itself.
            //
            while (_instance != null)
            {
                System.Threading.Monitor.Wait(_mutex);
            }

            adapters = new List<Ice.ObjectAdapter>(_adapters);
        }

        //
        // Now we wait for deactivation of each object adapter.
        //
        foreach (Ice.ObjectAdapter adapter in adapters)
        {
            adapter.waitForDeactivate();
        }
    }

    public bool isShutdown()
    {
        lock (_mutex)
        {
            return _instance == null;
        }
    }

    public void destroy()
    {
        //
        // First wait for shutdown to finish.
        //
        waitForShutdown();

        List<Ice.ObjectAdapter> adapters;
        lock (_mutex)
        {
            adapters = new List<Ice.ObjectAdapter>(_adapters);
        }

        foreach (Ice.ObjectAdapter adapter in adapters)
        {
            adapter.destroy();
        }

        lock (_mutex)
        {
            _adapters.Clear();
        }
    }

    public void
    updateConnectionObservers()
    {
        List<Ice.ObjectAdapter> adapters;
        lock (_mutex)
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
        lock (_mutex)
        {
            adapters = new List<Ice.ObjectAdapter>(_adapters);
        }

        foreach (Ice.ObjectAdapter adapter in adapters)
        {
            adapter.updateThreadObservers();
        }
    }

    public Ice.ObjectAdapter createObjectAdapter(
        string name,
        Ice.RouterPrx router,
        SslServerAuthenticationOptions serverAuthenticationOptions)
    {
        lock (_mutex)
        {
            if (_instance == null)
            {
                throw new Ice.CommunicatorDestroyedException();
            }

            if (name.Length > 0)
            {
                if (_adapterNamesInUse.Contains(name))
                {
                    throw new AlreadyRegisteredException("object adapter", name);
                }
                _adapterNamesInUse.Add(name);
            }
        }

        //
        // Must be called outside the synchronization since initialize can make client invocations
        // on the router if it's set.
        //
        Ice.ObjectAdapter adapter = null;
        try
        {
            if (name.Length == 0)
            {
                adapter = new Ice.ObjectAdapter(
                    _instance,
                    _communicator,
                    this,
                    System.Guid.NewGuid().ToString(),
                    null,
                    true,
                    serverAuthenticationOptions);
            }
            else
            {
                adapter = new Ice.ObjectAdapter(
                    _instance,
                    _communicator,
                    this,
                    name,
                    router,
                    false,
                    serverAuthenticationOptions);
            }

            lock (_mutex)
            {
                if (_instance == null)
                {
                    throw new Ice.CommunicatorDestroyedException();
                }
                _adapters.Add(adapter);
            }
        }
        catch (Ice.CommunicatorDestroyedException)
        {
            adapter?.destroy();
            throw;
        }
        catch (Ice.LocalException)
        {
            if (name.Length > 0)
            {
                lock (_mutex)
                {
                    _adapterNamesInUse.Remove(name);
                }
            }
            throw;
        }

        return adapter;
    }

    public Ice.ObjectAdapter findObjectAdapter(Reference reference)
    {
        List<Ice.ObjectAdapter> adapters;
        lock (_mutex)
        {
            if (_instance == null)
            {
                return null;
            }

            adapters = new List<Ice.ObjectAdapter>(_adapters);
        }

        foreach (Ice.ObjectAdapter adapter in adapters)
        {
            try
            {
                if (adapter.isLocal(reference))
                {
                    return adapter;
                }
            }
            catch (Ice.ObjectAdapterDestroyedException)
            {
                // Ignore.
            }
        }

        return null;
    }

    public void removeObjectAdapter(Ice.ObjectAdapter adapter)
    {
        lock (_mutex)
        {
            if (_instance == null)
            {
                return;
            }

            _adapters.Remove(adapter);
            _adapterNamesInUse.Remove(adapter.getName());
        }
    }

    public void flushAsyncBatchRequests(Ice.CompressBatch compressBatch, CommunicatorFlushBatchAsync outAsync)
    {
        List<Ice.ObjectAdapter> adapters;
        lock (_mutex)
        {
            adapters = new List<Ice.ObjectAdapter>(_adapters);
        }

        foreach (Ice.ObjectAdapter adapter in adapters)
        {
            adapter.flushAsyncBatchRequests(compressBatch, outAsync);
        }
    }

    //
    // Only for use by Instance.
    //
    internal ObjectAdapterFactory(Instance instance, Ice.Communicator communicator)
    {
        _instance = instance;
        _communicator = communicator;
        _adapterNamesInUse = new HashSet<string>();
        _adapters = new List<Ice.ObjectAdapter>();
    }

    private Instance _instance;
    private Ice.Communicator _communicator;
    private readonly HashSet<string> _adapterNamesInUse;
    private readonly List<Ice.ObjectAdapter> _adapters;
    private readonly object _mutex = new();
}
