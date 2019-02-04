//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public final class ValueWriter
{
    public static void
    write(java.lang.Object obj, com.zeroc.IceUtilInternal.OutputBase out)
    {
        writeValue(null, obj, null, out);
    }

    private static void
    writeValue(String name, java.lang.Object value, java.util.Map<java.lang.Object, java.lang.Object> objectTable,
               com.zeroc.IceUtilInternal.OutputBase out)
    {
        if(value == null)
        {
            writeName(name, out);
            out.print("(null)");
        }
        else
        {
            Class<?> c = value.getClass();
            if(c.equals(Byte.class) || c.equals(Short.class) || c.equals(Integer.class) || c.equals(Long.class) ||
               c.equals(Double.class) || c.equals(Float.class) || c.equals(Boolean.class))
            {
                writeName(name, out);
                out.print(value.toString());
            }
            else if(c.equals(String.class))
            {
                //
                // Indent the lines of a string value.
                //
                writeName(name, out);
                out.print("\"");
                out.useCurrentPosAsIndent();
                String str = value.toString();
                int start = 0, pos;
                while(start < str.length() && (pos = str.indexOf('\n', start)) != -1)
                {
                    out.print(str.substring(start, pos));
                    out.nl();
                    start = pos + 1;
                }
                if(start < str.length())
                {
                    out.print(str.substring(start));
                }
                out.print("\"");
                out.restoreIndent();
            }
            else if(c.isArray())
            {
                int n = java.lang.reflect.Array.getLength(value);
                for(int i = 0; i < n; i++)
                {
                    String elem = (name != null ? name : "");
                    elem += "[" + i + "]";
                    writeValue(elem, java.lang.reflect.Array.get(value, i), objectTable, out);
                }
            }
            else if(value instanceof java.util.Map)
            {
                java.util.Map<?,?> map = (java.util.Map<?,?>)value;
                java.util.Iterator<?> i = map.entrySet().iterator();
                while(i.hasNext())
                {
                    java.util.Map.Entry<?,?> entry = (java.util.Map.Entry<?,?>)i.next();
                    String elem = (name != null ? name + "." : "");
                    writeValue(elem + "key", entry.getKey(), objectTable, out);
                    writeValue(elem + "value", entry.getValue(), objectTable, out);
                }
            }
            else if(value instanceof com.zeroc.Ice._ObjectPrxI)
            {
                writeName(name, out);
                com.zeroc.Ice._ObjectPrxI proxy = (com.zeroc.Ice._ObjectPrxI)value;
                out.print(proxy._getReference().toString());
            }
            else if(value instanceof com.zeroc.Ice.Value)
            {
                //
                // Check for recursion.
                //
                if(objectTable != null && objectTable.containsKey(value))
                {
                    writeName(name, out);
                    out.print("(recursive)");
                }
                else
                {
                    if(objectTable == null)
                    {
                        objectTable = new java.util.IdentityHashMap<java.lang.Object, java.lang.Object>();
                    }
                    objectTable.put(value, null);
                    writeFields(name, value, c, objectTable, out);
                }
            }
            else if(value instanceof java.lang.Enum)
            {
                writeName(name, out);
                out.print(((java.lang.Enum<?>)value).name());
            }
            else
            {
                //
                // Must be struct.
                //
                writeFields(name, value, c, objectTable, out);
            }
        }
    }

    private static void
    writeFields(String name, java.lang.Object obj, Class<?> c,
                java.util.Map<java.lang.Object, java.lang.Object> objectTable, com.zeroc.IceUtilInternal.OutputBase out)
    {
        if(!c.equals(java.lang.Object.class))
        {
            //
            // Write the superclass first.
            //
            writeFields(name, obj, c.getSuperclass(), objectTable, out);

            //
            // Write the declared fields of the given class. We prefer to use the declared
            // fields because it includes protected fields that may have been defined using
            // the Slice "protected" metadata. However, if a security manager prevents us
            // from obtaining the declared fields, we will fall back to using the public ones.
            //
            java.lang.reflect.Field[] fields = null;
            try
            {
                fields = c.getDeclaredFields();
            }
            catch(java.lang.SecurityException ex)
            {
                try
                {
                    fields = c.getFields();
                }
                catch(java.lang.SecurityException e)
                {
                    return; // Nothing else we can do.
                }
            }
            assert(fields != null);
            for(java.lang.reflect.Field field : fields)
            {
                //
                // Only write public, non-static fields.
                //
                int mods = field.getModifiers();
                if(java.lang.reflect.Modifier.isPublic(mods) && !java.lang.reflect.Modifier.isStatic(mods))
                {
                    String fieldName = (name != null ? name + '.' + field.getName() : field.getName());

                    try
                    {
                        java.lang.Object value = field.get(obj);
                        writeValue(fieldName, value, objectTable, out);
                    }
                    catch(IllegalAccessException ex)
                    {
                        assert(false);
                    }
                }
            }
        }
    }

    private static void
    writeName(String name, com.zeroc.IceUtilInternal.OutputBase out)
    {
        if(name != null)
        {
            out.nl();
            out.print(name + " = ");
        }
    }
}
