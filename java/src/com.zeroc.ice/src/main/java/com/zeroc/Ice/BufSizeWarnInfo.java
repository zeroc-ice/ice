// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Structure to track warnings for attempts to set socket buffer sizes. */
class BufSizeWarnInfo {
    /** Whether send size warning has been emitted. */
    public boolean sndWarn;

    /** The send size for which the warning was emitted. */
    public int sndSize;

    /** Whether receive size warning has been emitted. */
    public boolean rcvWarn;

    /** The receive size for which the warning was emitted. */
    public int rcvSize;
}
