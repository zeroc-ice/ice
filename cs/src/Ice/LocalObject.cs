// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace Ice
{

    public interface LocalObject : System.ICloneable
    {
        int ice_hash();
    }

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
