// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public abstract class BatchOutgoingAsync implements OutgoingAsyncMessageCallback
{
    public
    BatchOutgoingAsync()
    {
    }

    public abstract void ice_exception(Ice.LocalException ex);

    public final BasicStream
    __os()
    {
        return __os;
    }

    public final void
    __sent(final Ice.ConnectionI connection)
    {
	synchronized(_monitor)
	{
            cleanup();
        }
    }
    
    public final void
    __finished(Ice.LocalException exc)
    {
        try
        {
            ice_exception(exc);
        }
        catch(java.lang.Exception ex)
        {
            warning(ex);
        }
        finally
        {
            synchronized(_monitor)
            {
		cleanup();
            }
        }
    }

    protected final void
    __prepare(Instance instance)
    {
        synchronized(_monitor)
        {
            while(__os != null)
            {
                try
                {
                    _monitor.wait();
                }
                catch(InterruptedException ex)
                {
                }
            }
            
            assert(__os == null);
            __os = new BasicStream(instance);   
        }
    }

    private final void
    warning(java.lang.Exception ex)
    {
        if(__os != null) // Don't print anything if cleanup() was already called.
        {
            if(__os.instance().initializationData().properties.getPropertyAsIntWithDefault(
                   "Ice.Warn.AMICallback", 1) > 0)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                IceUtil.OutputBase out = new IceUtil.OutputBase(pw);
                out.setUseTab(false);
                out.print("exception raised by AMI callback:\n");
                ex.printStackTrace(pw);
                pw.flush();
                __os.instance().initializationData().logger.warning(sw.toString());
            }
        }
    }

    private final void
    cleanup()
    {
        __os = null;
        _monitor.notify();
    }

    protected BasicStream __os;
    private final java.lang.Object _monitor = new java.lang.Object();
}
