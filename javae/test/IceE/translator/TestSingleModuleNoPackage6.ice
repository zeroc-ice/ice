// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use SingleModuleNoPackage types from (different) double module definitions

#include <SingleModuleNoPackage.ice>

module T1
{
module T2
{

const M::smnpEnum smnpTest6Constant = M::smnpE1;

struct smnpTest6Struct
{
    M::smnpEnum e;
    M::smnpStruct s;
    M::smnpStructSeq seq;
    M::smnpStringStructDict dict;
};

sequence<M::smnpStruct> smnpTest6StructSeq;

interface smnpTest6Interface extends M::smnpInterface {};

exception smnpTest6Exception extends M::smnpException
{
    M::smnpEnum e1;
    M::smnpStruct s1;
    M::smnpStructSeq seq1;
    M::smnpStringStructDict dict1;
};

class smnpTest6Class extends M::smnpBaseClass implements M::smnpBaseInterface
{
    M::smnpStruct
    smnpTest6Op1(M::smnpEnum i1,
                 M::smnpStruct i2,
                 M::smnpStructSeq i3,
                 M::smnpStringStructDict i4,
                 out M::smnpEnum o1,
                 out M::smnpStruct o2,
                 out M::smnpStructSeq o3,
                 out M::smnpStringStructDict o4)
        throws M::smnpException;

    ["ami"]
    M::smnpStruct
    smnpTest6Op2(M::smnpEnum i1,
                 M::smnpStruct i2,
                 M::smnpStructSeq i3,
                 M::smnpStringStructDict i4,
                 out M::smnpEnum o1,
                 out M::smnpStruct o2,
                 out M::smnpStructSeq o3,
                 out M::smnpStringStructDict o4)
        throws M::smnpException;

    ["amd"]
    M::smnpStruct
    smnpTest6Op3(M::smnpEnum i1,
                 M::smnpStruct i2,
                 M::smnpStructSeq i3,
                 M::smnpStringStructDict i4,
                 out M::smnpEnum o1,
                 out M::smnpStruct o2,
                 out M::smnpStructSeq o3,
                 out M::smnpStringStructDict o4)
        throws M::smnpException;
};

};
};
