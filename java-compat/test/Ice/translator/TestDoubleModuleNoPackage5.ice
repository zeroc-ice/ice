// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use DoubleModuleNoPackage types from (different) double module definitions

#include <DoubleModuleNoPackage.ice>

module T1
{
module T2
{

const ::M1::M2::dmnpEnum dmnpTest5Constant = ::M1::M2::dmnpE1;

struct dmnpTest5Struct
{
    ::M1::M2::dmnpEnum e;
    ::M1::M2::dmnpStruct s;
    ::M1::M2::dmnpStructSeq seq;
    ::M1::M2::dmnpStringStructDict dict;
    ::M1::M2::dmnpClass c;
    ::M1::M2::dmnpInterface i;
}

sequence<::M1::M2::dmnpStruct> dmnpTest5StructSeq;

dictionary<::M1::M2::dmnpStruct, ::M1::M2::dmnpBaseClass> dmnpTest5StructClassSeq;

interface dmnpTest5Interface extends ::M1::M2::dmnpInterface {}

exception dmnpTest5Exception extends ::M1::M2::dmnpException
{
    ::M1::M2::dmnpEnum e1;
    ::M1::M2::dmnpStruct s1;
    ::M1::M2::dmnpStructSeq seq1;
    ::M1::M2::dmnpStringStructDict dict1;
    ::M1::M2::dmnpClass c1;
    ::M1::M2::dmnpInterface i1;
}

class dmnpTest5Class extends ::M1::M2::dmnpBaseClass implements ::M1::M2::dmnpBaseInterface
{
    ::M1::M2::dmnpStruct
    dmnpTest5Op1(::M1::M2::dmnpEnum i1,
                 ::M1::M2::dmnpStruct i2,
                 ::M1::M2::dmnpStructSeq i3,
                 ::M1::M2::dmnpStringStructDict i4,
                 ::M1::M2::dmnpInterface i5,
                 ::M1::M2::dmnpClass i6,
                 out ::M1::M2::dmnpEnum o1,
                 out ::M1::M2::dmnpStruct o2,
                 out ::M1::M2::dmnpStructSeq o3,
                 out ::M1::M2::dmnpStringStructDict o4,
                 out ::M1::M2::dmnpInterface o5,
                 out ::M1::M2::dmnpClass o6)
        throws ::M1::M2::dmnpException;

    ["amd"]
    ::M1::M2::dmnpStruct
    dmnpTest5Op3(::M1::M2::dmnpEnum i1,
                 ::M1::M2::dmnpStruct i2,
                 ::M1::M2::dmnpStructSeq i3,
                 ::M1::M2::dmnpStringStructDict i4,
                 ::M1::M2::dmnpInterface i5,
                 ::M1::M2::dmnpClass i6,
                 out ::M1::M2::dmnpEnum o1,
                 out ::M1::M2::dmnpStruct o2,
                 out ::M1::M2::dmnpStructSeq o3,
                 out ::M1::M2::dmnpStringStructDict o4,
                 out ::M1::M2::dmnpInterface o5,
                 out ::M1::M2::dmnpClass o6)
        throws ::M1::M2::dmnpException;
}

}
}
