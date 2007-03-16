// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use DoubleModuleWithPackage types from (same) double module with (same) package definitions

#include <DoubleModuleWithPackage.ice>

[["java:package:dmwp"]]

module M1
{
module M2
{

const dmwpEnum dmwpTest10Constant = dmwpE1;

struct dmwpTest10Struct
{
    dmwpEnum e;
    dmwpStruct s;
    dmwpStructSeq seq;
    dmwpStringStructDict dict;
};

sequence<dmwpStruct> dmwpTest10StructSeq;

interface dmwpTest10Interface extends dmwpInterface {};

exception dmwpTest10Exception extends dmwpException
{
    dmwpEnum e1;
    dmwpStruct s1;
    dmwpStructSeq seq1;
    dmwpStringStructDict dict1;
};

class dmwpTest10Class extends dmwpBaseClass implements dmwpBaseInterface
{
    dmwpStruct
    dmwpTest10Op1(dmwpEnum i1,
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
    dmwpTest10Op2(dmwpEnum i1,
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
    dmwpTest10Op3(dmwpEnum i1,
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
