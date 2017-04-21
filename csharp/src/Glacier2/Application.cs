// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;

namespace Glacier2
{

/// <summary>
/// Utility base class that makes it easy to correctly initialize and finalize
/// the Ice run time, as well as handle signals. Unless the application specifies
/// a logger, Application installs a per-process logger that logs to the standard
/// error output.
///
/// Applications must create a derived class that implements the {@link #run} method.
///
/// A program can contain only one instance of this class.
/// </summary>
public abstract class Application : Ice.Application
{
    /// <summary>
    /// This exception is raised if the session should be restarted.
    /// </summary>
    public class RestartSessionException : Exception
    {
    }

    /// <summary>
    /// Initializes an instance that handles signals according to the signal policy.
    /// If not signal policy is provided the default SinalPolicy.HandleSignals 
    /// will be used, which calls Communicator.shutdown if a signal is received.
    /// </summary>
    /// <param name="signalPolicy">Determines how to respond to signals.</param>
    public Application(Ice.SignalPolicy signalPolicy = Ice.SignalPolicy.HandleSignals) : base(signalPolicy)
    {
    }

    /// <summary>
    /// Creates a new Glacier2 session. A call to createSession always
    /// precedes a call to runWithSession. If Ice.LocalException is thrown
    /// from this method, the application is terminated.
    /// </summary>
    /// <returns> The Glacier2 session.</returns>
    public abstract Glacier2.SessionPrx
    createSession();

    /// <summary>
    /// Called once the communicator has been initialized and the Glacier2 session
    /// has been established. A derived class must implement <code>runWithSession</code>,
    /// which is the application's starting method.
    /// </summary>
    /// <param name="args"> The argument vector for the application. Application
    /// scans the argument vector passed to <code>main</code> for options that are
    /// specific to the Ice run time and removes them; therefore, the vector passed
    /// to <code>runWithSession</code> is free from Ice-related options and contains
    /// only options and arguments that are application-specific.</param>
    ///
    /// <returns> The runWithSession method should return zero for successful
    /// termination, and non-zero otherwise. Application.main returns the
    /// value returned by runWithSession.</returns>
    ///
    public abstract int
    runWithSession(string[] args);

    /// <summary>
    /// Run should not be overridden for Glacier2.Application. Instead
    /// runWithSession should be used.
    /// </summary>
    public override int
    run(string[] args)
    {
        //
        // This shouldn't be called.
        //
        Debug.Assert(false);
        return 0;
    }

    /// <summary>
    /// Called when the session refresh thread detects that the session has been
    /// destroyed. A subclass can override this method to take action after the
    /// loss of connectivity with the Glacier2 router. This method is called
    /// according to the Ice invocation dipsatch rules (in other words, it
    /// uses the same rules as an servant upcall or AMI callback).
    /// </summary>
    public virtual void
    sessionDestroyed()
    {
    }

    /// <summary>
    /// Called to restart the application's Glacier2 session. This
    /// method never returns. The exception produce an application restart
    /// when called from the Application main thread.
    /// </summary>
    /// <returns>throws RestartSessionException This exception is
    /// always thrown.</returns>
    ///
    public static void
    restart()
    {
        throw new RestartSessionException();
    }

    /// <summary>
    /// Returns the Glacier2 router proxy.
    /// </summary>
    /// <returns>The router proxy.</returns>
    public static Glacier2.RouterPrx
    router()
    {
        return _router;
    }

    /// <summary>
    /// Returns the Glacier2 session proxy.
    /// </summary>
    /// <returns>The session proxy.</returns>
    public static SessionPrx
    session()
    {
        return _session;
    }

    /// <summary>
    /// Returns the category to be used in the identities of all of the client's
    /// callback objects. Clients must use this category for the router to
    /// forward callback requests to the intended client.
    /// Throws SessionNotExistException if no session exists.
    /// </summary>
    /// <returns>The category.</returns>
    public static string
    categoryForClient()
    {
        if(_router == null)
        {
            throw new SessionNotExistException();
        }
        return _category;
    }

    /// <summary>
    /// Create a new Ice identity for callback objects with the given
    /// identity name field.
    /// </summary>
    /// <returns>The identity.</returns>
    public static Ice.Identity
    createCallbackIdentity(string name)
    {
        return new Ice.Identity(name, categoryForClient());
    }

    /// <summary>
    /// Adds a servant to the callback object adapter's Active Servant Map with a UUID.
    /// </summary>
    /// <param name="servant">The servant to add.</param>
    /// <returns>The proxy for the servant.</returns>
    public static Ice.ObjectPrx
    addWithUUID(Ice.Object servant)
    {
        return objectAdapter().add(servant, createCallbackIdentity(Guid.NewGuid().ToString()));
    }

    /// <summary>
    /// Returns an object adapter for callback objects, creating it if necessary.
    /// </summary>
    /// <returns>The object adapter.</returns>
    public static Ice.ObjectAdapter
    objectAdapter()
    {
        if(_router == null)
        {
            throw new SessionNotExistException();
        }

        lock(iceMutex)
        {
            if(_adapter == null)
            {
                _adapter = communicator().createObjectAdapterWithRouter("", _router);
                _adapter.activate();
            }
        }
        return _adapter;
    }

    protected override int
    doMain(string[] originArgs, Ice.InitializationData initData)
    {
        //
        // Set the default properties for all Glacier2 applications.
        //
        initData.properties.setProperty("Ice.RetryIntervals", "-1");

        bool restart;
        int ret = 0;
        do
        {
            //
            // A copy of the initialization data and the string array
            // needs to be passed to doMainInternal, as these can be
            // changed by the application.
            //
            Ice.InitializationData id = (Ice.InitializationData)initData.Clone();
            id.properties = id.properties.ice_clone_();
            string[] args = (string[]) originArgs.Clone();

            restart = doMain(args, id, out ret);
        }
        while(restart);
        return ret;
    }

