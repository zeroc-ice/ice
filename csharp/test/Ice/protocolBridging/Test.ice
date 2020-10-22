//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[suppress-warning(reserved-identifier)]]

module ZeroC::Ice::Test::ProtocolBridging
{
    exception MyError
    {
        int number;
    }

    interface TestIntf
    {
        // Simple operations
        int op(int x);
        void opVoid();

        // Operation with both return and out
        int opReturnOut(int x, out string y);

        // Oneway operation
        [oneway] void opOneway(int x);

        // Operation that throws remote exception
        void opMyError();

        // Operation that throws ObjectNotExistException (one of the special
        // ice1 system exceptions)
        void opObjectNotExistException();

        // Operation that returns a new proxy
        TestIntf opNewProxy();

        void shutdown();
    }
}
