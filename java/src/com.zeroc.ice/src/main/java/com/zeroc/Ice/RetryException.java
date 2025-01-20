// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

class RetryException extends java.lang.Exception {
    public RetryException(LocalException ex) {
        _ex = ex;
    }

    public LocalException get() {
        return _ex;
    }

    private final LocalException _ex;

    private static final long serialVersionUID = -8555917196921366848L;
}
