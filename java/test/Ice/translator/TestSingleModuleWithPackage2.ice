// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


// Test: use SingleModuleWithPackage types from (different) package definitions

#include <SingleModuleWithPackage.ice>

[["java:package:smwpTest2"]]

const M::smwpEnum smwpTest2Constant = M::smwpE1;

struct smwpTest2Struct
{
    M::smwpEnum e;
    M::smwpStruct s;
    M::smwpStructSeq seq;
    M::smwpStringStructDict dict;
    M::smwpClass c;
    M::smwpInterface i;
};

sequence<M::smwpStruct> smwpTest2StructSeq;

dictionary<M::smwpStruct, M::smwpBaseClass> smwpTest2StructClassSeq;

interface smwpTest2Interface extends M::smwpInterface {};

exception smwpTest2Exception extends M::smwpException
{
    M::smwpEnum e1;
    M::smwpStruct s1;
    M::smwpStructSeq seq1;
    M::smwpStringStructDict dict1;
    M::smwpClass c1;
    M::smwpInterface i1;
};

class smwpTest2Class extends M::smwpBaseClass implements M::smwpBaseInterface
{
    M::smwpStruct
    smwpTest2Op1(M::smwpEnum i1,
                 M::smwpStruct i2,
                 M::smwpStructSeq i3,
                 M::smwpStringStructDict i4,
                 M::smwpInterface i5,
                 M::smwpClass i6,
                 out M::smwpEnum o1,
                 out M::smwpStruct o2,
                 out M::smwpStructSeq o3,
                 out M::smwpStringStructDict o4,
                 out M::smwpInterface o5,
                 out M::smwpClass o6)
        throws M::smwpException;

    ["ami"]
    M::smwpStruct
    smwpTest2Op2(M::smwpEnum i1,
                 M::smwpStruct i2,
                 M::smwpStructSeq i3,
                 M::smwpStringStructDict i4,
                 M::smwpInterface i5,
                 M::smwpClass i6,
                 out M::smwpEnum o1,
                 out M::smwpStruct o2,
                 out M::smwpStructSeq o3,
                 out M::smwpStringStructDict o4,
                 out M::smwpInterface o5,
                 out M::smwpClass o6)
        throws M::smwpException;

    ["amd"]
    M::smwpStruct
    smwpTest2Op3(M::smwpEnum i1,
                 M::smwpStruct i2,
                 M::smwpStructSeq i3,
                 M::smwpStringStructDict i4,
                 M::smwpInterface i5,
                 M::smwpClass i6,
                 out M::smwpEnum o1,
                 out M::smwpStruct o2,
                 out M::smwpStructSeq o3,
                 out M::smwpStringStructDict o4,
                 out M::smwpInterface o5,
                 out M::smwpClass o6)
        throws M::smwpException;
};
