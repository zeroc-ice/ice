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
