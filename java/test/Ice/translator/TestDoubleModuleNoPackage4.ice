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

// Test: use DoubleModuleNoPackage types from (same) single module definitions

#include <DoubleModuleNoPackage.ice>

module M1
{

const M2::dmnpEnum dmnpTest4Constant = M2::dmnpE1;

struct dmnpTest4Struct
{
    M2::dmnpEnum e;
    M2::dmnpStruct s;
    M2::dmnpStructSeq seq;
    M2::dmnpStringStructDict dict;
    M2::dmnpClass c;
    M2::dmnpInterface i;
};

sequence<M2::dmnpStruct> dmnpTest4StructSeq;

dictionary<M2::dmnpStruct, M2::dmnpBaseClass> dmnpTest4StructClassSeq;

interface dmnpTest4Interface extends M2::dmnpInterface {};

exception dmnpTest4Exception extends M2::dmnpException
{
    M2::dmnpEnum e1;
    M2::dmnpStruct s1;
    M2::dmnpStructSeq seq1;
    M2::dmnpStringStructDict dict1;
    M2::dmnpClass c1;
    M2::dmnpInterface i1;
};

class dmnpTest4Class extends M2::dmnpBaseClass implements M2::dmnpBaseInterface
{
    M2::dmnpStruct
    dmnpTest4Op1(M2::dmnpEnum i1,
                 M2::dmnpStruct i2,
                 M2::dmnpStructSeq i3,
                 M2::dmnpStringStructDict i4,
                 M2::dmnpInterface i5,
                 M2::dmnpClass i6,
                 out M2::dmnpEnum o1,
                 out M2::dmnpStruct o2,
                 out M2::dmnpStructSeq o3,
                 out M2::dmnpStringStructDict o4,
                 out M2::dmnpInterface o5,
                 out M2::dmnpClass o6)
        throws M2::dmnpException;

    ["ami"]
    M2::dmnpStruct
    dmnpTest4Op2(M2::dmnpEnum i1,
                 M2::dmnpStruct i2,
                 M2::dmnpStructSeq i3,
                 M2::dmnpStringStructDict i4,
                 M2::dmnpInterface i5,
                 M2::dmnpClass i6,
                 out M2::dmnpEnum o1,
                 out M2::dmnpStruct o2,
                 out M2::dmnpStructSeq o3,
                 out M2::dmnpStringStructDict o4,
                 out M2::dmnpInterface o5,
                 out M2::dmnpClass o6)
        throws M2::dmnpException;

    ["amd"]
    M2::dmnpStruct
    dmnpTest4Op3(M2::dmnpEnum i1,
                 M2::dmnpStruct i2,
                 M2::dmnpStructSeq i3,
                 M2::dmnpStringStructDict i4,
                 M2::dmnpInterface i5,
                 M2::dmnpClass i6,
                 out M2::dmnpEnum o1,
                 out M2::dmnpStruct o2,
                 out M2::dmnpStructSeq o3,
                 out M2::dmnpStringStructDict o4,
                 out M2::dmnpInterface o5,
                 out M2::dmnpClass o6)
        throws M2::dmnpException;
};

};
