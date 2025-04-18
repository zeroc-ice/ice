// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Implements SliceLoader using a map Slice module to Java package.
 */
public final class ModuleToPackageSliceLoader implements SliceLoader {
    private final Map<String, String> _moduleToPackageMap;
    private final ClassLoader _classLoader;

    // We cache successful resolutions. The size of this cache is bounded by the number of Slice classes and exceptions
    // in the program. We can't cache unsuccessful resolutions because it would create an unbounded cache.
    private final Map<String, Class<?>> _typeIdToClass = new ConcurrentHashMap<>();

    /**
     * Creates a ModuleToPackageSliceLoader.
     *
     * @param moduleToPackageMap A map of Slice module names to Java package names.
     * @param classLoader The class loader to use to load the classes. Can be null.
     */
    public ModuleToPackageSliceLoader(Map<String, String> moduleToPackageMap, ClassLoader classLoader) {
        _moduleToPackageMap = new HashMap<>(moduleToPackageMap);
        _classLoader = classLoader;
    }

    /**
     * Creates a ModuleToPackageSliceLoader using a single module to package mapping.
     *
     * @param module The type ID of the Slice module, for example "::VisitorCenter".
     * @param packageName The Java package name, for example "com.example.visitorcenter".
     */
    public ModuleToPackageSliceLoader(String module, String packageName) {
        this(Map.of(module, packageName), null);
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

        int pos = typeId.lastIndexOf("::");

        while (pos > 0) {
            String className = typeId.substring(pos + 2).replace("::", ".");
            String module = typeId.substring(0, pos);
            String packageName = _moduleToPackageMap.get(module);

            if (packageName != null) {
                mappedClass = Util.findClass(packageName + "." + className, _classLoader);
                if (mappedClass != null) {
                    _typeIdToClass.putIfAbsent(typeId, mappedClass);
                    return newInstance(mappedClass, typeId);
                }
            }

            pos = module.lastIndexOf("::");
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
