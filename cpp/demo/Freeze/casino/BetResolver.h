// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CASINO_BET_RESOLVER_H
#define CASINO_BET_RESOLVER_H

#include <IceUtil/IceUtil.h>
#include <Freeze/Freeze.h>
#include <Casino.h>
#include <CasinoStore.h>


class BetResolver
{
public:

    BetResolver();

    void start();
   
    void add(const CasinoStore::PersistentBetPrx&, Ice::Long);
    
    void destroy();
    
    int getBetCount() const;

#if (!defined(_MSC_VER) || (_MSC_VER >= 1300)) && (!defined(__BCPLUSPLUS__))
//
// Some compilers don't let local classes access private members
//
private:
#endif

    void decrementBetCount();

private:

    int _betCount;
    IceUtil::Mutex _mutex;
    std::vector<IceUtil::TimerPtr> _timers;
};

#endif

