// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use SingleModuleWithPackage types from (same) double module definitions

#include <SingleModuleWithPackage.ice>

module M
{
module N
{

const smwpEnum smwpTest10Constant = smwpE1;

struct smwpTest10Struct
{
    smwpEnum e;
    smwpStruct s;
    smwpStructSeq seq;
    smwpStringStructDict dict;
    smwpClass c;
    smwpInterface i;
}

sequence<smwpStruct> smwpTest10StructSeq;

dictionary<smwpStruct, smwpBaseClass> smwpTest10StructClassSeq;

interface smwpTest10Interface extends smwpInterface {}

exception smwpTest10Exception extends smwpException
{
    smwpEnum e1;
    smwpStruct s1;
    smwpStructSeq seq1;
    smwpStringStructDict dict1;
    smwpClass c1;
    smwpInterface i1;
}

class smwpTest10Class extends smwpBaseClass implements smwpBaseInterface
{
    smwpStruct
    smwpTest10Op1(smwpEnum i1,
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
    smwpTest10Op3(smwpEnum i1,
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
}

}
}
