// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.seqMapping.Serialize;

public class Struct implements java.io.Serializable // Used to test that null members marshal correctly.
{
    public Object o;
    public Object o2;
    public String s;
    public String s2;
}
