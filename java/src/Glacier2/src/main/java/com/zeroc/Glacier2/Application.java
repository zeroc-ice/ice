// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Glacier2;

import com.zeroc.Ice.Util;
import com.zeroc.Ice.ACMHeartbeat;
import com.zeroc.IceInternal.Ex;

/**
 * An extension of com.zeroc.Ice.Application that makes it easy to write
 * Glacier2 applications.
 *
 * <p> Applications must create a derived class that implements the
 * {@link #createSession} and {@link #runWithSession} methods.<p>
 *
 * The base class invokes {@link #createSession} to create a new
 * Glacier2 session and then invokes {@link #runWithSession} in
 * which the subclass performs its application logic. The base class
 * automatically destroys the session when {@link #runWithSession}
 * returns.
 *
 * If {@link #runWithSession} calls {@link #restart} or raises any of
 * the exceptions com.zeroc.Ice.ConnectionRefusedException,
 * com.zeroc.Ice.ConnectionLostException, com.zeroc.Ice.UnknownLocalException,
 * com.zeroc.Ice.RequestFailedException, or com.zeroc.Ice.TimeoutException, the base
 * class destroys the current session and restarts the application
 * with another call to {@link #createSession} followed by
 * {@link #runWithSession}.
 *
 * The application can optionally override the {@link #sessionDestroyed}
 * callback method if it needs to take action when connectivity with
 * the Glacier2 router is lost.
 *
 * A program can contain only one instance of this class.
 *
 * @see com.zeroc.Ice.Application
 * @see com.zeroc.Glacier2.Router
 * @see com.zeroc.Glacier2.Session
 * @see com.zeroc.Ice.Communicator
 * @see com.zeroc.Ice.Logger
 * @see #runWithSession
 **/
public abstract class Application extends com.zeroc.Ice.Application
{
    /**
     * This exception is raised if the session should be restarted.
     */
    public class RestartSessionException extends Exception
    {
    }

    /**
     * Initializes an instance that calls {@link Ice.Communicator#shutdown}
     * if a signal is received.
     **/
    public Application()
    {
    }

    /**
     * Initializes an instance that handles signals according to the signal
     * policy.
     *
     * @param signalPolicy Determines how to respond to signals.
     *
     * @see Ice.SignalPolicy
     **/
    public Application(com.zeroc.Ice.SignalPolicy signalPolicy)
    {
        super(signalPolicy);
    }

    /**
     * Called once the communicator has been initialized and the Glacier2 session
     * has been established. A derived class must implement <code>runWithSession</code>,
     * which is the application's starting method.
     *
     * @param args The argument vector for the application. <code>Application</code>
     * scans the argument vector passed to <code>main</code> for options that are
     * specific to the Ice run time and removes them; therefore, the vector passed
     * to <code>run</code> is free from Ice-related options and contains only options
     * and arguments that are application-specific.
     *
     * @return The <code>runWithSession</code> method should return zero for successful
     * termination, and non-zero otherwise. <code>Application.main</code> returns the
     * value returned by <code>runWithSession</code>.
     *
     * @throws RestartSessionException If the session should be restarted.
     **/
    public abstract int runWithSession(String[] args)
        throws RestartSessionException;

    /**
     * Run should not be overridden for com.zeroc.Glacier2.Application. Instead
     * <code>runWithSession</code> should be used.
     */
    @Override
    final public int run(String[] args)
    {
        // This shouldn't be called.
        assert false;
        return 0;
    }

    /**
     * Called to restart the application's Glacier2 session. This
     * method never returns. The exception produce an application restart
     * when called from the Application main thread.
     *
     * @throws RestartSessionException This exception is always thrown.
     **/
    public void restart()
        throws RestartSessionException
    {
        throw new RestartSessionException();
    }

    /**
     * Creates a new Glacier2 session. A call to
     * <code>createSession</code> always precedes a call to
     * <code>runWithSession</code>. If <code>com.zeroc.Ice.LocalException</code>
     * is thrown from this method, the application is terminated.
     *
     * @return The Glacier2 session.
     **/
    abstract public com.zeroc.Glacier2.SessionPrx createSession();

    /**
     * Called when the session refresh thread detects that the session has been
     * destroyed. A subclass can override this method to take action after the
     * loss of connectivity with the Glacier2 router. This method is called
     * according to the Ice invocation dipsatch rules (in other words, it
     * uses the same rules as an servant upcall or AMI callback).
     **/
    public void sessionDestroyed()
    {
    }

