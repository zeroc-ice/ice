// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using Test;

namespace ZeroC.Ice.Test.Interceptor
{
    public class MyObject : IMyObject
    {
        public int Add(int x, int y, Current current, CancellationToken cancel) => x + y;

        public int AddWithRetry(int x, int y, Current current, CancellationToken cancel)
        {
            if (current.Context.TryGetValue("retry", out string? value) && value == "no")
            {
                return x + y;
            }
            throw new RetryException();
        }

        public int BadAdd(int x, int y, Current current, CancellationToken cancel) =>
            throw new InvalidInputException("badAdd");

        public int NotExistAdd(int x, int y, Current current, CancellationToken cancel) =>
            throw new ObjectNotExistException();

        public void Op1(Current current, CancellationToken cancel) =>
            TestHelper.Assert(DispatchInterceptors.LocalContext.Value == int.Parse(current.Context["local-user"]));

        public void OpWithBinaryContext(Token token, Current current, CancellationToken cancel)
        {
        }

        public void Shutdown(Current current, CancellationToken cancel) => current.Communicator.ShutdownAsync();
    }
}
