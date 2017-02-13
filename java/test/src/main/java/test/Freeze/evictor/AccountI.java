// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Freeze.evictor;
import test.Freeze.evictor.Test.*;

public class AccountI extends Account
{
    public int
    getBalance(Ice.Current current)
    {
        return balance;
    }

    public void
    deposit(int amount, Ice.Current current)
        throws InsufficientFundsException
    {
        //
        // No need to synchronize since everything occurs within its own transaction
        //
        int newBalance = balance + amount;
        if(newBalance < 0)
        {
            throw new InsufficientFundsException();
        }
        balance = newBalance;
    }

    public void
    transfer(int amount, AccountPrx toAccount, Ice.Current current) throws InsufficientFundsException
    {
        test(_evictor.getCurrentTransaction() != null);

        toAccount.deposit(amount); // collocated call
        deposit(-amount, current); // direct call
    }

    public void
    transfer2_async(AMD_Account_transfer2 cb, int amount, AccountPrx toAccount, Ice.Current current)
    {
        //
        // Here the dispatch thread does everything
        //
        test(_evictor.getCurrentTransaction() != null);

        try
        {
            toAccount.deposit(amount); // collocated call
            deposit(-amount, current); // direct call
        }
        catch(InsufficientFundsException ex)
        {
            cb.ice_exception(ex);
            return;
        }

        cb.ice_response();
    }

    public void
    transfer3_async(final AMD_Account_transfer3 cb, int amount, AccountPrx toAccount, Ice.Current current)
    {
        //
        // Here the dispatch thread does the actual work, but a separate thread sends the response
        //

        class ResponseThread extends Thread
        {
            synchronized void
            response()
            {
                _response = true;
                notify();
            }

            synchronized void
            exception(Ice.UserException e)
            {
                _exception = e;
                notify();
            }

            public synchronized void
            run()
            {
                if(_response == false && _exception == null)
                {
                    try
                    {
                        wait(1000);
                    }
                    catch(InterruptedException e)
                    {
                    }
                }
                try
                {
                    test(_evictor.getCurrentTransaction() == null);
                }
                catch(Freeze.EvictorDeactivatedException ex)
                {
                    //
                    // Clearly nobody is waiting for a response!
                    //
                    return;
                }

                if(_response)
                {
                    cb.ice_response();
                }
                else if(_exception != null)
                {
                    cb.ice_exception(_exception);
                }
                else
                {
                    //
                    // We don't wait forever!
                    //
                    cb.ice_exception(new Ice.TimeoutException());
                }
            }

            private boolean _response = false;
            private Ice.UserException _exception;
        };

        ResponseThread thread = new ResponseThread();
        thread.setDaemon(true);

        test(_evictor.getCurrentTransaction() != null);

        try
        {
            toAccount.deposit(amount); // collocated call
            deposit(-amount, current); // direct call
        }
        catch(Ice.UserException e)
        {
            thread.start();
            Thread.yield();

            //
            // Need to rollback here -- "rollback on user exception" does not work
            // when the dispatch commits before it gets any response!
            //
            _evictor.getCurrentTransaction().rollback();

            thread.exception(e);
            return;
        }

        thread.start();
        Thread.yield();
        thread.response();
    }

    public
    AccountI(int initialBalance, Freeze.TransactionalEvictor evictor)
    {
        super(initialBalance);
        _evictor = evictor;
    }

    public
    AccountI()
    {
    }

    public void
    init(Freeze.TransactionalEvictor evictor)
    {
        assert _evictor == null;
        _evictor = evictor;
    }

    private Freeze.TransactionalEvictor _evictor;

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }
}
