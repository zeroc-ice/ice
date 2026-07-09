// Copyright (c) ZeroC, Inc.

#pragma once

["java:identifier:test.Ice.middleware.Test"]
module Test
{
    exception MyException {}

    interface MyObject
    {
        ["amd"] string getName();
    }
}
