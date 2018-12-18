// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{
    namespace retry
    {
        public sealed class RetryI : Test.RetryDisp_
        {
            public RetryI()
            {
            }

            public override void op(bool kill, Ice.Current current)
            {
                if(kill)
                {
                    if(current.con != null)
                    {
                        current.con.close(Ice.ConnectionClose.Forcefully);
                    }
                    else
                    {
                        throw new Ice.ConnectionLostException();
                    }
                }
            }

            public override int opIdempotent(int nRetry, Ice.Current current)
            {
                if(nRetry > _counter)
                {
                    ++_counter;
                    throw new Ice.ConnectionLostException();
                }
                int counter = _counter;
                _counter = 0;
                return counter;
            }

            public override void opNotIdempotent(Ice.Current current)
            {
                throw new Ice.ConnectionLostException();
            }

            public override void opSystemException(Ice.Current c)
            {
                throw new SystemFailure();
            }

            public override void shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            private int _counter;
        }
    }
}
