//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.servantLocator
{
    public sealed class TestIntf : Test.ITestIntf
    {
        public void requestFailedException(Current current)
        {
        }

        public void unknownUserException(Current current)
        {
        }

        public void unknownLocalException(Current current)
        {
        }

        public void unknownException(Current current)
        {
        }

        public void localException(Current current)
        {
        }

        public void userException(Current current)
        {
        }

        public void csException(Current current)
        {
        }

        public void unknownExceptionWithServantException(Current current) => throw new ObjectNotExistException();

        public string impossibleException(bool @throw, Current current)
        {
            if (@throw)
            {
                throw new Test.TestImpossibleException();
            }
            //
            // Return a value so we can be sure that the stream position
            // is reset correctly if finished() throws.
            //
            return "Hello";
        }

        public string intfUserException(bool @throw, Current current)
        {
            if (@throw)
            {
                throw new Test.TestIntfUserException();
            }
            //
            // Return a value so we can be sure that the stream position
            // is reset correctly if finished() throws.
            //
            return "Hello";
        }

        public void asyncResponse(Current current)
        {
            //
            // Only relevant for AMD.
            //
        }

        public void asyncException(Current current)
        {
            //
            // Only relevant for AMD.
            //
        }

        public void shutdown(Current current) => current.Adapter.Deactivate();
    }
}
