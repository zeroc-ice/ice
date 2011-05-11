// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CASINO_BANK_I_H
#define CASINO_BANK_I_H

#include <Freeze/Freeze.h>
#include <Casino.h>
#include <CasinoStore.h>
#include <BetResolver.h>

class BankI : public CasinoStore::PersistentBank
{
public:

    virtual bool buyChips(int, const Casino::PlayerPrx&, const Ice::Current&);

    virtual Casino::PlayerPrxSeq getPlayers(const Ice::Current&) const;

    virtual int getOutstandingChips(const Ice::Current&) const;

    virtual int getEarnings(const Ice::Current&) const;

    virtual int getLiveBetCount(const Ice::Current&) const;

    virtual bool checkAllChips(const Ice::Current&) const;

    virtual Casino::BetPrx createBet(int, int, const Ice::Current&);

    virtual void win(int, const Ice::Current&);

    virtual void returnAllChips(const CasinoStore::PersistentPlayerPrx&, const Ice::Current&);

    virtual void reloadBets(const Ice::Current&);

    BankI();

    BankI(const CasinoStore::PersistentBankPrx&,
          const Freeze::TransactionalEvictorPtr&, const Freeze::TransactionalEvictorPtr&,
          const Freeze::TransactionalEvictorPtr&, BetResolver&, int);

    void
    init(const CasinoStore::PersistentBankPrx&,
         const Freeze::TransactionalEvictorPtr&, const Freeze::TransactionalEvictorPtr&,
         const Freeze::TransactionalEvictorPtr&, BetResolver&, int);

private:

    std::vector<CasinoStore::PersistentBetPrx> getBets(const Ice::ObjectAdapterPtr&) const;

#if (defined(_MSC_VER) && (_MSC_VER < 1300)) || defined(__BCPLUSPLUS__)
//
// Some compilers don't let local classes access private data members
//
public:
#endif

    CasinoStore::PersistentBankPrx _prx;
    Freeze::TransactionalEvictorPtr _evictor;
    Freeze::TransactionalEvictorPtr _playerEvictor;
    Freeze::TransactionalEvictorPtr _betEvictor;
    BetResolver* _betResolver;
    int _bankEdge;
};

#endif
