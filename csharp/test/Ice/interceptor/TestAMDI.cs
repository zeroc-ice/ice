//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Interceptor
{
    public sealed class MyObjectAsync : IMyObjectAsync
    {
        public ValueTask<int> AddAsync(int x, int y, Current current) => new ValueTask<int>(x + y);
        public ValueTask<int> AddWithRetryAsync(int x, int y, Current current)
        {
            if (current.Context.ContainsKey("retry") && current.Context["retry"].Equals("no"))
            {
                return new ValueTask<int>(x + y);
            }
            throw new RetryException();
        }
        public ValueTask<int> BadAddAsync(int x, int y, Current current) => throw new InvalidInputException("badAdd");
        public ValueTask<int> NotExistAddAsync(int x, int y, Current current) => throw new ObjectNotExistException(current);
        public ValueTask Op1Async(Current current) => new ValueTask();
        public ValueTask OpWithBinaryContextAsync(Token token, Current current) => new ValueTask();
        public ValueTask ShutdownAsync(Current current)
        {
            current.Communicator.ShutdownAsync();
            return new ValueTask();
        }
    }
}
