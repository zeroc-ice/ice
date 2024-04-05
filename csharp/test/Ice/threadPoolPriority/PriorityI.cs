// Copyright (c) ZeroC, Inc.

namespace Ice
{
    namespace threadPoolPriority
    {
        public class PriorityI : Test.PriorityDisp_
        {

            public override void shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            public override string getPriority(Ice.Current current)
            {
                return Thread.CurrentThread.Priority.ToString();
            }
        }
    }
}