    /**
     * Returns the Glacier2 router proxy
     * @return The router proxy.
     **/
    public static com.zeroc.Glacier2.RouterPrx router()
    {
        return _router;
    }

    /**
     * Returns the Glacier2 session proxy
     * @return The session proxy.
     **/
    public static com.zeroc.Glacier2.SessionPrx session()
    {
        return _session;
    }

    /**
     * Returns the category to be used in the identities of all of the client's
     * callback objects. Clients must use this category for the router to
     * forward callback requests to the intended client.
     * @return The category.
     * @throws SessionNotExistException No session exists.
     **/
    public String categoryForClient()
        throws SessionNotExistException
    {
        if(_router == null)
        {
            throw new SessionNotExistException();
        }
        return _category;
    }

    /**
     * Create a new Ice identity for callback objects with the given identity name field.
     * @param name The identity name.
     * @return The identity with the given name and a unique category.
     * @throws SessionNotExistException No session exists.
     **/
    public com.zeroc.Ice.Identity createCallbackIdentity(String name)
        throws SessionNotExistException
    {
        return new com.zeroc.Ice.Identity(name, categoryForClient());
    }

    /**
     * Adds a servant to the callback object adapter's Active Servant Map with a UUID.
     * @param servant The servant to add.
     * @return The proxy for the servant.
     * @throws SessionNotExistException No session exists.
     **/
    public com.zeroc.Ice.ObjectPrx addWithUUID(com.zeroc.Ice.Object servant)
        throws SessionNotExistException
    {
        return objectAdapter().add(servant, createCallbackIdentity(java.util.UUID.randomUUID().toString()));
    }

    /**
     * Creates an object adapter for callback objects.
     * @return The object adapter.
     * @throws SessionNotExistException No session exists.
     */
    public com.zeroc.Ice.ObjectAdapter objectAdapter()
        throws SessionNotExistException
    {
        if(_router == null)
        {
            throw new SessionNotExistException();
        }

        synchronized(this)
        {
            if(_adapter == null)
            {
                _adapter = communicator().createObjectAdapterWithRouter("", _router);
                _adapter.activate();
            }
        }
        return _adapter;
    }

    private static class DoMainResult
    {
        int returnValue;
        boolean restart;
    }

    @Override
    protected int doMain(String[] args, com.zeroc.Ice.InitializationData initData)
    {
        //
        // Set the default properties for all Glacier2 applications.
        //
        initData.properties.setProperty("Ice.RetryIntervals", "-1");

        DoMainResult r;
        do
        {
            //
            // A copy of the initialization data and the string array
            // needs to be passed to doMainInternal, as these can be
            // changed by the application.
            //
            com.zeroc.Ice.InitializationData id = initData.clone();
            id.properties = id.properties._clone();
            String[] a = args.clone();

            r = doMainInternal(a, id);
        }
        while(r.restart);
        return r.returnValue;
    }

