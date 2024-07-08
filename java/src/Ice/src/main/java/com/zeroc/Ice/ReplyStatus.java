// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/** Represents the status of a reply. */
public enum ReplyStatus {
    Ok(0),
    UserException(1),
    ObjectNotExist(2),
    FacetNotExist(3),
    OperationNotExist(4),
    UnknownLocalException(5),
    UnknownUserException(6),
    UnknownException(7);

    private final byte value;

    public byte value() {
        return value;
    }

    public static ReplyStatus valueOf(byte v) {
        return switch (v) {
            case 0 -> Ok;
            case 1 -> UserException;
            case 2 -> ObjectNotExist;
            case 3 -> FacetNotExist;
            case 4 -> OperationNotExist;
            case 5 -> UnknownLocalException;
            case 6 -> UnknownUserException;
            case 7 -> UnknownException;
            default -> throw new MarshalException("Invalid value for ReplyStatus: " + v);
        };
    }

    private ReplyStatus(int value) {
        this.value = (byte) value;
    }
}
