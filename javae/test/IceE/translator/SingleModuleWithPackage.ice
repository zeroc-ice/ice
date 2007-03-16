// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

// smwp = single module with package

[["java:package:smwp"]]

module M
{

enum smwpEnum { smwpE1, smwpE2 };

const smwpEnum smwpConstant = smwpE2;

struct smwpStruct
{
    smwpEnum e;
};

sequence<smwpStruct> smwpStructSeq;

dictionary<string, smwpStruct> smwpStringStructDict;

interface smwpBaseInterface
{
    void smwpBaseInterfaceOp();
};

interface smwpInterface extends smwpBaseInterface
{
    void smwpInterfaceOp();
};

class smwpBaseClass
{
    smwpEnum e;
    smwpStruct s;
    smwpStructSeq seq;
    smwpStringStructDict dict;
};

class smwpClass extends smwpBaseClass implements smwpInterface
{
};

exception smwpBaseException
{
    smwpEnum e;
    smwpStruct s;
    smwpStructSeq seq;
    smwpStringStructDict dict;
};

exception smwpException extends smwpBaseException
{
};

};
