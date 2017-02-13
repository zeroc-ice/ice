// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

import java.nio.channels.SelectionKey;

public class SocketOperation
{
    public static final int None = 0;
    public static final int Read = SelectionKey.OP_READ;
    public static final int Write = SelectionKey.OP_WRITE;
    public static final int Connect = SelectionKey.OP_CONNECT;
}