    private bool
    doMain(string[] args, Ice.InitializationData initData, out int status)
    {
        //
        // Reset internal state variables from Ice.Application. The
        // remainder are reset at the end of this method.
        //
        iceCallbackInProgress = false;
        iceDestroyed = false;
        iceInterrupted = false;

        bool restart = false;
        bool sessionCreated = false;
        status = 0;

        try
        {
            iceCommunicator = Ice.Util.initialize(ref args, initData);

            _router = RouterPrxHelper.uncheckedCast(communicator().getDefaultRouter());
            if(_router == null)
            {
                Ice.Util.getProcessLogger().error(iceAppName + ": no Glacier2 router configured");
                status = 1;
            }
            else
            {
                //
                // The default is to destroy when a signal is received.
                //
                if(iceSignalPolicy == Ice.SignalPolicy.HandleSignals)
                {
                    destroyOnInterrupt();
                }

                //
                // If createSession throws, we're done.
                //
                try
                {
                    _session = createSession();
                    sessionCreated = true;
                }
                catch(Ice.LocalException ex)
                {
                    Ice.Util.getProcessLogger().error(ex.ToString());
                    status = 1;
                }

                if(sessionCreated)
                {
                    int acmTimeout = 0;
                    try
                    {
                        acmTimeout = _router.getACMTimeout();
                    }
                    catch(Ice.OperationNotExistException)
                    {
                    }
                    if(acmTimeout <= 0)
                    {
                        acmTimeout = (int)_router.getSessionTimeout();
                    }
                    if(acmTimeout > 0)
                    {
                        Ice.Connection connection = _router.ice_getCachedConnection();
                        Debug.Assert(connection != null);
                        connection.setACM(acmTimeout, Ice.Util.None, Ice.ACMHeartbeat.HeartbeatAlways);
                        connection.setCloseCallback(_ => sessionDestroyed());
                    }
                    _category = _router.getCategoryForClient();
                    status = runWithSession(args);
                }
            }
        }
        //
        // We want to restart on those exceptions that indicate a
        // break down in communications, but not those exceptions that
        // indicate a programming logic error (i.e., marshal, protocol
        // failure, etc).
        //
        catch(RestartSessionException)
        {
            restart = true;
        }
        catch(Ice.ConnectionRefusedException ex)
        {
            Ice.Util.getProcessLogger().error(ex.ToString());
            restart = true;
        }
        catch(Ice.ConnectionLostException ex)
        {
            Ice.Util.getProcessLogger().error(ex.ToString());
            restart = true;
        }
        catch(Ice.UnknownLocalException ex)
        {
            Ice.Util.getProcessLogger().error(ex.ToString());
            restart = true;
        }
        catch(Ice.RequestFailedException ex)
        {
            Ice.Util.getProcessLogger().error(ex.ToString());
            restart = true;
        }
        catch(Ice.TimeoutException ex)
        {
            Ice.Util.getProcessLogger().error(ex.ToString());
            restart = true;
        }
        catch(Ice.LocalException ex)
        {
            Ice.Util.getProcessLogger().error(ex.ToString());
            status = 1;
        }
        catch(Exception ex)
        {
            Ice.Util.getProcessLogger().error("unknown exception:\n" + ex.ToString());
            status = 1;
        }

        //
        // Don't want any new interrupt. And at this point
        // (post-run), it would not make sense to release a held
        // signal to run shutdown or destroy.
        //
        if(iceSignalPolicy == Ice.SignalPolicy.HandleSignals)
        {
            ignoreInterrupt();
        }

        lock(iceMutex)
        {
            while(iceCallbackInProgress)
            {
                System.Threading.Monitor.Wait(iceMutex);
            }

            if(iceDestroyed)
            {
                iceCommunicator = null;
            }
            else
            {
                iceDestroyed = true;
                //
                // And iceCommunicator != null, meaning will be
                // destroyed next, iceDestroyed = true also ensures that
                // any remaining callback won't do anything
                //
            }
        }

        if(sessionCreated && _router != null)
        {
            try
            {
                _router.destroySession();
            }
            catch(Ice.ConnectionLostException)
            {
                //
                // Expected if another thread invoked on an object from the session concurrently.
                //
            }
            catch(SessionNotExistException)
            {
                //
                // This can also occur.
                //
            }
            catch(Exception ex)
            {
                //
                // Not expected.
                //
                Ice.Util.getProcessLogger().error("unexpected exception when destroying the session:\n" +
                                                  ex.ToString());
            }
            _router = null;
        }

        if(iceCommunicator != null)
        {
            try
            {
                iceCommunicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                Ice.Util.getProcessLogger().error(ex.ToString());
                status = 1;
            }
            catch(Exception ex)
            {
                Ice.Util.getProcessLogger().error("unknown exception:\n" + ex.ToString());
                status = 1;
            }
            iceCommunicator = null;
        }

        //
        // Reset internal state. We cannot reset the Application state
        // here, since iceDestroyed must remain true until we re-run
        // this method.
        //
        _adapter = null;
        _router = null;
        _session = null;
        _category = null;

        return restart;
    }

    private static Ice.ObjectAdapter _adapter;
    private static RouterPrx _router;
    private static SessionPrx _session;
    private static string _category;
}

}
