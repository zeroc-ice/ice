//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Test: use SingleModuleWithPackage types from (different) single module with (different) package definitions

#include <SingleModuleWithPackage.ice>

[["java:package:smwpTest6"]]

module T1
{

const M::smwpEnum smwpTest6Constant = M::smwpE1;

struct smwpTest6Struct
{
    M::smwpEnum e;
    M::smwpStruct s;
    M::smwpStructSeq seq;
    M::smwpStringStructDict dict;
    M::smwpClass c;
    M::smwpInterface i;
}

sequence<M::smwpStruct> smwpTest6StructSeq;

dictionary<M::smwpStruct, M::smwpBaseClass> smwpTest6StructClassSeq;

interface smwpTest6Interface extends M::smwpInterface {}

exception smwpTest6Exception extends M::smwpException
{
    M::smwpEnum e1;
    M::smwpStruct s1;
    M::smwpStructSeq seq1;
    M::smwpStringStructDict dict1;
    M::smwpClass c1;
    M::smwpInterface i1;
}

class smwpTest6Class extends M::smwpBaseClass implements M::smwpBaseInterface
{
    M::smwpStruct
    smwpTest6Op1(M::smwpEnum i1,
                 M::smwpStruct i2,
                 M::smwpStructSeq i3,
                 M::smwpStringStructDict i4,
                 M::smwpInterface i5,
                 M::smwpClass i6,
                 out M::smwpEnum o1,
                 out M::smwpStruct o2,
                 out M::smwpStructSeq o3,
                 out M::smwpStringStructDict o4,
                 out M::smwpInterface o5,
                 out M::smwpClass o6)
        throws M::smwpException;

    ["amd"]
    M::smwpStruct
    smwpTest6Op3(M::smwpEnum i1,
                 M::smwpStruct i2,
                 M::smwpStructSeq i3,
                 M::smwpStringStructDict i4,
                 M::smwpInterface i5,
                 M::smwpClass i6,
                 out M::smwpEnum o1,
                 out M::smwpStruct o2,
                 out M::smwpStructSeq o3,
                 out M::smwpStringStructDict o4,
                 out M::smwpInterface o5,
                 out M::smwpClass o6)
        throws M::smwpException;
}

}
