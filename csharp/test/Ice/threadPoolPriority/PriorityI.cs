//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading;

namespace Ice.threadPoolPriority
{
    public class Priority : Test.IPriority
    {
        public void shutdown(Current current) => current.Adapter.Communicator.shutdown();
        public string getPriority(Current current) => Thread.CurrentThread.Priority.ToString();
    }
}
