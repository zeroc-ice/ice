//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace Ice.servantLocator.AMD
{
    public sealed class TestIntf : Test.ITestIntf
    {
        public Task requestFailedExceptionAsync(Current current) => null;

        public Task unknownUserExceptionAsync(Current current) => null;

        public Task unknownLocalExceptionAsync(Current current) => null;

        public Task unknownExceptionAsync(Current current) => null;

        public Task localExceptionAsync(Current current) => null;

        public Task userExceptionAsync(Current current) => null;

        public Task csExceptionAsync(Current current) => null;

        public Task
        unknownExceptionWithServantExceptionAsync(Current current) => throw new ObjectNotExistException();

        public Task<string>
        impossibleExceptionAsync(bool @throw, Current current)
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
                return Task.FromResult("Hello");
            }
        }

        public Task<string>
        intfUserExceptionAsync(bool @throw, Current current)
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
                return Task.FromResult("Hello");
            }
        }

        public Task asyncResponseAsync(Current current)
        {
            return null;
            throw new ObjectNotExistException();
        }

        public Task asyncExceptionAsync(Ice.Current current)
        {
            throw new Test.TestIntfUserException();
            throw new Ice.ObjectNotExistException();
        }

        public Task shutdownAsync(Current current)
        {
            current.Adapter.Deactivate();
            return null;
        }
    }
}
