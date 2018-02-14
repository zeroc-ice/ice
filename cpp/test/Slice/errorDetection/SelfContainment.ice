// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************



module Test
{

struct x
{
    int i;
    x j;        // Error
};

class y
{
    int i;
    y j;        // OK
};

};
