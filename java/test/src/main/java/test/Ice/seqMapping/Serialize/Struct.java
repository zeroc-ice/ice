// Copyright (c) ZeroC, Inc.

package test.Ice.seqMapping.Serialize;

import java.io.Serializable;

public class Struct
        implements Serializable // Used to test that null members marshal correctly.
{
    public Object o;
    public Object o2;
    public String s;
    public String s2;
}
