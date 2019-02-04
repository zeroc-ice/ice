//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading;

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
