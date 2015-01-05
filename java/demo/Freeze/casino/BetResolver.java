// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class BetResolver
{
    BetResolver()
    {
        // We want the executor to shutdown even if there are scheduled tasks.
        _executor.setExecuteExistingDelayedTasksAfterShutdownPolicy(false);
    }

    void
    add(final CasinoStore.PersistentBetPrx bet, long closeTime)
    {
        Runnable task = new Runnable()
            {
                @Override
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
                        _betCount.decrementAndGet();
                    }
                }
            };

        // closeTime is milliseconds from the epoch. Convert to relative time.
        long now = new java.util.Date().getTime();
        _executor.schedule(task, closeTime - now, java.util.concurrent.TimeUnit.MILLISECONDS);

        _betCount.incrementAndGet();
   }

    void
    cancel()
    {
        _executor.shutdown();
    }

    int
    getBetCount()
    {
        return _betCount.get();
    }

    private java.util.concurrent.ScheduledThreadPoolExecutor _executor =
        new java.util.concurrent.ScheduledThreadPoolExecutor(3);
    private java.util.Random _random = new java.util.Random();
    private java.util.concurrent.atomic.AtomicInteger _betCount = new java.util.concurrent.atomic.AtomicInteger();
}
