// Copyright (c) ZeroC, Inc.

package test.Ice.seqMapping.Serialize;

public class Struct
        implements java.io.Serializable // Used to test that null members marshal correctly.
{
    public Object o;
    public Object o2;
    public String s;
    public String s2;
}
