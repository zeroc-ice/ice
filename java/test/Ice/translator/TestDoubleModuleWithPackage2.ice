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

// Test: use DoubleModuleWithPackage types from (different) package definitions

#include <DoubleModuleWithPackage.ice>

[["java:package:dmwpTest2"]]

const ::M1::M2::dmwpEnum dmwpTest2Constant = ::M1::M2::dmwpE1;

struct dmwpTest2Struct
{
    ::M1::M2::dmwpEnum e;
    ::M1::M2::dmwpStruct s;
    ::M1::M2::dmwpStructSeq seq;
    ::M1::M2::dmwpStringStructDict dict;
    ::M1::M2::dmwpClass c;
    ::M1::M2::dmwpInterface i;
};

sequence<::M1::M2::dmwpStruct> dmwpTest2StructSeq;

dictionary<::M1::M2::dmwpStruct, ::M1::M2::dmwpBaseClass> dmwpTest2StructClassSeq;

interface dmwpTest2Interface extends ::M1::M2::dmwpInterface {};

exception dmwpTest2Exception extends ::M1::M2::dmwpException
{
    ::M1::M2::dmwpEnum e1;
    ::M1::M2::dmwpStruct s1;
    ::M1::M2::dmwpStructSeq seq1;
    ::M1::M2::dmwpStringStructDict dict1;
    ::M1::M2::dmwpClass c1;
    ::M1::M2::dmwpInterface i1;
};

class dmwpTest2Class extends ::M1::M2::dmwpBaseClass implements ::M1::M2::dmwpBaseInterface
{
    ::M1::M2::dmwpStruct
    dmwpTest2Op1(::M1::M2::dmwpEnum i1,
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

    ["ami"]
    ::M1::M2::dmwpStruct
    dmwpTest2Op2(::M1::M2::dmwpEnum i1,
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
    dmwpTest2Op3(::M1::M2::dmwpEnum i1,
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
};