    private DoMainResult doMainInternal(String[] args, com.zeroc.Ice.InitializationData initData)
    {
        //
        // Reset internal state variables from com.zeroc.Ice.Application. The
        // remainder are reset at the end of this method.
        //
        _callbackInProgress = false;
        _destroyed = false;
        _interrupted = false;

        DoMainResult r = new DoMainResult();
        r.restart = false;
        r.returnValue = 0;

        try
        {
            java.util.List<String> remainingArgs = new java.util.ArrayList<>();
            _communicator = Util.initialize(args, initData, remainingArgs);

            _router = com.zeroc.Glacier2.RouterPrx.uncheckedCast(communicator().getDefaultRouter());
            if(_router == null)
            {
                Util.getProcessLogger().error("no glacier2 router configured");
                r.returnValue = 1;
            }
            else
            {
                //
                // The default is to destroy when a signal is received.
                //
                if(_signalPolicy == com.zeroc.Ice.SignalPolicy.HandleSignals)
                {
                    destroyOnInterrupt();
                }

                //
                // If createSession throws, we're done.
                //
                try
                {
                    _session = createSession();
                    _createdSession = true;
                }
                catch(com.zeroc.Ice.LocalException ex)
                {
                    Util.getProcessLogger().error(Ex.toString(ex));
                    r.returnValue = 1;
                }

                if(_createdSession)
                {
                    int acmTimeout = 0;
                    try
                    {
                        acmTimeout = _router.getACMTimeout();
                    }
                    catch(com.zeroc.Ice.OperationNotExistException ex)
                    {
                    }
                    if(acmTimeout <= 0)
                    {
                        acmTimeout = (int)_router.getSessionTimeout();
                    }
                    if(acmTimeout > 0)
                    {
                        com.zeroc.Ice.Connection connection = _router.ice_getCachedConnection();
                        assert(connection != null);
                        connection.setACM(
                            java.util.OptionalInt.of(acmTimeout), null,
                            java.util.Optional.of(ACMHeartbeat.HeartbeatAlways));
                        connection.setCloseCallback(con -> sessionDestroyed());
                    }
                    _category = _router.getCategoryForClient();
                    r.returnValue = runWithSession(remainingArgs.toArray(new String[remainingArgs.size()]));
                }
            }
        }
        //
        // We want to restart on those exceptions that indicate a
        // break down in communications, but not those exceptions that
        // indicate a programming logic error (i.e., marshal, protocol
        // failure, etc).
        //
        catch(RestartSessionException ex)
        {
            r.restart = true;
        }
        catch(com.zeroc.Ice.ConnectionRefusedException ex)
        {
            Util.getProcessLogger().error(Ex.toString(ex));
            r.restart = true;
        }
        catch(com.zeroc.Ice.ConnectionLostException ex)
        {
            Util.getProcessLogger().error(Ex.toString(ex));
            r.restart = true;
        }
        catch(com.zeroc.Ice.UnknownLocalException ex)
        {
            Util.getProcessLogger().error(Ex.toString(ex));
            r.restart = true;
        }
        catch(com.zeroc.Ice.RequestFailedException ex)
        {
            Util.getProcessLogger().error(Ex.toString(ex));
            r.restart = true;
        }
        catch(com.zeroc.Ice.TimeoutException ex)
        {
            Util.getProcessLogger().error(Ex.toString(ex));
            r.restart = true;
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            Util.getProcessLogger().error(Ex.toString(ex));
            r.returnValue = 1;
        }
        catch(java.lang.Exception ex)
        {
            Util.getProcessLogger().error("unknown exception:\n" + Ex.toString(ex));
            r.returnValue = 1;
        }
        catch(java.lang.Error err)
        {
            //
            // We catch Error to avoid hangs in some non-fatal situations
            //
            Util.getProcessLogger().error("Java error:\n" + Ex.toString(err));
            r.returnValue = 1;
        }

        //
        // This clears any set interrupt.
        //
        if(_signalPolicy == com.zeroc.Ice.SignalPolicy.HandleSignals)
        {
            defaultInterrupt();
        }

        synchronized(_mutex)
        {
            while(_callbackInProgress)
            {
                try
                {
                    _mutex.wait();
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }

            if(_destroyed)
            {
                _communicator = null;
            }
            else
            {
                _destroyed = true;
                //
                // And _communicator != null, meaning will be
                // destroyed next, _destroyed = true also ensures that
                // any remaining callback won't do anything.
                //
            }
        }

        if(_createdSession && _router != null)
        {
            try
            {
                _router.destroySession();
            }
            catch(com.zeroc.Ice.ConnectionLostException ex)
            {
                //
                // Expected if another thread invoked on an object from the session concurrently.
                //
            }
            catch(com.zeroc.Glacier2.SessionNotExistException ex)
            {
                //
                // This can also occur.
                //
            }
            catch(Throwable ex)
            {
                //
                // Not expected.
                //
                Util.getProcessLogger().error("unexpected exception when destroying the session:\n" + Ex.toString(ex));
            }
            _router = null;
        }

        if(_communicator != null)
        {
            _communicator.destroy();
            _communicator = null;
        }

        synchronized(_mutex)
        {
            if(_appHook != null)
            {
                _appHook.done();
            }
        }

        //
        // Reset internal state. We cannot reset the Application state
        // here, since _destroyed must remain true until we re-run
        // this method.
        //
        _adapter = null;
        _router = null;
        _session = null;
        _createdSession = false;
        _category = null;

        return r;
    }

    private static com.zeroc.Ice.ObjectAdapter _adapter;
    private static com.zeroc.Glacier2.RouterPrx _router;
    private static com.zeroc.Glacier2.SessionPrx _session;
    private static boolean _createdSession = false;
    private static String _category;
}
