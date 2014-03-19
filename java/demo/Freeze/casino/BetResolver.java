// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class BetResolver
{
    BetResolver()
    {
        //
        // We create a little pool of timers
        //
        for(int i = 0; i < _timers.length; ++i)
        {
            _timers[i] = new java.util.Timer();
        }
    }

    void
    add(final CasinoStore.PersistentBetPrx bet, long closeTime)
    {
        java.util.TimerTask task = new java.util.TimerTask()
            {
                public void
                run()
                {
                    //
                    // Note that this is a collocated call, from a non-dispatch
                    // thread; even then Freeze will properly create/commit the
                    // transaction.
                    //

                    try
                    {
                        bet.complete(_random.nextInt());
                    }
                    catch(Ice.ObjectNotExistException one)
                    {
                        //
                        // Looks like this bet was never saved (committed)
                        //
                    }
                    finally
                    {
                        synchronized(BetResolver.this)
                        {
                            _betCount--;
                        }
                    }
                }
            };

        _timers[_random.nextInt(_timers.length)].schedule(task, new java.util.Date(closeTime));

        synchronized(this)
        {
            _betCount++;
        }
    }

    void
    cancel()
    {
        for(java.util.Timer timer : _timers)
        {
            timer.cancel();
        }
    }

    synchronized int
    getBetCount()
    {
        return _betCount;
    }

    private java.util.Timer[] _timers = new java.util.Timer[3];
    private java.util.Random _random = new java.util.Random();
    private int _betCount = 0;
}
