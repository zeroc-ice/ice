// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

abstract public class OutgoingAsyncMessageCallback
{
    public abstract void __sent(Ice.ConnectionI connection);
    public abstract void __finished(Ice.LocalException ex);
    public abstract void ice_exception(Ice.LocalException ex);

    public final BasicStream
    __os()
    {
        return __os;
    }

    public void
    __sent(Instance instance)
    {
        try
        {
            ((Ice.AMISentCallback)this).ice_sent();
        }
        catch(java.lang.Exception ex)
        {
            __warning(instance, ex);
        }
    }

    public void
    __exception(Ice.LocalException exc)
    {
        try
        {
            ice_exception(exc);
        }
        catch(java.lang.Exception ex)
        {
            __warning(ex);
        }
        finally
        {
            __releaseCallback();
        }
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        assert(__os == null);
        assert(__is == null);
    }

    protected void
    __acquireCallback(Ice.ObjectPrx proxy)
    {
        synchronized(__monitor)
        {
            //
            // We must first wait for other requests to finish.
            //
            while(__os != null)
            {
                try
                {
                    __monitor.wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            Reference ref = ((Ice.ObjectPrxHelperBase)proxy).__reference();
            assert(__is == null);
            __is = new BasicStream(ref.getInstance());
            assert(__os == null);
            __os = new BasicStream(ref.getInstance());              
        }
    }

    protected void
    __releaseCallback(final Ice.LocalException ex)
    {
        synchronized(__monitor)
        {
            assert(__os != null);

            //
            // This is called by the invoking thread to release the callback following a direct 
            // failure to marhsall/send the request. We call the ice_exception() callback with
            // the thread pool to avoid potential deadlocks in case the invoking thread locked 
            // some mutexes/resources (which couldn't be re-acquired by the callback).
            //

            try
            {
                __os.instance().clientThreadPool().execute(new ThreadPoolWorkItem()
                    {
                        public void
                        execute(ThreadPoolCurrent current)
                        {
                            current.ioCompleted();
                            __exception(ex);
                        }
                    });
            }
            catch(Ice.CommunicatorDestroyedException exc)
            {
                __releaseCallback();
                throw exc; // CommunicatorDestroyedException is the only exception that can propagate directly.
            }
        }
    }

    protected void
    __releaseCallback()
    {
        synchronized(__monitor)
        {
            assert(__is != null);
            __is = null;
            
            assert(__os != null);
            __os = null;
            
            __monitor.notify();
        }
    }

    protected void
    __warning(java.lang.Exception ex)
    {
        if(__os != null)
        {
            __warning(__os.instance(), ex);
        }
    }

    protected void
    __warning(Instance instance, java.lang.Exception ex)
    {
        if(instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.AMICallback", 1) > 0)
        {
            String s = "exception raised by AMI callback:\n" + Ex.toString(ex);
            instance.initializationData().logger.warning(s);
        }
    }

    protected final java.lang.Object __monitor = new java.lang.Object();
    protected BasicStream __is;
    protected BasicStream __os;
};
