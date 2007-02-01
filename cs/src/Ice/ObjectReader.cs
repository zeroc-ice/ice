// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

namespace Ice
{
    public abstract class ObjectReader : ObjectImpl
    {
        public abstract void read(InputStream inStream, bool rid);

        public override void write__(IceInternal.BasicStream os)
        {
            Debug.Assert(false);
        }

        public override void read__(IceInternal.BasicStream istr, bool rid)
        {
            IceInternal.BasicInputStream bis = (IceInternal.BasicInputStream)istr;
            read(bis.in_, rid);
        }
    }
}
