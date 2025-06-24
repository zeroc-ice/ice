// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

/**
 * Exception thrown when an update operation fails in the IceGrid GUI application.
 */
public class UpdateFailedException extends java.lang.Exception {
    /**
     * Constructs an UpdateFailedException for a duplicate child ID.
     *
     * @param parent the parent node
     * @param id     the conflicting child ID
     */
    UpdateFailedException(TreeNode parent, String id) {
        _message = parent.getFullId().toString() + " has already a child with '" + id + "' as ID.";
    }

    /**
     * Constructs an UpdateFailedException with a custom message.
     *
     * @param message the detail message for this exception
     */
    UpdateFailedException(String message) {
        _message = message;
    }

    @Override
    public String toString() {
        return _message;
    }

    private String _message;
}
