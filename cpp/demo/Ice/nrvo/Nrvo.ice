// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef NRVO_ICE
#define NRVO_ICE

[["cpp:include:MyStringSeq.h"]]

module Demo
{

["cpp:type:MyStringSeq"] sequence<string> StringSeq;

interface Nrvo
{
    StringSeq op1();
    StringSeq op2();
    StringSeq op3(int size);
    void shutdown();
};

};

#endif
