// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use SingleModuleWithPackage types from (different) single module with (different) package definitions

#include <SingleModuleWithPackage.ice>

[["java:package:smwpTest6"]]

module T1
{

const M::smwpEnum smwpTest6Constant = M::smwpE1;

struct smwpTest6Struct
{
    M::smwpEnum e;
    M::smwpStruct s;
    M::smwpStructSeq seq;
    M::smwpStringStructDict dict;
};

sequence<M::smwpStruct> smwpTest6StructSeq;

interface smwpTest6Interface extends M::smwpInterface {};

exception smwpTest6Exception extends M::smwpException
{
    M::smwpEnum e1;
    M::smwpStruct s1;
    M::smwpStructSeq seq1;
    M::smwpStringStructDict dict1;
};

class smwpTest6Class extends M::smwpBaseClass implements M::smwpBaseInterface
{
    M::smwpStruct
    smwpTest6Op1(M::smwpEnum i1,
                 M::smwpStruct i2,
                 M::smwpStructSeq i3,
                 M::smwpStringStructDict i4,
                 out M::smwpEnum o1,
                 out M::smwpStruct o2,
                 out M::smwpStructSeq o3,
                 out M::smwpStringStructDict o4)
        throws M::smwpException;

    ["ami"]
    M::smwpStruct
    smwpTest6Op2(M::smwpEnum i1,
                 M::smwpStruct i2,
                 M::smwpStructSeq i3,
                 M::smwpStringStructDict i4,
                 out M::smwpEnum o1,
                 out M::smwpStruct o2,
                 out M::smwpStructSeq o3,
                 out M::smwpStringStructDict o4)
        throws M::smwpException;

    ["amd"]
    M::smwpStruct
    smwpTest6Op3(M::smwpEnum i1,
                 M::smwpStruct i2,
                 M::smwpStructSeq i3,
                 M::smwpStringStructDict i4,
                 out M::smwpEnum o1,
                 out M::smwpStruct o2,
                 out M::smwpStructSeq o3,
                 out M::smwpStringStructDict o4)
        throws M::smwpException;
};

};
