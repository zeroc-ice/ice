//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

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
