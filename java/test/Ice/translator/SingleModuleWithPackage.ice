// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

// smwp = single module with package

[["java:package:smwp"]]

module M
{

enum smwpEnum { smwpE1, smwpE2 }

const smwpEnum smwpConstant = smwpE2;

struct smwpStruct
{
    smwpEnum e;
}

sequence<smwpStruct> smwpStructSeq;

dictionary<string, smwpStruct> smwpStringStructDict;

interface smwpBaseInterface
{
    void smwpBaseInterfaceOp();
}

interface smwpInterface extends smwpBaseInterface
{
    void smwpInterfaceOp();
}

class smwpBaseClass
{
    smwpEnum e;
    smwpStruct s;
    smwpStructSeq seq;
    smwpStringStructDict dict;
}

class smwpClass extends smwpBaseClass implements smwpInterface
{
}

exception smwpBaseException
{
    smwpEnum e;
    smwpStruct s;
    smwpStructSeq seq;
    smwpStringStructDict dict;
    smwpClass c;
}

exception smwpException extends smwpBaseException
{
}

}
