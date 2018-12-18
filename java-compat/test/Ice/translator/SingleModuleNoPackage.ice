// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

// smnp = single module no package

module M
{

enum smnpEnum { smnpE1, smnpE2 }

const smnpEnum smnpConstant = smnpE2;

struct smnpStruct
{
    smnpEnum e;
}

sequence<smnpStruct> smnpStructSeq;

dictionary<string, smnpStruct> smnpStringStructDict;

interface smnpBaseInterface
{
    void smnpBaseInterfaceOp();
}

interface smnpInterface extends smnpBaseInterface
{
    void smnpInterfaceOp();
}

class smnpBaseClass
{
    smnpEnum e;
    smnpStruct s;
    smnpStructSeq seq;
    smnpStringStructDict dict;
}

class smnpClass extends smnpBaseClass implements smnpInterface
{
}

exception smnpBaseException
{
    smnpEnum e;
    smnpStruct s;
    smnpStructSeq seq;
    smnpStringStructDict dict;
    smnpClass c;
}

exception smnpException extends smnpBaseException
{
}

}
