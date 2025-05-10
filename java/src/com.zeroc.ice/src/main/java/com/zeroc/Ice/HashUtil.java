// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.Arrays;

/**
 * @hidden Public because it's used by IceBT, IceGridGUI, and the generated code.
 */
public final class HashUtil {
    public static int hashAdd(int hashCode, boolean value) {
        return shift(hashCode) ^ (value ? 0 : 1);
    }

    public static int hashAdd(int hashCode, byte value) {
        return shift(hashCode) ^ (int) (2654435761L * value);
    }

    public static int hashAdd(int hashCode, short value) {
        return shift(hashCode) ^ (int) (2654435761L * value);
    }

    public static int hashAdd(int hashCode, int value) {
        return shift(hashCode) ^ (int) (2654435761L * value);
    }

    public static int hashAdd(int hashCode, long value) {
        return shift(hashCode) ^ (int) (value ^ (value >>> 32));
    }

    public static int hashAdd(int hashCode, float value) {
        return shift(hashCode) ^ Float.floatToIntBits(value);
    }

    public static int hashAdd(int hashCode, double value) {
        long v = Double.doubleToLongBits(value);
        return shift(hashCode) ^ (int) (v ^ (v >>> 32));
    }

    public static int hashAdd(int hashCode, java.lang.Object value) {
        if (value != null) {
            hashCode = shift(hashCode) ^ value.hashCode();
        }
        return hashCode;
    }

    public static int hashAdd(int hashCode, boolean[] arr) {
        return shift(hashCode) ^ Arrays.hashCode(arr);
    }

    public static int hashAdd(int hashCode, byte[] arr) {
        return shift(hashCode) ^ Arrays.hashCode(arr);
    }

    public static int hashAdd(int hashCode, char[] arr) {
        return shift(hashCode) ^ Arrays.hashCode(arr);
    }

    public static int hashAdd(int hashCode, short[] arr) {
        return shift(hashCode) ^ Arrays.hashCode(arr);
    }

    public static int hashAdd(int hashCode, int[] arr) {
        return shift(hashCode) ^ Arrays.hashCode(arr);
    }

    public static int hashAdd(int hashCode, long[] arr) {
        return shift(hashCode) ^ Arrays.hashCode(arr);
    }

    public static int hashAdd(int hashCode, float[] arr) {
        return shift(hashCode) ^ Arrays.hashCode(arr);
    }

    public static int hashAdd(int hashCode, double[] arr) {
        return shift(hashCode) ^ Arrays.hashCode(arr);
    }

    public static int hashAdd(int hashCode, java.lang.Object[] arr) {
        return shift(hashCode) ^ Arrays.hashCode(arr);
    }

    private static int shift(int hashCode) {
        return (hashCode << 5) + hashCode;
    }

    private HashUtil() {}
}
