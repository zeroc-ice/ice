// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class BetI extends CasinoStore.PersistentBet
{
    public int
    getAmount(Ice.Current current)
    {
        return amount;
    }

    public void
    accept(Casino.PlayerPrx p, Ice.Current current)
        throws Casino.OutOfChipsException
    {
        if(p == null)
        {
            throw new Casino.OutOfChipsException();
        }

        CasinoStore.PersistentPlayerPrx player = CasinoStore.PersistentPlayerPrxHelper.uncheckedCast(p);

        try
        {
            player.withdraw(amount);
            potentialWinners.add(player);
        }
        catch(Ice.ObjectNotExistException one)
        {
            throw new Casino.OutOfChipsException();
        }
    }

    public int
    getChipsInPlay(Ice.Current current)
    {
        return amount * potentialWinners.size();
    }

    public long
    getCloseTime(Ice.Current current)
    {
        return closeTime;
    }

    public void
    complete(int random, Ice.Current current)
    {
        if(random < 0)
        {
            random = -random;
        }

        int size = potentialWinners.size();

        //
        // Pick a winner using random
        //
        int winnerIndex = random % (size + (_bankEdge - 1));

        if(winnerIndex >= size)
        {
            winnerIndex = 0;
        }

        CasinoStore.WinnerPrx winner = potentialWinners.elementAt(winnerIndex);

        try
        {
            winner.win(amount * size);
        }
        catch(Ice.ObjectNotExistException ex)
        {
            //
            // Goes to the bank
            //
            winner = potentialWinners.elementAt(0);
            winner.win(amount * size);
        }

        //
        // Self-destroys
        //
        _evictor.remove(current.id);
    }

    BetI()
    {
    }

    BetI(int amount, long closeTime, CasinoStore.PersistentBankPrx bank, Freeze.TransactionalEvictor evictor,
         int bankEdge)
    {
        this.amount = amount;
        this.closeTime = closeTime;
        init(evictor, bankEdge);
        potentialWinners = new java.util.Vector<CasinoStore.WinnerPrx>();
        potentialWinners.add(bank);
    }

    void
    init(Freeze.TransactionalEvictor evictor, int bankEdge)
    {
        _evictor = evictor;
        _bankEdge = bankEdge;
    }

    private Freeze.TransactionalEvictor _evictor;
    private int _bankEdge;
}
