//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Interceptor
{
    public class MyObject : IMyObject
    {
        public int Add(int x, int y, Current current) => x + y;

        public int AddWithRetry(int x, int y, Current current)
        {
            if (current.Context.TryGetValue("retry", out string? value) && value == "no")
            {
                return x + y;
            }
            throw new RetryException();
        }

        public int BadAdd(int x, int y, Current current) => throw new InvalidInputException("badAdd");

        public int NotExistAdd(int x, int y, Current current) => throw new ObjectNotExistException(current);

        public void Op1(Current current)
        {
        }

        public void OpWithBinaryContext(Token token, Current current)
        {
        }

        public void Shutdown(Current current) => current.Communicator.ShutdownAsync();
    }
}
