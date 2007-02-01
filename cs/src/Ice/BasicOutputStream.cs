// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    public class BasicOutputStream : BasicStream
    {
        public BasicOutputStream(Instance instance, Ice.OutputStream outStream)
            : base(instance)
        {
            out_ = outStream;
        }

        public Ice.OutputStream out_;
    }
}
