// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


// nmnp = no module no package

enum nmnpEnum { nmnpE1, nmnpE2 };

const nmnpEnum nmnpConstant = nmnpE2;

struct nmnpStruct
{
    nmnpEnum e;
};

sequence<nmnpStruct> nmnpStructSeq;

dictionary<string, nmnpStruct> nmnpStringStructDict;

interface nmnpBaseInterface
{
    void nmnpBaseInterfaceOp();
};

interface nmnpInterface extends nmnpBaseInterface
{
    void nmnpInterfaceOp();
};

class nmnpBaseClass
{
    nmnpEnum e;
    nmnpStruct s;
    nmnpStructSeq seq;
    nmnpStringStructDict dict;
};

class nmnpClass extends nmnpBaseClass implements nmnpInterface
{
};

exception nmnpBaseException
{
    nmnpEnum e;
    nmnpStruct s;
    nmnpStructSeq seq;
    nmnpStringStructDict dict;
    nmnpClass c;
};

exception nmnpException extends nmnpBaseException
{
};
