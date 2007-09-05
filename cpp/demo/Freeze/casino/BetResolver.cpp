// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <BetResolver.h>


BetResolver::BetResolver() :
    _betCount(0)
{
}

void
BetResolver::start()
{
    //
    // Initialize the timers
    //
    for(int i = 0; i < 3; ++i)
    {
        _timers.push_back(new IceUtil::Timer);
    }
}


void
BetResolver::add(const CasinoStore::PersistentBetPrx& bet, Ice::Long closeTime)
{
    class Task : public IceUtil::TimerTask
    {
    public:

        Task(BetResolver& resolver, const CasinoStore::PersistentBetPrx& bet) :
            _resolver(resolver),
            _bet(bet)
        {
        }

        virtual void 
        run()
        {
            try
            {
                _bet->complete(IceUtil::random());
            }
            catch(const Ice::ObjectNotExistException&)
            {
                //
                // Looks like this bet was never saved (committed)
                //
            }
            catch(...)
            {
                _resolver.decrementBetCount();
                throw;
            }
            _resolver.decrementBetCount();
        }

    private:
        BetResolver& _resolver;
        CasinoStore::PersistentBetPrx _bet;
     
    };

    _timers[IceUtil::random(_timers.size())]->schedule(new Task(*this, bet), IceUtil::Time::milliSeconds(closeTime));

    IceUtil::Mutex::Lock sync(_mutex);
    _betCount++;
}

void
BetResolver::destroy()
{
    for(size_t i = 0; i < _timers.size(); ++i)
    {
        _timers[i]->destroy();
    }
}

int
BetResolver::getBetCount() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _betCount;
}

void
BetResolver::decrementBetCount()
{
    IceUtil::Mutex::Lock sync(_mutex);
    _betCount--;
}
