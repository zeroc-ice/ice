// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

class Ex {
    public static <T extends Value> void throwUOE(Class<T> expectedType, Value v) {
        // If the object is an unknown sliced object, we didn't find a value factory.
        if (v instanceof UnknownSlicedValue) {
            var usv = (UnknownSlicedValue) v;
            throw new MarshalException(
                    "Cannot find value factory to unmarshal class with type ID '"
                            + usv.ice_id()
                            + "'.");
        }

        String type = v.ice_id();
        String expected;
        try {
            expected = (String) expectedType.getMethod("ice_staticId").invoke(null);
        } catch (Exception ex) {
            expected = "";
            assert (false);
        }
        throw new MarshalException(
                "Failed to unmarshal class with type ID '"
                        + expected
                        + "': value factory returned class with type ID '"
                        + type
                        + "'.");
    }

    public static void throwUOE(String expectedType, Value v) {
        // If the object is an unknown sliced object, we didn't find a value factory.
        if (v instanceof UnknownSlicedValue) {
            var usv = (UnknownSlicedValue) v;
            throw new MarshalException(
                    "Cannot find value factory to unmarshal class with type ID '"
                            + usv.ice_id()
                            + "'.");
        }

        String type = v.ice_id();
        throw new MarshalException(
                "Failed to unmarshal class with type ID '"
                        + expectedType
                        + "': value factory returned class with type ID '"
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
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        ex.printStackTrace(pw);
        pw.flush();
        return sw.toString();
    }
}
