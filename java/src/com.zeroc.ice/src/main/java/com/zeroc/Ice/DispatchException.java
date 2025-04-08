// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * The dispatch failed. This is the base class for local exceptions that can be marshaled and
 * transmitted "over the wire".
 */
public class DispatchException extends LocalException {
    public DispatchException(int replyStatus, String message, Throwable cause) {
        super(createMessage(message, replyStatus), cause);
        this.replyStatus = replyStatus;
    }

    public DispatchException(int replyStatus, String message) {
        this(replyStatus, message, null);
    }

    public DispatchException(int replyStatus) {
        this(replyStatus, null, null);
    }

    /** The reply status, as an int. */
    public final int replyStatus;

    @Override
    public String ice_id() {
        return "::Ice::DispatchException";
    }

    private static final long serialVersionUID = 0L;

    private static String createMessage(String message, int replyStatus) {
        if (message == null) {
            return "The dispatch failed with reply status "
                + replyStatusToString(replyStatus)
                + ".";
        } else {
            return message;
        }
    }

    private static String replyStatusToString(int replyStatusInt) {
        var replyStatus = ReplyStatus.valueOf(replyStatusInt);
        if (replyStatus == null) {
            return Integer.toString(replyStatusInt);
        } else {
            return replyStatus.toString();
        }
    }
}
