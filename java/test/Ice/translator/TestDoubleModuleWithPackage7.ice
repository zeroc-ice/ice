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

// Test: use DoubleModuleWithPackage types from (same) single module with (same) package definitions

#include <DoubleModuleWithPackage.ice>

[["java:package:dmwp"]]

module M1
{

const M2::dmwpEnum dmwpTest7Constant = M2::dmwpE1;

struct dmwpTest7Struct
{
    M2::dmwpEnum e;
    M2::dmwpStruct s;
    M2::dmwpStructSeq seq;
    M2::dmwpStringStructDict dict;
    M2::dmwpClass c;
    M2::dmwpInterface i;
};

sequence<M2::dmwpStruct> dmwpTest7StructSeq;

dictionary<M2::dmwpStruct, M2::dmwpBaseClass> dmwpTest7StructClassSeq;

interface dmwpTest7Interface extends M2::dmwpInterface {};

exception dmwpTest7Exception extends M2::dmwpException
{
    M2::dmwpEnum e1;
    M2::dmwpStruct s1;
    M2::dmwpStructSeq seq1;
    M2::dmwpStringStructDict dict1;
    M2::dmwpClass c1;
    M2::dmwpInterface i1;
};

class dmwpTest7Class extends M2::dmwpBaseClass implements M2::dmwpBaseInterface
{
    M2::dmwpStruct
    dmwpTest7Op1(M2::dmwpEnum i1,
                 M2::dmwpStruct i2,
                 M2::dmwpStructSeq i3,
                 M2::dmwpStringStructDict i4,
                 M2::dmwpInterface i5,
                 M2::dmwpClass i6,
                 out M2::dmwpEnum o1,
                 out M2::dmwpStruct o2,
                 out M2::dmwpStructSeq o3,
                 out M2::dmwpStringStructDict o4,
                 out M2::dmwpInterface o5,
                 out M2::dmwpClass o6)
        throws M2::dmwpException;

    ["ami"]
    M2::dmwpStruct
    dmwpTest7Op2(M2::dmwpEnum i1,
                 M2::dmwpStruct i2,
                 M2::dmwpStructSeq i3,
                 M2::dmwpStringStructDict i4,
                 M2::dmwpInterface i5,
                 M2::dmwpClass i6,
                 out M2::dmwpEnum o1,
                 out M2::dmwpStruct o2,
                 out M2::dmwpStructSeq o3,
                 out M2::dmwpStringStructDict o4,
                 out M2::dmwpInterface o5,
                 out M2::dmwpClass o6)
        throws M2::dmwpException;

    ["amd"]
    M2::dmwpStruct
    dmwpTest7Op3(M2::dmwpEnum i1,
                 M2::dmwpStruct i2,
                 M2::dmwpStructSeq i3,
                 M2::dmwpStringStructDict i4,
                 M2::dmwpInterface i5,
                 M2::dmwpClass i6,
                 out M2::dmwpEnum o1,
                 out M2::dmwpStruct o2,
                 out M2::dmwpStructSeq o3,
                 out M2::dmwpStringStructDict o4,
                 out M2::dmwpInterface o5,
                 out M2::dmwpClass o6)
        throws M2::dmwpException;
};

};
