// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

namespace Ice
{

    [Obsolete("This interface is deprecated, use System.Object (or nothing) instead.")]
    public interface LocalObject : System.ICloneable
    {
        int ice_hash();
    }

    [Obsolete("This class is deprecated, use System.Object (or nothing) instead.")]
    public abstract class LocalObjectImpl : LocalObject
    {
        public virtual int
            ice_hash()
        {
            return GetHashCode();
        }

        public object Clone()
        {
            return MemberwiseClone();
        }
    }
}
