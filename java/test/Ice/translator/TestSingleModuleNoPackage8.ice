// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use SingleModuleNoPackage types from (same) double module definitions with package

#include <SingleModuleNoPackage.ice>

[["java:package:smnpTest8"]]

module M
{
module N
{

const smnpEnum smnpTest8Constant = smnpE1;

struct smnpTest8Struct
{
    smnpEnum e;
    smnpStruct s;
    smnpStructSeq seq;
    smnpStringStructDict dict;
    smnpClass c;
    smnpInterface i;
};

sequence<smnpStruct> smnpTest8StructSeq;

dictionary<smnpStruct, smnpBaseClass> smnpTest8StructClassSeq;

interface smnpTest8Interface extends smnpInterface {};

exception smnpTest8Exception extends smnpException
{
    smnpEnum e1;
    smnpStruct s1;
    smnpStructSeq seq1;
    smnpStringStructDict dict1;
    smnpClass c1;
    smnpInterface i1;
};

class smnpTest8Class extends smnpBaseClass implements smnpBaseInterface
{
    smnpStruct
    smnpTest8Op1(smnpEnum i1,
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
    smnpTest8Op2(smnpEnum i1,
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
    smnpTest8Op3(smnpEnum i1,
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
};
