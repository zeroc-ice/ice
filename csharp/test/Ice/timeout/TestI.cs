// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

using System.Threading;

namespace Ice
{
    namespace timeout
    {
        internal class ActivateAdapterThread
        {
            internal ActivateAdapterThread(Ice.ObjectAdapter adapter, int timeout)
            {
                _adapter = adapter;
                _timeout = timeout;
            }

            internal void run()
            {
                _adapter.waitForHold();
                Thread.Sleep(_timeout);
                _adapter.activate();
            }

            private Ice.ObjectAdapter _adapter;
            private int _timeout;
        }

        internal class TimeoutI : Test.TimeoutDisp_
        {
            public override void op(Ice.Current current)
            {
            }

            public override void sendData(byte[] seq, Ice.Current current)
            {
            }

            public override void sleep(int to, Ice.Current current)
            {
                Thread.Sleep(to);
            }
        }

        internal class ControllerI : Test.ControllerDisp_
        {
            public ControllerI(Ice.ObjectAdapter adapter)
            {
                _adapter = adapter;
            }

            public override void holdAdapter(int to, Ice.Current current)
            {
                _adapter.hold();
                if(to >= 0)
                {
                    ActivateAdapterThread act = new ActivateAdapterThread(_adapter, to);
                    Thread thread = new Thread(new ThreadStart(act.run));
                    thread.Start();
                }
            }

            public override void resumeAdapter(Ice.Current current)
            {
                _adapter.activate();
            }

            public override void shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            private Ice.ObjectAdapter _adapter;
        }
    }
}
