// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class AccountI extends Test.Account
{
    public int
    getBalance(Ice.Current current)
    {
        return balance;
    }

    public void
    deposit(int amount, Ice.Current current) throws Test.InsufficientFundsException
    {
        test(_evictor.getCurrentTransaction() != null);

        //
        // No need to synchronize since everything occurs within its own transaction
        //
        int newBalance = balance + amount;
        if(newBalance < 0)
        {
            throw new Test.InsufficientFundsException();
        }
        balance = newBalance;
    }

    public void
    transfer(int amount, Test.AccountPrx toAccount, Ice.Current current) throws Test.InsufficientFundsException
    {
        test(_evictor.getCurrentTransaction() != null);

        toAccount.deposit(amount); // collocated call
        deposit(-amount, current); // direct call
    }

    public AccountI(int initialBalance, Freeze.TransactionalEvictor evictor)
    {
        super(initialBalance);
        _evictor = evictor;
    }

    public AccountI()
    {
    }

    public void init(Freeze.TransactionalEvictor evictor)
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
