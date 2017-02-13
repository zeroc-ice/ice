// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module Test
{

exception UserEx
{
};

sequence<byte> ByteSeq;

interface Metrics
{
    void op();

    idempotent void fail();

    void opWithUserException()
        throws UserEx;

    void opWithRequestFailedException();

    void opWithLocalException();

    void opWithUnknownException();

    void opByteS(ByteSeq bs);

    Object* getAdmin();

    void shutdown();
};

interface Controller
{
    void hold();

    void resume();
};

};
