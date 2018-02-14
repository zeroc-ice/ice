// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.evictor;
import test.Freeze.evictor.Test.*;

public class ServantI implements _ServantOperations
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static class DelayedResponse extends Thread
    {
        DelayedResponse(AMD_Servant_slowGetValue cb, int val)
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

        private AMD_Servant_slowGetValue _cb;
        private int _val;
    }

    ServantI(Servant tie)
    {
        _tie = tie;
    }

    ServantI(Servant tie, RemoteEvictorI remoteEvictor, Freeze.Evictor evictor, int value)
    {
        _tie = tie;
        _remoteEvictor = remoteEvictor;
        _evictor = evictor;
        _tie.value = value;
    }

    void
    init(RemoteEvictorI remoteEvictor, Freeze.Evictor evictor)
    {
        _remoteEvictor = remoteEvictor;
        _evictor = evictor;
    }

    public void
    destroy(Ice.Current current)
    {
        _evictor.remove(current.id);

        //
        // Need to remove the accounts as well!
        //
        if(_tie.accounts != null)
        {
            for(Ice.Identity id : _tie.accounts)
            {
                _evictor.remove(id);
            }
        }
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
    slowGetValue_async(AMD_Servant_slowGetValue cb, Ice.Current current)
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
    setValueAsync_async(AMD_Servant_setValueAsync __cb, int value, Ice.Current current)
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
        throws AlreadyRegisteredException
    {
        _FacetTie tie = new _FacetTie();
        tie.ice_delegate(new FacetI(tie, _remoteEvictor, _evictor, 0, data));

        try
        {
            _evictor.addFacet(tie, current.id, name);
        }
        catch(Ice.AlreadyRegisteredException ex)
        {
            throw new AlreadyRegisteredException();
        }
    }

    public void
    removeFacet(String name, Ice.Current current)
        throws NotRegisteredException
    {
        try
        {
            _evictor.removeFacet(current.id, name);
        }
        catch(Ice.NotRegisteredException ex)
        {
            throw new NotRegisteredException();
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
        Freeze.BackgroundSaveEvictor bse = (Freeze.BackgroundSaveEvictor)_evictor;
        bse.keep(current.id);
    }

    public void
    release(Ice.Current current)
        throws NotRegisteredException
    {
        Freeze.BackgroundSaveEvictor bse = (Freeze.BackgroundSaveEvictor)_evictor;

        try
        {
            bse.release(current.id);
        }
        catch(Ice.NotRegisteredException e)
        {
            throw new NotRegisteredException();
        }
    }

    public AccountPrx[]
    getAccounts(Ice.Current current)
    {
        Freeze.TransactionalEvictor te = (Freeze.TransactionalEvictor)_evictor;

        if(te.getCurrentTransaction() != null)
        {
            if(_tie.accounts == null || _tie.accounts.length == 0)
            {
                _tie.accounts = new Ice.Identity[10];

                for(int i = 0; i < _tie.accounts.length; ++i)
                {
                    _tie.accounts[i] = new Ice.Identity(current.id.name + "-account#" + i, current.id.category);
                    _evictor.add(new AccountI(1000, te), _tie.accounts[i]);
                }
            }
            else
            {
                te.getCurrentTransaction().rollback(); // not need to re-write this servant
            }
        }

        if(_tie.accounts == null || _tie.accounts.length == 0)
        {
            return new AccountPrx[0];
        }

        AccountPrx[] result = new AccountPrx[_tie.accounts.length];

        for(int i = 0; i < _tie.accounts.length; ++i)
        {
            result[i] = AccountPrxHelper.uncheckedCast(current.adapter.createProxy(_tie.accounts[i]));
        }
        return result;
    }

    public int
    getTotalBalance(Ice.Current current)
    {
        AccountPrx[] accounts = getAccounts(current);

        //
        // Need to start a transaction to ensure a consistent result
        //
        Freeze.TransactionalEvictor te = (Freeze.TransactionalEvictor)_evictor;

        for(;;)
        {
            test(te.getCurrentTransaction() == null);
            Freeze.Connection con =
                Freeze.Util.createConnection(current.adapter.getCommunicator(), _remoteEvictor.envName());
            te.setCurrentTransaction(con.beginTransaction());
            int total = 0;
            try
            {
                for(AccountPrx account : accounts)
                {
                    total += account.getBalance();
                }
                return total;
            }
            catch(Freeze.TransactionalEvictorDeadlockException e)
            {
                // retry
            }
            finally
            {
                con.close();
                te.setCurrentTransaction(null);
            }
        }
    }

    protected RemoteEvictorI _remoteEvictor;
    protected Freeze.Evictor _evictor;
    protected AMD_Servant_setValueAsync _setValueAsyncCB;
    protected int _setValueAsyncValue;
    protected Servant _tie;
    private int _transientValue = -1;
}
