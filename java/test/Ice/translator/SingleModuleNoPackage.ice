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

// smnp = single module no package

module M
{

enum smnpEnum { smnpE1, smnpE2 };

const smnpEnum smnpConstant = smnpE2;

struct smnpStruct
{
    smnpEnum e;
};

sequence<smnpStruct> smnpStructSeq;

dictionary<string, smnpStruct> smnpStringStructDict;

interface smnpBaseInterface
{
    void smnpBaseInterfaceOp();
};

interface smnpInterface extends smnpBaseInterface
{
    void smnpInterfaceOp();
};

class smnpBaseClass
{
    smnpEnum e;
    smnpStruct s;
    smnpStructSeq seq;
    smnpStringStructDict dict;
};

class smnpClass extends smnpBaseClass implements smnpInterface
{
};

exception smnpBaseException
{
    smnpEnum e;
    smnpStruct s;
    smnpStructSeq seq;
    smnpStringStructDict dict;
    smnpClass c;
};

exception smnpException extends smnpBaseException
{
};

};
