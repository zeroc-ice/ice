// Copyright (c) ZeroC, Inc.

namespace Ice.retry;

public sealed class RetryI : Test.RetryDisp_
{
    public RetryI()
    {
    }

    public override void op(bool kill, Ice.Current current)
    {
        if (kill)
        {
            if (current.con != null)
            {
                current.con.abort();
            }
            else
            {
                throw new Ice.ConnectionLostException(peerAddress: null);
            }
        }
    }

    public override int opIdempotent(int c, Ice.Current current)
    {
        if (c > _counter)
        {
            ++_counter;
            throw new Ice.ConnectionLostException(peerAddress: null);
        }
        int counter = _counter;
        _counter = 0;
        return counter;
    }

    public override void opNotIdempotent(Ice.Current current) => throw new Ice.ConnectionLostException(peerAddress: null);

    public override void sleep(int delay, Ice.Current current) => Thread.Sleep(delay);

    public override void shutdown(Ice.Current current) => current.adapter.getCommunicator().shutdown();

    private int _counter;
}
