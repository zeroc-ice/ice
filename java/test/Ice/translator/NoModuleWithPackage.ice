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

// nmwp = no module with package

[["java:package:nmwp"]]

enum nmwpEnum { nmwpE1, nmwpE2 };

const nmwpEnum nmwpConstant = nmwpE2;

struct nmwpStruct
{
    nmwpEnum e;
};

sequence<nmwpStruct> nmwpStructSeq;

dictionary<string, nmwpStruct> nmwpStringStructDict;

interface nmwpBaseInterface
{
    void nmwpBaseInterfaceOp();
};

interface nmwpInterface extends nmwpBaseInterface
{
    void nmwpInterfaceOp();
};

class nmwpBaseClass
{
    nmwpEnum e;
    nmwpStruct s;
    nmwpStructSeq seq;
    nmwpStringStructDict dict;
};

class nmwpClass extends nmwpBaseClass implements nmwpInterface
{
};

exception nmwpBaseException
{
    nmwpEnum e;
    nmwpStruct s;
    nmwpStructSeq seq;
    nmwpStringStructDict dict;
    nmwpClass c;
};

exception nmwpException extends nmwpBaseException
{
};
