//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Interceptor
{
    public class MyObject : IMyObject
    {
        public int
        add(int x, int y, Current current) => x + y;

        public int
        addWithRetry(int x, int y, Current current)
        {
            if (current.Context.ContainsKey("retry") && current.Context["retry"].Equals("no"))
            {
                return x + y;
            }
            throw new RetryException();
        }

        public int
        badAdd(int x, int y, Current current) => throw new InvalidInputException("badAdd");

        public int
        notExistAdd(int x, int y, Current current) => throw new ObjectNotExistException(current);

        //
        // AMD
        //
        public async ValueTask<int>
        amdAddAsync(int x, int y, Current current)
        {
            await Task.Delay(10);
            return x + y;
        }

        public async ValueTask<int>
        amdAddWithRetryAsync(int x, int y, Current current)
        {
            if (current.Context.ContainsKey("retry") && current.Context["retry"].Equals("no"))
            {
                await Task.Delay(10);
                return x + y;
            }
            else
            {
                throw new RetryException();
            }
        }

        public async ValueTask<int> amdBadAddAsync(int x, int y, Current current)
        {
            await Task.Delay(10);
            throw new InvalidInputException("amdBadAdd");
        }

        public async ValueTask<int> amdNotExistAddAsync(int x, int y, Current current)
        {
            await Task.Delay(10);
            throw new ObjectNotExistException(current);
        }
    }
}
