// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// dmwp = double module with package

[["java:package:dmwp"]]

module M1
{
module M2
{

enum dmwpEnum { dmwpE1, dmwpE2 }

const dmwpEnum dmwpConstant = dmwpE2;

struct dmwpStruct
{
    dmwpEnum e;
}

sequence<dmwpStruct> dmwpStructSeq;

dictionary<string, dmwpStruct> dmwpStringStructDict;

interface dmwpBaseInterface
{
    void dmwpBaseInterfaceOp();
}

interface dmwpInterface extends dmwpBaseInterface
{
    void dmwpInterfaceOp();
}

class dmwpBaseClass
{
    dmwpEnum e;
    dmwpStruct s;
    dmwpStructSeq seq;
    dmwpStringStructDict dict;
}

class dmwpClass extends dmwpBaseClass implements dmwpInterface
{
}

exception dmwpBaseException
{
    dmwpEnum e;
    dmwpStruct s;
    dmwpStructSeq seq;
    dmwpStringStructDict dict;
    dmwpClass c;
}

exception dmwpException extends dmwpBaseException
{
}

}
}
