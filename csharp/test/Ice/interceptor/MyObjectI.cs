//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Interceptor
{
    public class MyObject : IMyObject
    {
        public int Add(int x, int y, Current current) => x + y;

        public int AddWithRetry(int x, int y, Current current)
        {
            if (current.Context.ContainsKey("retry") && current.Context["retry"].Equals("no"))
            {
                return x + y;
            }
            throw new RetryException();
        }

        public int BadAdd(int x, int y, Current current) => throw new InvalidInputException("badAdd");

        public int NotExistAdd(int x, int y, Current current) => throw new ObjectNotExistException(current);

        //
        // AMD
        //
        public async ValueTask<int> AmdAddAsync(int x, int y, Current current)
        {
            await Task.Delay(10);
            return x + y;
        }

        public async ValueTask<int> AmdAddWithRetryAsync(int x, int y, Current current)
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

        public async ValueTask<int> AmdBadAddAsync(int x, int y, Current current)
        {
            await Task.Delay(10);
            throw new InvalidInputException("amdBadAdd");
        }

        public async ValueTask<int> AmdNotExistAddAsync(int x, int y, Current current)
        {
            await Task.Delay(10);
            throw new ObjectNotExistException(current);
        }
    }
}
