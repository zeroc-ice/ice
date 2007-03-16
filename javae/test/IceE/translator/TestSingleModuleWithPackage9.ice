// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use SingleModuleWithPackage types from (different) double module definitions

#include <SingleModuleWithPackage.ice>

module T1
{
module T2
{

const ::M::smwpEnum smwpTest9Constant = ::M::smwpE1;

struct smwpTest9Struct
{
    ::M::smwpEnum e;
    ::M::smwpStruct s;
    ::M::smwpStructSeq seq;
    ::M::smwpStringStructDict dict;
};

sequence<::M::smwpStruct> smwpTest9StructSeq;

interface smwpTest9Interface extends ::M::smwpInterface {};

exception smwpTest9Exception extends ::M::smwpException
{
    ::M::smwpEnum e1;
    ::M::smwpStruct s1;
    ::M::smwpStructSeq seq1;
    ::M::smwpStringStructDict dict1;
};

class smwpTest9Class extends ::M::smwpBaseClass implements ::M::smwpBaseInterface
{
    ::M::smwpStruct
    smwpTest9Op1(::M::smwpEnum i1,
                 ::M::smwpStruct i2,
                 ::M::smwpStructSeq i3,
                 ::M::smwpStringStructDict i4,
                 out ::M::smwpEnum o1,
                 out ::M::smwpStruct o2,
                 out ::M::smwpStructSeq o3,
                 out ::M::smwpStringStructDict o4)
        throws ::M::smwpException;

    ["ami"]
    ::M::smwpStruct
    smwpTest9Op2(::M::smwpEnum i1,
                 ::M::smwpStruct i2,
                 ::M::smwpStructSeq i3,
                 ::M::smwpStringStructDict i4,
                 out ::M::smwpEnum o1,
                 out ::M::smwpStruct o2,
                 out ::M::smwpStructSeq o3,
                 out ::M::smwpStringStructDict o4)
        throws ::M::smwpException;

    ["amd"]
    ::M::smwpStruct
    smwpTest9Op3(::M::smwpEnum i1,
                 ::M::smwpStruct i2,
                 ::M::smwpStructSeq i3,
                 ::M::smwpStringStructDict i4,
                 out ::M::smwpEnum o1,
                 out ::M::smwpStruct o2,
                 out ::M::smwpStructSeq o3,
                 out ::M::smwpStringStructDict o4)
        throws ::M::smwpException;
};

};
};
