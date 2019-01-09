// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

[["js:es6-module"]]

module Test
{

interface Retry
{
    void op(bool kill);

    idempotent int opIdempotent(int c);
    void opNotIdempotent();
    void opSystemException();

    idempotent void shutdown();
}

}
