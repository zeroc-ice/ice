// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use DoubleModuleNoPackage types from (same) single module definitions

#include <DoubleModuleNoPackage.ice>

module M1
{

const M1::M2::dmnpEnum dmnpTest4Constant = M1::M2::dmnpE1;

struct dmnpTest4Struct
{
    M1::M2::dmnpEnum e;
    M1::M2::dmnpStruct s;
    M1::M2::dmnpStructSeq seq;
    M1::M2::dmnpStringStructDict dict;
    M1::M2::dmnpClass c;
    M1::M2::dmnpInterface i;
}

sequence<M1::M2::dmnpStruct> dmnpTest4StructSeq;

dictionary<M1::M2::dmnpStruct, M1::M2::dmnpBaseClass> dmnpTest4StructClassSeq;

interface dmnpTest4Interface extends M1::M2::dmnpInterface {}

exception dmnpTest4Exception extends M1::M2::dmnpException
{
    M1::M2::dmnpEnum e1;
    M1::M2::dmnpStruct s1;
    M1::M2::dmnpStructSeq seq1;
    M1::M2::dmnpStringStructDict dict1;
    M1::M2::dmnpClass c1;
    M1::M2::dmnpInterface i1;
}

class dmnpTest4Class extends M1::M2::dmnpBaseClass implements M1::M2::dmnpBaseInterface
{
    M1::M2::dmnpStruct
    dmnpTest4Op1(M1::M2::dmnpEnum i1,
                 M1::M2::dmnpStruct i2,
                 M1::M2::dmnpStructSeq i3,
                 M1::M2::dmnpStringStructDict i4,
                 M1::M2::dmnpInterface i5,
                 M1::M2::dmnpClass i6,
                 out M1::M2::dmnpEnum o1,
                 out M1::M2::dmnpStruct o2,
                 out M1::M2::dmnpStructSeq o3,
                 out M1::M2::dmnpStringStructDict o4,
                 out M1::M2::dmnpInterface o5,
                 out M1::M2::dmnpClass o6)
        throws M1::M2::dmnpException;

    ["amd"]
    M1::M2::dmnpStruct
    dmnpTest4Op3(M1::M2::dmnpEnum i1,
                 M1::M2::dmnpStruct i2,
                 M1::M2::dmnpStructSeq i3,
                 M1::M2::dmnpStringStructDict i4,
                 M1::M2::dmnpInterface i5,
                 M1::M2::dmnpClass i6,
                 out M1::M2::dmnpEnum o1,
                 out M1::M2::dmnpStruct o2,
                 out M1::M2::dmnpStructSeq o3,
                 out M1::M2::dmnpStringStructDict o4,
                 out M1::M2::dmnpInterface o5,
                 out M1::M2::dmnpClass o6)
        throws M1::M2::dmnpException;
}

}
