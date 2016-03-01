// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use SingleModuleNoPackage types from (same) single module definitions

#include <SingleModuleNoPackage.ice>

module M
{

const smnpEnum smnpTest4Constant = smnpE1;

struct smnpTest4Struct
{
    smnpEnum e;
    smnpStruct s;
    smnpStructSeq seq;
    smnpStringStructDict dict;
    smnpClass c;
    smnpInterface i;
};

sequence<smnpStruct> smnpTest4StructSeq;

dictionary<smnpStruct, smnpBaseClass> smnpTest4StructClassSeq;

interface smnpTest4Interface extends smnpInterface {};

exception smnpTest4Exception extends smnpException
{
    smnpEnum e1;
    smnpStruct s1;
    smnpStructSeq seq1;
    smnpStringStructDict dict1;
    smnpClass c1;
    smnpInterface i1;
};

class smnpTest4Class extends smnpBaseClass implements smnpBaseInterface
{
    smnpStruct
    smnpTest4Op1(smnpEnum i1,
                 smnpStruct i2,
                 smnpStructSeq i3,
                 smnpStringStructDict i4,
                 smnpInterface i5,
                 smnpClass i6,
                 out smnpEnum o1,
                 out smnpStruct o2,
                 out smnpStructSeq o3,
                 out smnpStringStructDict o4,
                 out smnpInterface o5,
                 out smnpClass o6)
        throws smnpException;

    ["amd"]
    smnpStruct
    smnpTest4Op3(smnpEnum i1,
                 smnpStruct i2,
                 smnpStructSeq i3,
                 smnpStringStructDict i4,
                 smnpInterface i5,
                 smnpClass i6,
                 out smnpEnum o1,
                 out smnpStruct o2,
                 out smnpStructSeq o3,
                 out smnpStringStructDict o4,
                 out smnpInterface o5,
                 out smnpClass o6)
        throws smnpException;
};

};
