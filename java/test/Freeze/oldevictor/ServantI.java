// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class ServantI implements Test._ServantOperations
{
    static class DelayedResponse extends Thread
    {   
        DelayedResponse(Test.AMD_Servant_slowGetValue cb, int val)
        {
            _cb = cb;
            _val = val;
        }
        
        public void
        run()
        { 
            try
            {
                sleep(500);
            }
            catch(InterruptedException e)
            {
                // Ignored
            }
            _cb.ice_response(_val);
        }

        private Test.AMD_Servant_slowGetValue _cb;
        private int _val;
    }


    ServantI(Test.Servant tie)
    {
        _tie = tie;
    }

    ServantI(Test.Servant tie, RemoteEvictorI remoteEvictor, Freeze.BackgroundSaveEvictor evictor, int value)
    {
        _tie = tie;
        _remoteEvictor = remoteEvictor;
        _evictor = evictor;
        _tie.value = value;
    }

    void
    init(RemoteEvictorI remoteEvictor, Freeze.BackgroundSaveEvictor evictor)
    {   
        _remoteEvictor = remoteEvictor;
        _evictor = evictor;
    }

    public void
    destroy(Ice.Current current)
    {
        _evictor.remove(current.id);
    }

    public int
    getValue(Ice.Current current)
    {
        synchronized(_tie)
        {
            return _tie.value;
        }
    }
    
    public void
    slowGetValue_async(Test.AMD_Servant_slowGetValue cb, Ice.Current current)
    {
        synchronized(_tie)
        {
            Thread t = new DelayedResponse(cb, _tie.value);
            t.setDaemon(true);
            t.start();
        }
    }

    public void
    setValue(int value, Ice.Current current)
    {
        synchronized(_tie)
        {
            _tie.value = value;
        }
    }

    public void
    setValueAsync_async(Test.AMD_Servant_setValueAsync __cb, int value, Ice.Current current)
    {
        synchronized(_tie)
        {
            _setValueAsyncCB = __cb;
            _setValueAsyncValue = value;
            _tie.notify();
        }
    }

    public void
    releaseAsync(Ice.Current current)
    {
        synchronized(_tie)
        {
            while(_setValueAsyncCB == null)
            {
                try
                {
                    _tie.wait();
                }
                catch(InterruptedException ie)
                {
                    break;
                }
            }

            _tie.value = _setValueAsyncValue;
            _setValueAsyncCB.ice_response();
            _setValueAsyncCB = null;
        }
    }

    public void
    addFacet(String name, String data, Ice.Current current)
        throws Test.AlreadyRegisteredException
    {
        Test._FacetTie tie = new Test._FacetTie();
        tie.ice_delegate(new FacetI(tie, _remoteEvictor, _evictor, 0, data));

        try
        {
            _evictor.addFacet(tie, current.id, name);
        }
        catch(Ice.AlreadyRegisteredException ex)
        {
            throw new Test.AlreadyRegisteredException();
        }
    }

    public void
    removeFacet(String name, Ice.Current current)
        throws Test.NotRegisteredException
    {
        try
        {
            _evictor.removeFacet(current.id, name);
        }
        catch(Ice.NotRegisteredException ex)
        {
            throw new Test.NotRegisteredException();
        }
   
    }

    public synchronized int
    getTransientValue(Ice.Current current)
    {
        return _transientValue;
    }

    public synchronized void
    setTransientValue(int val, Ice.Current current)
    {
        _transientValue = val;
    }

    public void
    keepInCache(Ice.Current current)
    {
        _evictor.keep(current.id);
    }

    public void
    release(Ice.Current current)
        throws Test.NotRegisteredException
    {
        try
        {
            _evictor.release(current.id);
        }
        catch(Ice.NotRegisteredException e)
        {
            throw new Test.NotRegisteredException();
        }
    }

    protected RemoteEvictorI _remoteEvictor;
    protected Freeze.BackgroundSaveEvictor _evictor;
    protected Test.AMD_Servant_setValueAsync _setValueAsyncCB;
    protected int _setValueAsyncValue;
    protected Test.Servant _tie;
    private int _transientValue = -1;
}
