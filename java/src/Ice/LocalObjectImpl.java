// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

 /**
  * @deprecated This class is deprecated; you should use a plain java.lang.Object
  **/
public abstract class LocalObjectImpl implements LocalObject, java.lang.Cloneable
{
    public java.lang.Object
    clone() throws java.lang.CloneNotSupportedException
    {
        return super.clone();
    }

    public int
    ice_hash()
    {
        return hashCode();
    }
}
