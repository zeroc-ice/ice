// Copyright (c) ZeroC, Inc.

package com.zeroc.IceBox;

import com.zeroc.Ice.LocalException;

/**
 * This exception is a general failure notification. It is thrown for errors such as a service
 * encountering an error during initialization, or the service manager being unable to load a
 * service executable.
 */
public final class FailureException extends LocalException {
    public FailureException(String message) {
        super(message);
    }

    public FailureException(String message, Throwable cause) {
        super(message, cause);
    }

    public String ice_id() {
        return "::IceBox::FailureException";
    }

    private static final long serialVersionUID = -7740030157337496059L;
}
