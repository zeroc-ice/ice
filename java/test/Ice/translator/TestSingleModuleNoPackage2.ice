// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use SingleModuleNoPackage types from package definitions

#include <SingleModuleNoPackage.ice>

[["java:package:smnpTest2"]]

const M::smnpEnum smnpTest2Constant = M::smnpE1;

struct smnpTest2Struct
{
    M::smnpEnum e;
    M::smnpStruct s;
    M::smnpStructSeq seq;
    M::smnpStringStructDict dict;
    M::smnpClass c;
    M::smnpInterface i;
};

sequence<M::smnpStruct> smnpTest2StructSeq;

dictionary<M::smnpStruct, M::smnpBaseClass> smnpTest2StructClassSeq;

interface smnpTest2Interface extends M::smnpInterface {};

exception smnpTest2Exception extends M::smnpException
{
    M::smnpEnum e1;
    M::smnpStruct s1;
    M::smnpStructSeq seq1;
    M::smnpStringStructDict dict1;
    M::smnpClass c1;
    M::smnpInterface i1;
};

class smnpTest2Class extends M::smnpBaseClass implements M::smnpBaseInterface
{
    M::smnpStruct
    smnpTest2Op1(M::smnpEnum i1,
                 M::smnpStruct i2,
                 M::smnpStructSeq i3,
                 M::smnpStringStructDict i4,
                 M::smnpInterface i5,
                 M::smnpClass i6,
                 out M::smnpEnum o1,
                 out M::smnpStruct o2,
                 out M::smnpStructSeq o3,
                 out M::smnpStringStructDict o4,
                 out M::smnpInterface o5,
                 out M::smnpClass o6)
        throws M::smnpException;

    ["ami"]
    M::smnpStruct
    smnpTest2Op2(M::smnpEnum i1,
                 M::smnpStruct i2,
                 M::smnpStructSeq i3,
                 M::smnpStringStructDict i4,
                 M::smnpInterface i5,
                 M::smnpClass i6,
                 out M::smnpEnum o1,
                 out M::smnpStruct o2,
                 out M::smnpStructSeq o3,
                 out M::smnpStringStructDict o4,
                 out M::smnpInterface o5,
                 out M::smnpClass o6)
        throws M::smnpException;

    ["amd"]
    M::smnpStruct
    smnpTest2Op3(M::smnpEnum i1,
                 M::smnpStruct i2,
                 M::smnpStructSeq i3,
                 M::smnpStringStructDict i4,
                 M::smnpInterface i5,
                 M::smnpClass i6,
                 out M::smnpEnum o1,
                 out M::smnpStruct o2,
                 out M::smnpStructSeq o3,
                 out M::smnpStringStructDict o4,
                 out M::smnpInterface o5,
                 out M::smnpClass o6)
        throws M::smnpException;
};
