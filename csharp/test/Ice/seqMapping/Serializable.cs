// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

[assembly: CLSCompliant(true)]

namespace Serialize
{

[Serializable]
public class Small // Fewer than 254 bytes with a BinaryFormatter.
{
    public int i;
}

[Serializable]
public class Large // More than 254 bytes with a BinaryFormatter.
{
    public double d1;
    public double d2;
    public double d3;
    public double d4;
    public double d5;
    public double d6;
    public double d7;
    public double d8;
    public double d9;
    public double d10;
}

[Serializable]
public class Struct // Used to test that null members marshal correctly.
{
    public object o;
    public object o2;
    public string s;
    public string s2;
}

}
