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
    smwpClass c;
};

exception smwpException extends smwpBaseException
{
};

};
