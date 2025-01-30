// Copyright (c) ZeroC, Inc.

namespace Ice
{
    namespace servantLocator
    {
        namespace AMD
        {
            public sealed class TestI : Test.TestIntfDisp_
            {
                public override Task requestFailedExceptionAsync(Ice.Current current)
                {
                    return Task.CompletedTask;
                }

                public override Task unknownUserExceptionAsync(Ice.Current current)
                {
                    return Task.CompletedTask;
                }

                public override Task unknownLocalExceptionAsync(Ice.Current current)
                {
                    return Task.CompletedTask;
                }

                public override Task unknownExceptionAsync(Ice.Current current)
                {
                    return Task.CompletedTask;
                }

                public override Task localExceptionAsync(Ice.Current current)
                {
                    return Task.CompletedTask;
                }

                public override Task userExceptionAsync(Ice.Current current)
                {
                    return Task.CompletedTask;
                }

                public override Task csExceptionAsync(Ice.Current current)
                {
                    return Task.CompletedTask;
                }

                public override Task
                unknownExceptionWithServantExceptionAsync(Ice.Current current)
                {
                    throw new Ice.ObjectNotExistException();
                }

                public override Task<string>
                impossibleExceptionAsync(bool shouldThrow, Ice.Current current)
                {
                    if (shouldThrow)
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

                public override Task<string>
                intfUserExceptionAsync(bool shouldThrow, Ice.Current current)
                {
                    if (shouldThrow)
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

                public override Task asyncResponseAsync(Ice.Current current)
                {
                    return Task.CompletedTask;
                    throw new Ice.ObjectNotExistException();
                }

                public override Task asyncExceptionAsync(Ice.Current current)
                {
                    throw new Test.TestIntfUserException();
                    throw new Ice.ObjectNotExistException();
                }

                public override Task shutdownAsync(Ice.Current current)
                {
                    current.adapter.deactivate();
                    return Task.CompletedTask;
                }
            }
        }
    }
}
