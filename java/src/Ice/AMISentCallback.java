// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Callback interface for AMI invocations. This is useful
 * for applications that send asynchronous invocations that
 * might not be sent for some time and, therefore, are
 * queued in the Ice run time for transmission.
 * <p>
 * The Ice run time calls <code>ice_sent</code> if an
 * an AMI invocation cannot be written to transport and was
 * queued for later transmission. Applications that need to
 * implement flow control for AMI invocations can implement
 * this interface on the corresponding AMI callback object.
 **/
public interface AMISentCallback
{
    /**
     * Indicates to the caller of an AMI operation that
     * the invocation was queued for later transmission.
     **/
    void ice_sent();
};
