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
