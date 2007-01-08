// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef NESTED_ICE
#define NESTED_ICE

module Demo
{

interface Nested
{
    void nestedCall(int level, Nested* proxy);
};

};

#endif
