// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use SingleModuleNoPackage types from (same) double module definitions

#include <SingleModuleNoPackage.ice>

module M
{
module N
{

const smnpEnum smnpTest7Constant = smnpE1;

struct smnpTest7Struct
{
    smnpEnum e;
    smnpStruct s;
    smnpStructSeq seq;
    smnpStringStructDict dict;
};

sequence<smnpStruct> smnpTest7StructSeq;

interface smnpTest7Interface extends smnpInterface {};

exception smnpTest7Exception extends smnpException
{
    smnpEnum e1;
    smnpStruct s1;
    smnpStructSeq seq1;
    smnpStringStructDict dict1;
};

class smnpTest7Class extends smnpBaseClass implements smnpBaseInterface
{
    smnpStruct
    smnpTest7Op1(smnpEnum i1,
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
    smnpTest7Op2(smnpEnum i1,
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
    smnpTest7Op3(smnpEnum i1,
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
};
