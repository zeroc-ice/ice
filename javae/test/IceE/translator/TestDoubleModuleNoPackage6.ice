// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use DoubleModuleNoPackage types from (same) double module definitions

#include <DoubleModuleNoPackage.ice>

module M1
{
module M2
{

const dmnpEnum dmnpTest6Constant = dmnpE1;

struct dmnpTest6Struct
{
    dmnpEnum e;
    dmnpStruct s;
    dmnpStructSeq seq;
    dmnpStringStructDict dict;
};

sequence<dmnpStruct> dmnpTest6StructSeq;

interface dmnpTest6Interface extends dmnpInterface {};

exception dmnpTest6Exception extends dmnpException
{
    dmnpEnum e1;
    dmnpStruct s1;
    dmnpStructSeq seq1;
    dmnpStringStructDict dict1;
};

class dmnpTest6Class extends dmnpBaseClass implements dmnpBaseInterface
{
    dmnpStruct
    dmnpTest6Op1(dmnpEnum i1,
                 dmnpStruct i2,
                 dmnpStructSeq i3,
                 dmnpStringStructDict i4,
                 out dmnpEnum o1,
                 out dmnpStruct o2,
                 out dmnpStructSeq o3,
                 out dmnpStringStructDict o4)
        throws dmnpException;

    ["ami"]
    dmnpStruct
    dmnpTest6Op2(dmnpEnum i1,
                 dmnpStruct i2,
                 dmnpStructSeq i3,
                 dmnpStringStructDict i4,
                 out dmnpEnum o1,
                 out dmnpStruct o2,
                 out dmnpStructSeq o3,
                 out dmnpStringStructDict o4)
        throws dmnpException;

    ["amd"]
    dmnpStruct
    dmnpTest6Op3(dmnpEnum i1,
                 dmnpStruct i2,
                 dmnpStructSeq i3,
                 dmnpStringStructDict i4,
                 out dmnpEnum o1,
                 out dmnpStruct o2,
                 out dmnpStructSeq o3,
                 out dmnpStringStructDict o4)
        throws dmnpException;
};

};
};
