//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace Ice
{
    namespace servantLocator
    {
        namespace AMD
        {
            public sealed class TestI : Test.TestIntf
            {
                public Task requestFailedExceptionAsync(Ice.Current current)
                {
                    return null;
                }

                public Task unknownUserExceptionAsync(Ice.Current current)
                {
                    return null;
                }

                public Task unknownLocalExceptionAsync(Ice.Current current)
                {
                    return null;
                }

                public Task unknownExceptionAsync(Ice.Current current)
                {
                    return null;
                }

                public Task localExceptionAsync(Ice.Current current)
                {
                    return null;
                }

                public Task userExceptionAsync(Ice.Current current)
                {
                    return null;
                }

                public Task csExceptionAsync(Ice.Current current)
                {
                    return null;
                }

                public Task
                unknownExceptionWithServantExceptionAsync(Ice.Current current)
                {
                    throw new Ice.ObjectNotExistException();
                }

                public Task<string>
                impossibleExceptionAsync(bool @throw, Ice.Current current)
                {
                    if (@throw)
                    {
                        throw new Test.TestImpossibleException();
                    }
                    else
                    {
                        //
                        // Return a value so we can be sure that the stream position
                        // is reset correctly iuf finished throws.
                        //
                        return Task.FromResult<string>("Hello");
                    }
                }

                public Task<string>
                intfUserExceptionAsync(bool @throw, Ice.Current current)
                {
                    if (@throw)
                    {
                        throw new Test.TestIntfUserException();
                    }
                    else
                    {
                        //
                        // Return a value so we can be sure that the stream position
                        // is reset correctly iuf finished throws.
                        //
                        return Task.FromResult<string>("Hello");
                    }
                }

                public Task asyncResponseAsync(Ice.Current current)
                {
                    return null;
                    throw new Ice.ObjectNotExistException();
                }

                public Task asyncExceptionAsync(Ice.Current current)
                {
                    throw new Test.TestIntfUserException();
                    throw new Ice.ObjectNotExistException();
                }

                public Task shutdownAsync(Ice.Current current)
                {
                    current.adapter.deactivate();
                    return null;
                }
            }
        }
    }
}
