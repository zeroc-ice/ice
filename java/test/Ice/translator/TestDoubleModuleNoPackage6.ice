//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Test: use DoubleModuleNoPackage types from (same) double module definitions

#include <DoubleModuleNoPackage.ice>

module M1::M2
{

const dmnpEnum dmnpTest6Constant = dmnpE1;

struct dmnpTest6Struct
{
    dmnpEnum e;
    dmnpStruct s;
    dmnpStructSeq seq;
    dmnpStringStructDict dict;
    dmnpClass c;
    dmnpInterface i;
}

sequence<dmnpStruct> dmnpTest6StructSeq;

dictionary<dmnpStruct, dmnpBaseClass> dmnpTest6StructClassSeq;

interface dmnpTest6Interface : dmnpInterface {}

exception dmnpTest6Exception : dmnpException
{
    dmnpEnum e1;
    dmnpStruct s1;
    dmnpStructSeq seq1;
    dmnpStringStructDict dict1;
    dmnpClass c1;
    dmnpInterface i1;
}

class dmnpTest6Class : dmnpBaseClass implements dmnpBaseInterface
{
    dmnpStruct
    dmnpTest6Op1(dmnpEnum i1,
                 dmnpStruct i2,
                 dmnpStructSeq i3,
                 dmnpStringStructDict i4,
                 dmnpInterface i5,
                 dmnpClass i6,
                 out dmnpEnum o1,
                 out dmnpStruct o2,
                 out dmnpStructSeq o3,
                 out dmnpStringStructDict o4,
                 out dmnpInterface o5,
                 out dmnpClass o6)
        throws dmnpException;

    [amd]
    dmnpStruct
    dmnpTest6Op3(dmnpEnum i1,
                 dmnpStruct i2,
                 dmnpStructSeq i3,
                 dmnpStringStructDict i4,
                 dmnpInterface i5,
                 dmnpClass i6,
                 out dmnpEnum o1,
                 out dmnpStruct o2,
                 out dmnpStructSeq o3,
                 out dmnpStringStructDict o4,
                 out dmnpInterface o5,
                 out dmnpClass o6)
        throws dmnpException;
}

}
