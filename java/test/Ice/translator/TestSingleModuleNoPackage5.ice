// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use SingleModuleNoPackage types from (same) single module with package definitions

#include <SingleModuleNoPackage.ice>

[["java:package:snmpTest5"]]

module M
{

const smnpEnum smnpTest5Constant = smnpE1;

struct smnpTest5Struct
{
    smnpEnum e;
    smnpStruct s;
    smnpStructSeq seq;
    smnpStringStructDict dict;
    smnpClass c;
    smnpInterface i;
};

sequence<smnpStruct> smnpTest5StructSeq;

dictionary<smnpStruct, smnpBaseClass> smnpTest5StructClassSeq;

interface smnpTest5Interface extends smnpInterface {};

exception smnpTest5Exception extends smnpException
{
    smnpEnum e1;
    smnpStruct s1;
    smnpStructSeq seq1;
    smnpStringStructDict dict1;
    smnpClass c1;
    smnpInterface i1;
};

class smnpTest5Class extends smnpBaseClass implements smnpBaseInterface
{
    smnpStruct
    smnpTest5Op1(smnpEnum i1,
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

    ["ami"]
    smnpStruct
    smnpTest5Op2(smnpEnum i1,
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
    smnpTest5Op3(smnpEnum i1,
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
