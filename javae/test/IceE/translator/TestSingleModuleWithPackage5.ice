// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use SingleModuleWithPackage types from (same) single module definitions

#include <SingleModuleWithPackage.ice>

module M
{

const smwpEnum smwpTest5Constant = smwpE1;

struct smwpTest5Struct
{
    smwpEnum e;
    smwpStruct s;
    smwpStructSeq seq;
    smwpStringStructDict dict;
};

sequence<smwpStruct> smwpTest5StructSeq;

interface smwpTest5Interface extends smwpInterface {};

exception smwpTest5Exception extends smwpException
{
    smwpEnum e1;
    smwpStruct s1;
    smwpStructSeq seq1;
    smwpStringStructDict dict1;
};

class smwpTest5Class extends smwpBaseClass implements smwpBaseInterface
{
    smwpStruct
    smwpTest5Op1(smwpEnum i1,
                 smwpStruct i2,
                 smwpStructSeq i3,
                 smwpStringStructDict i4,
                 out smwpEnum o1,
                 out smwpStruct o2,
                 out smwpStructSeq o3,
                 out smwpStringStructDict o4)
        throws smwpException;

    ["ami"]
    smwpStruct
    smwpTest5Op2(smwpEnum i1,
                 smwpStruct i2,
                 smwpStructSeq i3,
                 smwpStringStructDict i4,
                 out smwpEnum o1,
                 out smwpStruct o2,
                 out smwpStructSeq o3,
                 out smwpStringStructDict o4)
        throws smwpException;

    ["amd"]
    smwpStruct
    smwpTest5Op3(smwpEnum i1,
                 smwpStruct i2,
                 smwpStructSeq i3,
                 smwpStringStructDict i4,
                 out smwpEnum o1,
                 out smwpStruct o2,
                 out smwpStructSeq o3,
                 out smwpStringStructDict o4)
        throws smwpException;
};

};
