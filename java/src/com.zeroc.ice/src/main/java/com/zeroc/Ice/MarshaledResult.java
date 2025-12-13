// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Base interface for marshaled result classes, which are generated for operations marked with
 * "marshaled-result" metadata.
 *
 * @hidden
 */
public interface MarshaledResult {
    OutputStream getOutputStream();
}
