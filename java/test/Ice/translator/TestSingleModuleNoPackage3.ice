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

// Test: use SingleModuleNoPackage types from (different) single module definitions

#include <SingleModuleNoPackage.ice>

module T1
{

const ::M::smnpEnum smnpTest3Constant = ::M::smnpE1;

struct smnpTest3Struct
{
    ::M::smnpEnum e;
    ::M::smnpStruct s;
    ::M::smnpStructSeq seq;
    ::M::smnpStringStructDict dict;
    ::M::smnpClass c;
    ::M::smnpInterface i;
};

sequence<::M::smnpStruct> smnpTest3StructSeq;

dictionary<::M::smnpStruct, ::M::smnpBaseClass> smnpTest3StructClassSeq;

interface smnpTest3Interface extends ::M::smnpInterface {};

exception smnpTest3Exception extends ::M::smnpException
{
    ::M::smnpEnum e1;
    ::M::smnpStruct s1;
    ::M::smnpStructSeq seq1;
    ::M::smnpStringStructDict dict1;
    ::M::smnpClass c1;
    ::M::smnpInterface i1;
};

class smnpTest3Class extends ::M::smnpBaseClass implements ::M::smnpBaseInterface
{
    ::M::smnpStruct
    smnpTest3Op1(::M::smnpEnum i1,
                 ::M::smnpStruct i2,
                 ::M::smnpStructSeq i3,
                 ::M::smnpStringStructDict i4,
                 ::M::smnpInterface i5,
                 ::M::smnpClass i6,
                 out ::M::smnpEnum o1,
                 out ::M::smnpStruct o2,
                 out ::M::smnpStructSeq o3,
                 out ::M::smnpStringStructDict o4,
                 out ::M::smnpInterface o5,
                 out ::M::smnpClass o6)
        throws ::M::smnpException;

    ["ami"]
    ::M::smnpStruct
    smnpTest3Op2(::M::smnpEnum i1,
                 ::M::smnpStruct i2,
                 ::M::smnpStructSeq i3,
                 ::M::smnpStringStructDict i4,
                 ::M::smnpInterface i5,
                 ::M::smnpClass i6,
                 out ::M::smnpEnum o1,
                 out ::M::smnpStruct o2,
                 out ::M::smnpStructSeq o3,
                 out ::M::smnpStringStructDict o4,
                 out ::M::smnpInterface o5,
                 out ::M::smnpClass o6)
        throws ::M::smnpException;

    ["amd"]
    ::M::smnpStruct
    smnpTest3Op3(::M::smnpEnum i1,
                 ::M::smnpStruct i2,
                 ::M::smnpStructSeq i3,
                 ::M::smnpStringStructDict i4,
                 ::M::smnpInterface i5,
                 ::M::smnpClass i6,
                 out ::M::smnpEnum o1,
                 out ::M::smnpStruct o2,
                 out ::M::smnpStructSeq o3,
                 out ::M::smnpStringStructDict o4,
                 out ::M::smnpInterface o5,
                 out ::M::smnpClass o6)
        throws ::M::smnpException;
};

};
