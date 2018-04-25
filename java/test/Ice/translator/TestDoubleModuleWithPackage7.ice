// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use DoubleModuleWithPackage types from (same) single module with (same) package definitions

#include <DoubleModuleWithPackage.ice>

[["java:package:dmwp"]]

module M1
{

const ::M1::M2::dmwpEnum dmwpTest7Constant = ::M1::M2::dmwpE1;

struct dmwpTest7Struct
{
    ::M1::M2::dmwpEnum e;
    ::M1::M2::dmwpStruct s;
    ::M1::M2::dmwpStructSeq seq;
    ::M1::M2::dmwpStringStructDict dict;
    ::M1::M2::dmwpClass c;
    ::M1::M2::dmwpInterface i;
}

sequence<::M1::M2::dmwpStruct> dmwpTest7StructSeq;

dictionary<::M1::M2::dmwpStruct, ::M1::M2::dmwpBaseClass> dmwpTest7StructClassSeq;

interface dmwpTest7Interface extends ::M1::M2::dmwpInterface {}

exception dmwpTest7Exception extends ::M1::M2::dmwpException
{
    ::M1::M2::dmwpEnum e1;
    ::M1::M2::dmwpStruct s1;
    ::M1::M2::dmwpStructSeq seq1;
    ::M1::M2::dmwpStringStructDict dict1;
    ::M1::M2::dmwpClass c1;
    ::M1::M2::dmwpInterface i1;
}

class dmwpTest7Class extends ::M1::M2::dmwpBaseClass implements ::M1::M2::dmwpBaseInterface
{
    ::M1::M2::dmwpStruct
    dmwpTest7Op1(::M1::M2::dmwpEnum i1,
                 ::M1::M2::dmwpStruct i2,
                 ::M1::M2::dmwpStructSeq i3,
                 ::M1::M2::dmwpStringStructDict i4,
                 ::M1::M2::dmwpInterface i5,
                 ::M1::M2::dmwpClass i6,
                 out ::M1::M2::dmwpEnum o1,
                 out ::M1::M2::dmwpStruct o2,
                 out ::M1::M2::dmwpStructSeq o3,
                 out ::M1::M2::dmwpStringStructDict o4,
                 out ::M1::M2::dmwpInterface o5,
                 out ::M1::M2::dmwpClass o6)
        throws ::M1::M2::dmwpException;

    ["amd"]
    ::M1::M2::dmwpStruct
    dmwpTest7Op3(::M1::M2::dmwpEnum i1,
                 ::M1::M2::dmwpStruct i2,
                 ::M1::M2::dmwpStructSeq i3,
                 ::M1::M2::dmwpStringStructDict i4,
                 ::M1::M2::dmwpInterface i5,
                 ::M1::M2::dmwpClass i6,
                 out ::M1::M2::dmwpEnum o1,
                 out ::M1::M2::dmwpStruct o2,
                 out ::M1::M2::dmwpStructSeq o3,
                 out ::M1::M2::dmwpStringStructDict o4,
                 out ::M1::M2::dmwpInterface o5,
                 out ::M1::M2::dmwpClass o6)
        throws ::M1::M2::dmwpException;
}

}
