//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace Ice
{
    namespace interceptor
    {
        class MySystemException : Ice.SystemException
        {
            public
            MySystemException()
            {
            }

            override public string
            ice_id()
            {
                return "::MySystemException";
            }
        };

        class MyObjectI : Test.MyObject
        {
            protected static void
            test(bool b)
            {
                if (!b)
                {
                    throw new System.Exception();
                }
            }

            public int
            add(int x, int y, Ice.Current current)
            {
                return x + y;
            }

            public int
            addWithRetry(int x, int y, Ice.Current current)
            {
                test(current != null);
                test(current.Context != null);

                if (current.Context.ContainsKey("retry") && current.Context["retry"].Equals("no"))
                {
                    return x + y;
                }
                throw new RetryException();
            }

            public int
            badAdd(int x, int y, Ice.Current current)
            {
                throw new Test.InvalidInputException();
            }

            public int
            notExistAdd(int x, int y, Ice.Current current)
            {
                throw new Ice.ObjectNotExistException();
            }

            public int
            badSystemAdd(int x, int y, Ice.Current current)
            {
                throw new MySystemException();
            }

            //
            // AMD
            //
            public async Task<int>
            amdAddAsync(int x, int y, Ice.Current current)
            {
                await Task.Delay(10);
                return x + y;
            }

            public async Task<int>
            amdAddWithRetryAsync(int x, int y, Ice.Current current)
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

            public async Task<int>
            amdBadAddAsync(int x, int y, Ice.Current current)
            {
                await Task.Delay(10);
                throw new Test.InvalidInputException();
            }

            public async Task<int>
            amdNotExistAddAsync(int x, int y, Ice.Current current)
            {
                await Task.Delay(10);
                throw new Ice.ObjectNotExistException();
            }

            public async Task<int>
            amdBadSystemAddAsync(int x, int y, Ice.Current current)
            {
                await Task.Delay(10);
                throw new MySystemException();
            }
        }
    }
}
