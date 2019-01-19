//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceUtilInternal;

public class StopWatch
{
    public void
    start()
    {
        _s = System.nanoTime();
    }

    public long
    stop()
    {
        assert(isStarted());
        long d = (System.nanoTime() - _s) / 1000;
        _s = 0;
        return d;
    }

    public boolean
    isStarted()
    {
        return _s != 0;
    }

    public long
    delay()
    {
        return (System.nanoTime() - _s) / 1000;
    }

    private long _s = 0;
}
