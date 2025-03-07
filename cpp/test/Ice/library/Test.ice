// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:LIBRARY_TEST_API"]]

module Test
{
    exception UserError
    {
        string message;
    }
<<<<<<< Updated upstream

=======

>>>>>>> Stashed changes
    interface MyInterface
    {
        void op(bool throwIt) throws UserError;
    }
}
