// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;

namespace Ice {

    public abstract class ObjectWriter : ObjectImpl
    {
        public abstract void write(OutputStream outStream);

        public override void write__(IceInternal.BasicStream os)
        {
            IceInternal.BasicOutputStream bos = (IceInternal.BasicOutputStream)os;
            write(bos.out_);
        }

        public override void read__(IceInternal.BasicStream istr, bool rid)
        {
            Debug.Assert(false);
        }
    }

}
