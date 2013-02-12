// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CASINO_PLAYER_I_H
#define CASINO_PLAYER_I_H

#include <Freeze/Freeze.h>
#include <Casino.h>
#include <CasinoStore.h>

class PlayerI : public CasinoStore::PersistentPlayer
{
public:

    virtual int getChips(const Ice::Current&) const;

    virtual void destroy(const Ice::Current&);

    virtual void  withdraw(int, const Ice::Current&);

    virtual void win(int count, const Ice::Current&);

    PlayerI();

    void init(const CasinoStore::PersistentPlayerPrx&, const Freeze::TransactionalEvictorPtr&,
              const CasinoStore::PersistentBankPrx&);

private:

    CasinoStore::PersistentPlayerPrx _myPrx;
    Freeze::TransactionalEvictorPtr _evictor;
    CasinoStore::PersistentBankPrx _bank;
};

#endif
