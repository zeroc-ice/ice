// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use NoModuleWithPackage types from double module with (different) package definitions

#include <NoModuleWithPackage.ice>

[["java:package:nmwpTest8"]]

module P
{
module Q
{

const nmwpEnum nmwpTest8Constant = nmwpE1;

struct nmwpTest8Struct
{
    nmwpEnum e;
    nmwpStruct s;
    nmwpStructSeq seq;
    nmwpStringStructDict dict;
    nmwpClass c;
    nmwpInterface i;
};

sequence<nmwpStruct> nmwpTest8StructSeq;

dictionary<nmwpStruct, nmwpBaseClass> nmwpTest8StructClassSeq;

interface nmwpTest8Interface extends nmwpInterface {};

exception nmwpTest8Exception extends nmwpException
{
    nmwpEnum e1;
    nmwpStruct s1;
    nmwpStructSeq seq1;
    nmwpStringStructDict dict1;
    nmwpClass c1;
    nmwpInterface i1;
};

class nmwpTest8Class extends nmwpBaseClass implements nmwpBaseInterface
{
    nmwpStruct
    nmwpTest8Op1(nmwpEnum i1,
                 nmwpStruct i2,
                 nmwpStructSeq i3,
                 nmwpStringStructDict i4,
                 nmwpInterface i5,
                 nmwpClass i6,
                 out nmwpEnum o1,
                 out nmwpStruct o2,
                 out nmwpStructSeq o3,
                 out nmwpStringStructDict o4,
                 out nmwpInterface o5,
                 out nmwpClass o6)
        throws nmwpException;

    ["ami"]
    nmwpStruct
    nmwpTest8Op2(nmwpEnum i1,
                 nmwpStruct i2,
                 nmwpStructSeq i3,
                 nmwpStringStructDict i4,
                 nmwpInterface i5,
                 nmwpClass i6,
                 out nmwpEnum o1,
                 out nmwpStruct o2,
                 out nmwpStructSeq o3,
                 out nmwpStringStructDict o4,
                 out nmwpInterface o5,
                 out nmwpClass o6)
        throws nmwpException;

    ["amd"]
    nmwpStruct
    nmwpTest8Op3(nmwpEnum i1,
                 nmwpStruct i2,
                 nmwpStructSeq i3,
                 nmwpStringStructDict i4,
                 nmwpInterface i5,
                 nmwpClass i6,
                 out nmwpEnum o1,
                 out nmwpStruct o2,
                 out nmwpStructSeq o3,
                 out nmwpStringStructDict o4,
                 out nmwpInterface o5,
                 out nmwpClass o6)
        throws nmwpException;
};

};
};
