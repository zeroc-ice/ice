// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// Test: use NoModuleNoPackage types from top-level definitions. Note that
// we can *only* test from top-level definitions, because it's not possible
// to refer to top-level Java classes from within packaged types.

#include <NoModuleNoPackage.ice>

const nmnpEnum nmnpTestConstant = nmnpE1;

struct nmnpTestStruct
{
    nmnpEnum e;
    nmnpStruct s;
    nmnpStructSeq seq;
    nmnpStringStructDict dict;
    nmnpClass c;
    nmnpInterface i;
};

sequence<nmnpStruct> nmnpTestStructSeq;

dictionary<nmnpStruct, nmnpBaseClass> nmnpTestStructClassSeq;

interface nmnpTestInterface extends nmnpInterface {};

exception nmnpTestException extends nmnpException
{
    nmnpEnum e1;
    nmnpStruct s1;
    nmnpStructSeq seq1;
    nmnpStringStructDict dict1;
    nmnpClass c1;
    nmnpInterface i1;
};

class nmnpTestClass extends nmnpBaseClass implements nmnpBaseInterface
{
    nmnpStruct
    nmnpTestOp1(nmnpEnum i1,
                nmnpStruct i2,
                nmnpStructSeq i3,
                nmnpStringStructDict i4,
                nmnpInterface i5,
                nmnpClass i6,
                out nmnpEnum o1,
                out nmnpStruct o2,
                out nmnpStructSeq o3,
                out nmnpStringStructDict o4,
                out nmnpInterface o5,
                out nmnpClass o6)
        throws nmnpException;

    ["ami"]
    nmnpStruct
    nmnpTestOp2(nmnpEnum i1,
                nmnpStruct i2,
                nmnpStructSeq i3,
                nmnpStringStructDict i4,
                nmnpInterface i5,
                nmnpClass i6,
                out nmnpEnum o1,
                out nmnpStruct o2,
                out nmnpStructSeq o3,
                out nmnpStringStructDict o4,
                out nmnpInterface o5,
                out nmnpClass o6)
        throws nmnpException;

    ["amd"]
    nmnpStruct
    nmnpTestOp3(nmnpEnum i1,
                nmnpStruct i2,
                nmnpStructSeq i3,
                nmnpStringStructDict i4,
                nmnpInterface i5,
                nmnpClass i6,
                out nmnpEnum o1,
                out nmnpStruct o2,
                out nmnpStructSeq o3,
                out nmnpStringStructDict o4,
                out nmnpInterface o5,
                out nmnpClass o6)
        throws nmnpException;
};
