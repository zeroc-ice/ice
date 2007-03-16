// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use DoubleModuleWithPackage types from (same) double module definitions

#include <DoubleModuleWithPackage.ice>

module M1
{
module M2
{

const dmwpEnum dmwpTest9Constant = dmwpE1;

struct dmwpTest9Struct
{
    dmwpEnum e;
    dmwpStruct s;
    dmwpStructSeq seq;
    dmwpStringStructDict dict;
};

sequence<dmwpStruct> dmwpTest9StructSeq;

interface dmwpTest9Interface extends dmwpInterface {};

exception dmwpTest9Exception extends dmwpException
{
    dmwpEnum e1;
    dmwpStruct s1;
    dmwpStructSeq seq1;
    dmwpStringStructDict dict1;
};

class dmwpTest9Class extends dmwpBaseClass implements dmwpBaseInterface
{
    dmwpStruct
    dmwpTest9Op1(dmwpEnum i1,
                 dmwpStruct i2,
                 dmwpStructSeq i3,
                 dmwpStringStructDict i4,
                 out dmwpEnum o1,
                 out dmwpStruct o2,
                 out dmwpStructSeq o3,
                 out dmwpStringStructDict o4)
        throws dmwpException;

    ["ami"]
    dmwpStruct
    dmwpTest9Op2(dmwpEnum i1,
                 dmwpStruct i2,
                 dmwpStructSeq i3,
                 dmwpStringStructDict i4,
                 out dmwpEnum o1,
                 out dmwpStruct o2,
                 out dmwpStructSeq o3,
                 out dmwpStringStructDict o4)
        throws dmwpException;

    ["amd"]
    dmwpStruct
    dmwpTest9Op3(dmwpEnum i1,
                 dmwpStruct i2,
                 dmwpStructSeq i3,
                 dmwpStringStructDict i4,
                 out dmwpEnum o1,
                 out dmwpStruct o2,
                 out dmwpStructSeq o3,
                 out dmwpStringStructDict o4)
        throws dmwpException;
};

};
};
