// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use NoModuleWithPackage types from single module with (same) package definitions

#include <NoModuleWithPackage.ice>

[["java:package:nmwp"]]

module P
{

const nmwpEnum nmwpTest7Constant = nmwpE1;

struct nmwpTest7Struct
{
    nmwpEnum e;
    nmwpStruct s;
    nmwpStructSeq seq;
    nmwpStringStructDict dict;
    nmwpClass c;
    nmwpInterface i;
};

sequence<nmwpStruct> nmwpTest7StructSeq;

dictionary<nmwpStruct, nmwpBaseClass> nmwpTest7StructClassSeq;

interface nmwpTest7Interface extends nmwpInterface {};

exception nmwpTest7Exception extends nmwpException
{
    nmwpEnum e1;
    nmwpStruct s1;
    nmwpStructSeq seq1;
    nmwpStringStructDict dict1;
    nmwpClass c1;
    nmwpInterface i1;
};

class nmwpTest7Class extends nmwpBaseClass implements nmwpBaseInterface
{
    nmwpStruct
    nmwpTest7Op1(nmwpEnum i1,
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
    nmwpTest7Op2(nmwpEnum i1,
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
    nmwpTest7Op3(nmwpEnum i1,
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
