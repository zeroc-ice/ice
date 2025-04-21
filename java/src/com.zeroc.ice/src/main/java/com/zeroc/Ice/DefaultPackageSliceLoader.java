// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Implements SliceLoader using a default package.
 */
public final class DefaultPackageSliceLoader implements SliceLoader {
    private final String _packagePrefix;
    private final ClassLoader _classLoader;

    // We cache successful resolutions. The size of this cache is bounded by the number of Slice classes and exceptions
    // in the program. We can't cache unsuccessful resolutions because it would create an unbounded cache.
    private final Map<String, Class<?>> _typeIdToClass = new ConcurrentHashMap<>();

    /**
     * Creates a DefaultPackageSliceLoader.
     *
     * @param defaultPackage The package that holds all Slice classes and exceptions. Can be empty.
     * @param classLoader The class loader to use to load the classes. Can be null.
     */
    public DefaultPackageSliceLoader(String defaultPackage, ClassLoader classLoader) {
        _packagePrefix = defaultPackage.isEmpty() ? "" : defaultPackage + ".";
        _classLoader = classLoader;
    }

    @Override
    public java.lang.Object newInstance(String typeId) {
        if (!typeId.startsWith("::")) {
            // This could be for example a compact ID. This implementation doesn't handle compact IDs.
            return null;
        }

        // Check cache.
        Class<?> mappedClass = _typeIdToClass.get(typeId);
        if (mappedClass != null) {
            return newInstance(mappedClass, typeId);
        }

        String className = typeId.substring(2).replace("::", ".");
        mappedClass = Util.findClass(_packagePrefix + className, _classLoader);
        if (mappedClass != null) {
            _typeIdToClass.putIfAbsent(typeId, mappedClass);
            return newInstance(mappedClass, typeId);
        }
        return null;
    }

    private static java.lang.Object newInstance(Class<?> mappedClass, String typeId) {
        try {
            return mappedClass.getDeclaredConstructor().newInstance();
        } catch (Exception ex) {
            throw new MarshalException(String.format(
                    "Failed to create an instance of class '%s' for type ID '%s'.",
                    mappedClass.getName(),
                    typeId),
                ex);
        }
    }
}
