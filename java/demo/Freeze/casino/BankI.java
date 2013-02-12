// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class BankI extends CasinoStore.PersistentBank
{
    public boolean
    buyChips(int chips, Casino.PlayerPrx p, Ice.Current current)
    {
        outstandingChips += chips;
        chipsSold += chips;

        CasinoStore.PersistentPlayerPrx player = CasinoStore.PersistentPlayerPrxHelper.uncheckedCast(p);

        try
        {
            player.win(chips);
        }
        catch(Ice.ObjectNotExistException one)
        {
            _evictor.getCurrentTransaction().rollback();
            return false;
        }

        return true;
    }

    public Casino.PlayerPrx[]
    getPlayers(Ice.Current current)
    {
        java.util.Vector<Casino.PlayerPrx> result = new java.util.Vector<Casino.PlayerPrx>();

        Freeze.EvictorIterator p = _playerEvictor.getIterator("", 10);

        while(p.hasNext())
        {
            Ice.Identity ident = p.next();
            result.add(Casino.PlayerPrxHelper.uncheckedCast(current.adapter.createProxy(ident)));
        }

        return result.toArray(new Casino.PlayerPrx[0]);
    }

    public int
    getOutstandingChips(Ice.Current current)
    {
        return outstandingChips;
    }

    public int
    getEarnings(Ice.Current current)
    {
        return chipsSold - outstandingChips;
    }

    public int
    getLiveBetCount(Ice.Current current)
    {
        return _betResolver.getBetCount();
    }

    public boolean
    checkAllChips(Ice.Current current)
    {
        int playerTotal = 0;

        Casino.PlayerPrx[] players = getPlayers(current);
        for(Casino.PlayerPrx player : players)
        {
            playerTotal += player.getChips();
        }

        System.out.println("The players hold a total of " + playerTotal + " chips");

        int betTotal = 0;

        CasinoStore.PersistentBetPrx[] bets = getBets(current.adapter);
        for(CasinoStore.PersistentBetPrx bet : bets)
        {
            betTotal += bet.getChipsInPlay();
        }
        System.out.println("The bets hold a total of " + betTotal + " chips");
        System.out.println("players + bets chips == " + (playerTotal + betTotal));

        System.out.println("The bank has " + outstandingChips + " outstanding chips");

        return (playerTotal + betTotal) == outstandingChips;
    }

    public Casino.BetPrx
    createBet(int amount, int lifetime, Ice.Current current)
    {
        Ice.Identity ident = new Ice.Identity(java.util.UUID.randomUUID().toString(), "bet");
        long closeTime = System.currentTimeMillis() + lifetime;

        outstandingChips += amount;
        BetI betI = new BetI(amount, closeTime, _prx, _betEvictor, _bankEdge);

        CasinoStore.PersistentBetPrx newBet =
            CasinoStore.PersistentBetPrxHelper.uncheckedCast(_betEvictor.add(betI, ident));

        _betResolver.add(newBet, closeTime);
        return newBet;
    }

    public void
    win(int count, Ice.Current current)
    {
        outstandingChips -= count;
    }

    public void
    returnAllChips(CasinoStore.PersistentPlayerPrx p, Ice.Current current)
    {
        int count = 0;

        try
        {
            count = p.getChips();
        }
        catch(Ice.ObjectNotExistException ex)
        {
            //
            // impossible
            //
            assert false;
            throw new Freeze.DatabaseException("returnAllChips: player does not exist");
        }

        if(count != 0)
        {
            try
            {
               p.withdraw(count);
            }
            catch(Casino.OutOfChipsException e)
            {
                //
                // Impossible
                //
                assert false;
                throw new Freeze.DatabaseException("returnAllChips: out of chips");
            }
            outstandingChips -= count;
            chipsSold -= count;
        }
    }

    public void
    reloadBets(Ice.Current current)
    {
        CasinoStore.PersistentBetPrx[] bets = getBets(current.adapter);
        for(CasinoStore.PersistentBetPrx bet : bets)
        {
            _betResolver.add(bet, bet.getCloseTime());
        }
    }

    BankI()
    {
    }

    BankI(CasinoStore.PersistentBankPrx prx,
          Freeze.TransactionalEvictor evictor, Freeze.TransactionalEvictor playerEvictor,
          Freeze.TransactionalEvictor betEvictor, BetResolver betResolver, int bankEdge)
    {
        chipsSold = 0;
        outstandingChips = 0;
        init(prx, evictor, playerEvictor, betEvictor, betResolver, bankEdge);
    }

    void
    init(CasinoStore.PersistentBankPrx prx,
         Freeze.TransactionalEvictor evictor, Freeze.TransactionalEvictor playerEvictor,
         Freeze.TransactionalEvictor betEvictor, BetResolver betResolver, int bankEdge)
    {
        _prx = prx;
        _evictor = evictor;
        _playerEvictor = playerEvictor;
        _betEvictor = betEvictor;
        _betResolver = betResolver;
        _bankEdge = bankEdge;
    }

    private CasinoStore.PersistentBetPrx[]
    getBets(Ice.ObjectAdapter adapter)
    {
        java.util.Vector<CasinoStore.PersistentBetPrx> result = new java.util.Vector<CasinoStore.PersistentBetPrx>();

        Freeze.EvictorIterator p = _betEvictor.getIterator("", 100);

        while(p.hasNext())
        {
            Ice.Identity ident = p.next();
            result.add(CasinoStore.PersistentBetPrxHelper.uncheckedCast(adapter.createProxy(ident)));
        }

        return result.toArray(new CasinoStore.PersistentBetPrx[0]);
    }

    private CasinoStore.PersistentBankPrx _prx;
    private Freeze.TransactionalEvictor _evictor;
    private Freeze.TransactionalEvictor _playerEvictor;
    private Freeze.TransactionalEvictor _betEvictor;
    private BetResolver _betResolver;
    private int _bankEdge;
}
