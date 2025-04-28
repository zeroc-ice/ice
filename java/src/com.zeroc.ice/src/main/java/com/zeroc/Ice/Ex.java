// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.PrintWriter;
import java.io.StringWriter;

class Ex {
    public static <T extends Value> void throwUOE(Class<T> expectedType, Value v) {
        if (v instanceof UnknownSlicedValue) {
            var usv = (UnknownSlicedValue) v;
            throw new MarshalException(
                "The Slice loader did not find a class for type ID '"
                    + usv.ice_id()
                    + "'.");
        }

        String type = v.ice_id();
        String expected;
        try {
            expected = (String) expectedType.getMethod("ice_staticId").invoke(null);
        } catch (Exception ex) {
            expected = "";
            assert false;
        }
        throw new MarshalException(
            "Failed to unmarshal class with type ID '"
                + expected
                + "': the Slice loader returned class with type ID '"
                + type
                + "'.");
    }

    public static void throwUOE(String expectedType, Value v) {
        if (v instanceof UnknownSlicedValue) {
            var usv = (UnknownSlicedValue) v;
            throw new MarshalException(
                "The Slice loader did not find a class for type ID '"
                    + usv.ice_id()
                    + "'.");
        }

        String type = v.ice_id();
        throw new MarshalException(
            "Failed to unmarshal class with type ID '"
                + expectedType
                + "': the Slice loader returned class with type ID '"
                + type
                + "'.");
    }

    public static void throwMemoryLimitException(int requested, int maximum) {
        throw new MarshalException(
            "Cannot unmarshal Ice message: the message size of "
                + requested
                + " bytes exceeds the maximum allowed of "
                + maximum
                + " bytes (see Ice.MessageSizeMax).");
    }

    //
    // A small utility to get the stack trace of the exception (which also includes toString()).
    //
    public static String toString(Throwable ex) {
        StringWriter sw = new StringWriter();
        PrintWriter pw = new PrintWriter(sw);
        ex.printStackTrace(pw);
        pw.flush();
        return sw.toString();
    }

    private Ex() {}
}
