//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading;

namespace Ice
{
    namespace threadPoolPriority
    {
        public class PriorityI : Test.Priority
        {

            public void shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            public string getPriority(Ice.Current current)
            {
                return Thread.CurrentThread.Priority.ToString();
            }
        }
    }
}
