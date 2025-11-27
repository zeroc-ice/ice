// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * Base class for marshaled-result classes, which are generated for operations with "marshaled-result" metadata.
 *
 * @hidden
 */
public interface MarshaledResult {
    OutputStream getOutputStream();
}
