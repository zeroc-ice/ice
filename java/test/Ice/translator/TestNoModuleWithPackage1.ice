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


// Test: use NoModuleWithPackage types from top-level definitions

#include <NoModuleWithPackage.ice>

const nmwpEnum nmwpTest1Constant = nmwpE1;

struct nmwpTest1Struct
{
    nmwpEnum e;
    nmwpStruct s;
    nmwpStructSeq seq;
    nmwpStringStructDict dict;
    nmwpClass c;
    nmwpInterface i;
};

sequence<nmwpStruct> nmwpTest1StructSeq;

dictionary<nmwpStruct, nmwpBaseClass> nmwpTest1StructClassSeq;

interface nmwpTest1Interface extends nmwpInterface {};

exception nmwpTest1Exception extends nmwpException
{
    nmwpEnum e1;
    nmwpStruct s1;
    nmwpStructSeq seq1;
    nmwpStringStructDict dict1;
    nmwpClass c1;
    nmwpInterface i1;
};

class nmwpTest1Class extends nmwpBaseClass implements nmwpBaseInterface
{
    nmwpStruct
    nmwpTest1Op1(nmwpEnum i1,
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
    nmwpTest1Op2(nmwpEnum i1,
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
    nmwpTest1Op3(nmwpEnum i1,
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
