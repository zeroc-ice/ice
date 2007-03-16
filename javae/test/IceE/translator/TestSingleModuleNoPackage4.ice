// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use SingleModuleNoPackage types from (same) single module definitions

#include <SingleModuleNoPackage.ice>

module M
{

const smnpEnum smnpTest4Constant = smnpE1;

struct smnpTest4Struct
{
    smnpEnum e;
    smnpStruct s;
    smnpStructSeq seq;
    smnpStringStructDict dict;
};

sequence<smnpStruct> smnpTest4StructSeq;

interface smnpTest4Interface extends smnpInterface {};

exception smnpTest4Exception extends smnpException
{
    smnpEnum e1;
    smnpStruct s1;
    smnpStructSeq seq1;
    smnpStringStructDict dict1;
};

class smnpTest4Class extends smnpBaseClass implements smnpBaseInterface
{
    smnpStruct
    smnpTest4Op1(smnpEnum i1,
                 smnpStruct i2,
                 smnpStructSeq i3,
                 smnpStringStructDict i4,
                 out smnpEnum o1,
                 out smnpStruct o2,
                 out smnpStructSeq o3,
                 out smnpStringStructDict o4)
        throws smnpException;

    ["ami"]
    smnpStruct
    smnpTest4Op2(smnpEnum i1,
                 smnpStruct i2,
                 smnpStructSeq i3,
                 smnpStringStructDict i4,
                 out smnpEnum o1,
                 out smnpStruct o2,
                 out smnpStructSeq o3,
                 out smnpStringStructDict o4)
        throws smnpException;

    ["amd"]
    smnpStruct
    smnpTest4Op3(smnpEnum i1,
                 smnpStruct i2,
                 smnpStructSeq i3,
                 smnpStringStructDict i4,
                 out smnpEnum o1,
                 out smnpStruct o2,
                 out smnpStructSeq o3,
                 out smnpStringStructDict o4)
        throws smnpException;
};

};
