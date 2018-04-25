// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Test.ice>

module Test
{

class D3 extends B
{
    string sd3;
    B pd3;
}

["preserve-slice"]
class PCUnknown extends PBase
{
    string pu;
}

class PCDerived extends PDerived
{
    PBaseSeq pbs;
}

class PCDerived2 extends PCDerived
{
    int pcd2;
}

class PCDerived3 extends PCDerived2
{
    Object pcd3;
}

class CompactPCDerived(57) extends CompactPDerived
{
    PBaseSeq pbs;
}

}
