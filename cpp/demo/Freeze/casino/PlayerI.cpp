// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <PlayerI.h>

int
PlayerI::getChips(const Ice::Current&) const
{
    return chips;
}

void
PlayerI::destroy(const Ice::Current& current)
{
    _bank->returnAllChips(_myPrx);
    _evictor->remove(current.id);
}

void
PlayerI::withdraw(int count, const Ice::Current&)
{
    int newBalance = chips - count;
    if(newBalance < 0)
    {
        throw Casino::OutOfChipsException();
    }
    chips = newBalance;
}

void
PlayerI::win(int count, const Ice::Current&)
{
    assert(count >= 0);
    chips += count;
}

PlayerI::PlayerI() :
    CasinoStore::PersistentPlayer(0)
{
}

void
PlayerI::init(const CasinoStore::PersistentPlayerPrx& myPrx, const Freeze::TransactionalEvictorPtr& evictor,
              const CasinoStore::PersistentBankPrx& bank)
{
    _myPrx = myPrx;
    _evictor = evictor;
    _bank = bank;
}
