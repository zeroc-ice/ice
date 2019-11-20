//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace servantLocator
    {
        public sealed class TestI : Test.TestIntf
        {
            public void requestFailedException(Ice.Current current)
            {
            }

            public void unknownUserException(Ice.Current current)
            {
            }

            public void unknownLocalException(Ice.Current current)
            {
            }

            public void unknownException(Ice.Current current)
            {
            }

            public void localException(Ice.Current current)
            {
            }

            public void userException(Ice.Current current)
            {
            }

            public void csException(Ice.Current current)
            {
            }

            public void unknownExceptionWithServantException(Ice.Current current)
            {
                throw new Ice.ObjectNotExistException();
            }

            public string impossibleException(bool @throw, Ice.Current current)
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

            public string intfUserException(bool @throw, Ice.Current current)
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

            public void asyncResponse(Ice.Current current)
            {
                //
                // Only relevant for AMD.
                //
            }

            public void asyncException(Ice.Current current)
            {
                //
                // Only relevant for AMD.
                //
            }

            public void shutdown(Ice.Current current)
            {
                current.adapter.Deactivate();
            }
        }
    }
}
