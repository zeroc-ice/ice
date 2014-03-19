// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <BankI.h>
#include <BetI.h>
#include <IceUtil/IceUtil.h>

using namespace std;

bool
BankI::buyChips(int chips, const Casino::PlayerPrx& p, const Ice::Current&)
{
    outstandingChips += chips;
    chipsSold += chips;

    CasinoStore::PersistentPlayerPrx player = CasinoStore::PersistentPlayerPrx::uncheckedCast(p);

    try
    {
        player->win(chips);
    }
    catch(const Ice::ObjectNotExistException&)
    {
        _evictor->getCurrentTransaction()->rollback();
        return false;
    }

    return true;
}

Casino::PlayerPrxSeq
BankI::getPlayers(const Ice::Current& current) const
{
    Casino::PlayerPrxSeq result;

    Freeze::EvictorIteratorPtr p = _playerEvictor->getIterator("", 10);

    while(p->hasNext())
    {
        Ice::Identity ident = p->next();
        result.push_back(Casino::PlayerPrx::uncheckedCast(current.adapter->createProxy(ident)));
    }

    return result;
}

int
BankI::getOutstandingChips(const Ice::Current&) const
{
    return outstandingChips;
}

int
BankI::getEarnings(const Ice::Current&) const
{
    return chipsSold - outstandingChips;
}

int
BankI::getLiveBetCount(const Ice::Current&) const
{
    return _betResolver->getBetCount();
}


bool
BankI::checkAllChips(const Ice::Current& current) const
{
    int playerTotal = 0;

    Casino::PlayerPrxSeq players = getPlayers(current);
    size_t i;
    for(i = 0; i < players.size(); ++i)
    {
        playerTotal += players[i]->getChips();
    }

    cout << "The players hold a total of " << playerTotal << " chips" << endl;

    int betTotal = 0;

    vector<CasinoStore::PersistentBetPrx> bets = getBets(current.adapter);
    for(i = 0; i < bets.size(); ++i)
    {
        betTotal += bets[i]->getChipsInPlay();
    }
    cout << "The bets hold a total of " << betTotal << " chips" << endl;
    cout << "players + bets chips == " << (playerTotal + betTotal) << endl;

    cout <<"The bank has " << outstandingChips << " outstanding chips" << endl;

    return (playerTotal + betTotal) == outstandingChips;
}

Casino::BetPrx
BankI::createBet(int amount, int lifetime, const Ice::Current&)
{
    Ice::Identity ident = { IceUtil::generateUUID(), "bet" };
    Ice::Long closeTime = IceUtil::Time::now().toMilliSeconds() + lifetime;

    outstandingChips += amount;
    Ice::ObjectPtr betI = new BetI(amount, closeTime, _prx, _betEvictor, _bankEdge);

    CasinoStore::PersistentBetPrx newBet =
        CasinoStore::PersistentBetPrx::uncheckedCast(_betEvictor->add(betI, ident));

    _betResolver->add(newBet, closeTime);
    return newBet;
}

void
BankI::win(int count, const Ice::Current&)
{
    outstandingChips -= count;
}

void
BankI::returnAllChips(const CasinoStore::PersistentPlayerPrx& p, const Ice::Current&)
{
    int count = 0;

    try
    {
        count = p->getChips();
    }
    catch(const Ice::ObjectNotExistException&)
    {
        //
        // impossible
        //
        assert(0);
        throw Freeze::DatabaseException(__FILE__, __LINE__, "returnAllChips: player does not exist");
    }

    if(count != 0)
    {
        try
        {
            p->withdraw(count);
        }
        catch(const Casino::OutOfChipsException&)
        {
            //
            // Impossible
            //
            assert(0);
            throw  Freeze::DatabaseException(__FILE__, __LINE__, "returnAllChips: out of chips");
        }
        outstandingChips -= count;
        chipsSold -= count;
    }
}

void
BankI::reloadBets(const Ice::Current& current)
{
    vector<CasinoStore::PersistentBetPrx> bets = getBets(current.adapter);

    for(size_t i = 0; i < bets.size(); ++i)
    {
        _betResolver->add(bets[i], bets[i]->getCloseTime());
    }
}

BankI::BankI()
{
}

BankI::BankI(const CasinoStore::PersistentBankPrx& prx,
             const Freeze::TransactionalEvictorPtr& evictor, const Freeze::TransactionalEvictorPtr& playerEvictor,
             const Freeze::TransactionalEvictorPtr& betEvictor, BetResolver& betResolver, int bankEdge)
{
    chipsSold = 0;
    outstandingChips = 0;
    init(prx, evictor, playerEvictor, betEvictor, betResolver, bankEdge);
}

void
BankI::init(const CasinoStore::PersistentBankPrx& prx,
            const Freeze::TransactionalEvictorPtr& evictor, const Freeze::TransactionalEvictorPtr& playerEvictor,
            const Freeze::TransactionalEvictorPtr& betEvictor, BetResolver& betResolver, int bankEdge)
{
    _prx = prx;
    _evictor = evictor;
    _playerEvictor = playerEvictor;
    _betEvictor = betEvictor;
    _betResolver = &betResolver;
    _bankEdge = bankEdge;
}

vector<CasinoStore::PersistentBetPrx>
BankI::getBets(const Ice::ObjectAdapterPtr& adapter) const
{
    vector<CasinoStore::PersistentBetPrx> result;

    Freeze::EvictorIteratorPtr p = _betEvictor->getIterator("", 100);

    while(p->hasNext())
    {
        Ice::Identity ident = p->next();
        result.push_back(CasinoStore::PersistentBetPrx::uncheckedCast(adapter->createProxy(ident)));
    }

    return result;
}
