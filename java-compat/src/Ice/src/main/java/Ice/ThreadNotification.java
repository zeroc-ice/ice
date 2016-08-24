// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Interface for thread notification hooks. Applications can derive
 * a class that implements the <code>start</code> and <code>stop</code>
 * methods to intercept creation and destruction of threads created
 * by the Ice run time.
 *
 * @see InitializationData
 **/
public interface ThreadNotification
{
    /**
     * The Ice run time calls <code>start</code> for each new
     * thread it creates. The call is made by the newly-started thread.
     **/
    void start();

    /**
     * The Ice run time calls <code>stop</code> before it destroys
     * a thread. The call is made by thread that is about to be
     * destroyed.
     **/
    void stop();
}
