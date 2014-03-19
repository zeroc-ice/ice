// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class PlayerI extends CasinoStore.PersistentPlayer
{
    public int
    getChips(Ice.Current current)
    {
        return chips;
    }

    public void
    destroy(Ice.Current current)
    {
        _bank.returnAllChips(_myPrx);
        _evictor.remove(current.id);
    }

    public void
    withdraw(int count, Ice.Current current)
        throws Casino.OutOfChipsException
    {
        int newBalance = chips - count;
        if(newBalance < 0)
        {
            throw new Casino.OutOfChipsException();
        }
        chips = newBalance;
    }

    public void
    win(int count, Ice.Current current)
    {
        assert count >= 0;
        chips += count;
    }

    void
    init(CasinoStore.PersistentPlayerPrx myPrx, Freeze.TransactionalEvictor evictor,
         CasinoStore.PersistentBankPrx bank)
    {
        _myPrx = myPrx;
        _evictor = evictor;
        _bank = bank;
    }

    private CasinoStore.PersistentPlayerPrx _myPrx;
    private Freeze.TransactionalEvictor _evictor;
    private CasinoStore.PersistentBankPrx _bank;
}
