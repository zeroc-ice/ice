// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef NRV0_ICE
#define NRV0_ICE

module Demo
{

[["cpp:include:MyStringSeq.h"]]
["cpp:type:MyStringSeq"] sequence<string> StringSeq;
const int StringSeqSize = 50000;

interface Nrvo
{
    StringSeq op1();
    StringSeq op2();
    StringSeq op3(int size);
    void shutdown();
};

};

#endif
