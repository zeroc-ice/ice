// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Options for configuring the behavior of a connection. All timeouts are specified in seconds, and a timeout
 * value of 0 or less indicates an infinite timeout.
 *
 * @param connectTimeout the timeout for establishing a connection
 * @param closeTimeout the timeout for closing a connection
 * @param idleTimeout the timeout for an idle connection
 * @param enableIdleCheck whether to enable idle connection checks
 * @param inactivityTimeout the timeout for inactivity on a connection
 * @param maxDispatches the maximum number of dispatches allowed on a connection before it is closed
 */
public record ConnectionOptions(
        int connectTimeout,
        int closeTimeout,
        int idleTimeout,
        boolean enableIdleCheck,
        int inactivityTimeout,
        int maxDispatches) {}
