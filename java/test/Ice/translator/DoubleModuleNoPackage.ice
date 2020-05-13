//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// dmnp = double module no package

module M1::M2
{

enum dmnpEnum { dmnpE1, dmnpE2 }

const dmnpEnum dmnpConstant = dmnpE2;

struct dmnpStruct
{
    dmnpEnum e;
}

sequence<dmnpStruct> dmnpStructSeq;

dictionary<string, dmnpStruct> dmnpStringStructDict;

interface dmnpBaseInterface
{
    void dmnpBaseInterfaceOp();
}

interface dmnpInterface : dmnpBaseInterface
{
    void dmnpInterfaceOp();
}

class dmnpBaseClass
{
    dmnpEnum e;
    dmnpStruct s;
    dmnpStructSeq seq;
    dmnpStringStructDict dict;
}

class dmnpClass : dmnpBaseClass implements dmnpInterface
{
}

exception dmnpBaseException
{
    dmnpEnum e;
    dmnpStruct s;
    dmnpStructSeq seq;
    dmnpStringStructDict dict;
    dmnpClass c;
}

exception dmnpException : dmnpBaseException
{
}

}
