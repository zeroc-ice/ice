// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace Ice
{
    namespace servantLocator
    {
        public sealed class TestI : Test.TestIntfDisp_
        {
            public override void requestFailedException(Ice.Current current)
            {
            }

            public override void unknownUserException(Ice.Current current)
            {
            }

            public override void unknownLocalException(Ice.Current current)
            {
            }

            public override void unknownException(Ice.Current current)
            {
            }

            public override void localException(Ice.Current current)
            {
            }

            public override void userException(Ice.Current current)
            {
            }

            public override void csException(Ice.Current current)
            {
            }

            public override void unknownExceptionWithServantException(Ice.Current current)
            {
                throw new Ice.ObjectNotExistException();
            }

            public override string impossibleException(bool @throw, Ice.Current current)
            {
                if(@throw)
                {
                    throw new Test.TestImpossibleException();
                }
                //
                // Return a value so we can be sure that the stream position
                // is reset correctly if finished() throws.
                //
                return "Hello";
            }

            public override string intfUserException(bool @throw, Ice.Current current)
            {
                if(@throw)
                {
                    throw new Test.TestIntfUserException();
                }
                //
                // Return a value so we can be sure that the stream position
                // is reset correctly if finished() throws.
                //
                return "Hello";
            }

            public override void asyncResponse(Ice.Current current)
            {
                //
                // Only relevant for AMD.
                //
            }

            public override void asyncException(Ice.Current current)
            {
                //
                // Only relevant for AMD.
                //
            }

            public override void shutdown(Ice.Current current)
            {
                current.adapter.deactivate();
            }
        }
    }
}
