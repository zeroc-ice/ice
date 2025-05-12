// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.HashMap;
import java.util.Map;

/**
 * Implements SliceLoader using an array of annotated classes.
 */
public final class ClassSliceLoader implements SliceLoader {
    private final Map<String, Class<?>> _typeIdToClass = new HashMap<>();

    /**
     * Creates a ClassSliceLoader.
     *
     * @param classes An array of classes with the {@link SliceTypeId} annotation. Each class may also have the
     *                {@link CompactSliceTypeId} annotation.
     * @throws IllegalArgumentException If any class is not annotated with {@link SliceTypeId}.
     */
    public ClassSliceLoader(Class<?>... classes) {
        for (Class<?> c : classes) {
            SliceTypeId typeId = c.getAnnotation(SliceTypeId.class);
            if (typeId != null) {
                _typeIdToClass.put(typeId.value(), c);

                // CompactSliceTypeId is always in addition to SliceTypeId.
                CompactSliceTypeId compactTypeId = c.getAnnotation(CompactSliceTypeId.class);
                if (compactTypeId != null) {
                    _typeIdToClass.put(String.valueOf(compactTypeId.value()), c);
                }
            } else {
                throw new IllegalArgumentException(
                    String.format(
                        "Class '%s' is not annotated with @SliceTypeId.",
                        c.getName()));
            }
        }
    }

    @Override
    public java.lang.Object newInstance(String typeId) {
        Class<?> c = _typeIdToClass.get(typeId);
        if (c != null) {
            try {
                return c.getDeclaredConstructor().newInstance();
            } catch (Exception ex) {
                throw new MarshalException(
                    String.format(
                        "Failed to create an instance of class '%s' for type ID '%s'.",
                        c.getName(), typeId),
                    ex);
            }
        }
        return null;
    }
}
