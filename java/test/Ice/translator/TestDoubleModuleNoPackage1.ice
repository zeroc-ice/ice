// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

// Test: use DoubleModuleNoPackage types from top-level definitions

#include <DoubleModuleNoPackage.ice>

const ::M1::M2::dmnpEnum dmnpTest1Constant = ::M1::M2::dmnpE1;

struct dmnpTest1Struct
{
    ::M1::M2::dmnpEnum e;
    ::M1::M2::dmnpStruct s;
    ::M1::M2::dmnpStructSeq seq;
    ::M1::M2::dmnpStringStructDict dict;
    ::M1::M2::dmnpClass c;
    ::M1::M2::dmnpInterface i;
};

sequence<::M1::M2::dmnpStruct> dmnpTest1StructSeq;

dictionary<::M1::M2::dmnpStruct, ::M1::M2::dmnpBaseClass> dmnpTest1StructClassSeq;

interface dmnpTest1Interface extends ::M1::M2::dmnpInterface {};

exception dmnpTest1Exception extends ::M1::M2::dmnpException
{
    ::M1::M2::dmnpEnum e1;
    ::M1::M2::dmnpStruct s1;
    ::M1::M2::dmnpStructSeq seq1;
    ::M1::M2::dmnpStringStructDict dict1;
    ::M1::M2::dmnpClass c1;
    ::M1::M2::dmnpInterface i1;
};

class dmnpTest1Class extends ::M1::M2::dmnpBaseClass implements ::M1::M2::dmnpBaseInterface
{
    ::M1::M2::dmnpStruct
    dmnpTest1Op1(::M1::M2::dmnpEnum i1,
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

    ["ami"]
    ::M1::M2::dmnpStruct
    dmnpTest1Op2(::M1::M2::dmnpEnum i1,
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
    dmnpTest1Op3(::M1::M2::dmnpEnum i1,
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
};
