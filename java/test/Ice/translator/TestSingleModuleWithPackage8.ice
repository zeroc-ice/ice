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

// Test: use SingleModuleWithPackage types from (same) single module with (same) package definitions

#include <SingleModuleWithPackage.ice>

[["java:package:smwp"]]

module M
{

const smwpEnum smwpTest8Constant = smwpE1;

struct smwpTest8Struct
{
    smwpEnum e;
    smwpStruct s;
    smwpStructSeq seq;
    smwpStringStructDict dict;
    smwpClass c;
    smwpInterface i;
};

sequence<smwpStruct> smwpTest8StructSeq;

dictionary<smwpStruct, smwpBaseClass> smwpTest8StructClassSeq;

interface smwpTest8Interface extends smwpInterface {};

exception smwpTest8Exception extends smwpException
{
    smwpEnum e1;
    smwpStruct s1;
    smwpStructSeq seq1;
    smwpStringStructDict dict1;
    smwpClass c1;
    smwpInterface i1;
};

class smwpTest8Class extends smwpBaseClass implements smwpBaseInterface
{
    smwpStruct
    smwpTest8Op1(smwpEnum i1,
                 smwpStruct i2,
                 smwpStructSeq i3,
                 smwpStringStructDict i4,
                 smwpInterface i5,
                 smwpClass i6,
                 out smwpEnum o1,
                 out smwpStruct o2,
                 out smwpStructSeq o3,
                 out smwpStringStructDict o4,
                 out smwpInterface o5,
                 out smwpClass o6)
        throws smwpException;

    ["ami"]
    smwpStruct
    smwpTest8Op2(smwpEnum i1,
                 smwpStruct i2,
                 smwpStructSeq i3,
                 smwpStringStructDict i4,
                 smwpInterface i5,
                 smwpClass i6,
                 out smwpEnum o1,
                 out smwpStruct o2,
                 out smwpStructSeq o3,
                 out smwpStringStructDict o4,
                 out smwpInterface o5,
                 out smwpClass o6)
        throws smwpException;

    ["amd"]
    smwpStruct
    smwpTest8Op3(smwpEnum i1,
                 smwpStruct i2,
                 smwpStructSeq i3,
                 smwpStringStructDict i4,
                 smwpInterface i5,
                 smwpClass i6,
                 out smwpEnum o1,
                 out smwpStruct o2,
                 out smwpStructSeq o3,
                 out smwpStringStructDict o4,
                 out smwpInterface o5,
                 out smwpClass o6)
        throws smwpException;
};

};
