// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public final class ValueWriter
{
    public static void
    write(java.lang.Object obj, IceUtil.OutputBase out)
    {
        writeValue(obj, true, null, out);
    }

    private static void
    writeValue(java.lang.Object value, boolean outer, java.util.Map objectTable, IceUtil.OutputBase out)
    {
        if(value == null)
        {
            out.print("(null)");
        }
        else
        {
            Class c = value.getClass();
            if(c.equals(Byte.class) || c.equals(Short.class) || c.equals(Integer.class) || c.equals(Long.class) ||
               c.equals(Double.class) || c.equals(Float.class) || c.equals(Boolean.class))
            {
                out.print(value.toString());
            }
            else if(c.equals(String.class))
            {
                out.print("\"");
                out.print(value.toString());
                out.print("\"");
            }
            else if(c.isArray())
            {
                int n = java.lang.reflect.Array.getLength(value);
                out.print("[ ");
                for(int i = 0; i < n; i++)
                {
                    if(i > 0)
                    {
                        out.print(", ");
                    }
                    writeValue(java.lang.reflect.Array.get(value, i), false, objectTable, out);
                }
                out.print(" ]");
            }
            else if(value instanceof java.util.Map)
            {
                if(!outer)
                {
                    out.print("{");
                }
                out.inc();
                java.util.Map map = (java.util.Map)value;
                java.util.Iterator i = map.entrySet().iterator();
                while(i.hasNext())
                {
                    java.util.Map.Entry entry = (java.util.Map.Entry)i.next();
                    out.nl();
                    writeValue(entry.getKey(), false, objectTable, out);
                    out.print(" => ");
                    writeValue(entry.getValue(), false, objectTable, out);
                }
                out.dec();
                if(!outer)
                {
                    out.nl();
                    out.print("}");
                }
            }
            else if(value instanceof Ice.ObjectPrxHelper)
            {
                Ice.ObjectPrxHelper proxy = (Ice.ObjectPrxHelper)value;
                out.print(proxy.__reference().toString());
            }
            else if(value instanceof Ice.Object)
            {
                //
                // Check for recursion.
                //
                if(objectTable != null && objectTable.containsKey(value))
                {
                    out.print("(recursive)");
                }
                else
                {
                    if(objectTable == null)
                    {
                        objectTable = new java.util.IdentityHashMap();
                    }
                    objectTable.put(value, null);
                    if(!outer)
                    {
                        out.print("{");
                    }
                    out.inc();
                    writeFields(value, c, objectTable, out);
                    out.dec();
                    if(!outer)
                    {
                        out.nl();
                        out.print("}");
                    }
                }
            }
            else
            {
                //
                // Check for enum characteristics.
                //
                while(true)
                {
                    try
                    {
                        java.lang.reflect.Field __value = c.getDeclaredField("__value");
                        if(!__value.getType().equals(Integer.TYPE) ||
                           !java.lang.reflect.Modifier.isPrivate(__value.getModifiers()) ||
                           java.lang.reflect.Modifier.isStatic(__value.getModifiers()))
                        {
                            break;
                        }
                        java.lang.reflect.Field __values = c.getDeclaredField("__values");
                        if(!__values.getType().isArray() ||
                           !__values.getType().getComponentType().equals(c) ||
                           !java.lang.reflect.Modifier.isPrivate(__values.getModifiers()) ||
                           !java.lang.reflect.Modifier.isStatic(__values.getModifiers()))
                        {
                            break;
                        }
                        java.lang.reflect.Method valueMethod = c.getDeclaredMethod("value", new Class[0]);
                        if(!valueMethod.getReturnType().equals(Integer.TYPE) ||
                           !java.lang.reflect.Modifier.isPublic(valueMethod.getModifiers()) ||
                           java.lang.reflect.Modifier.isStatic(valueMethod.getModifiers()))
                        {
                            break;
                        }

                        java.lang.Object val = valueMethod.invoke(value, new java.lang.Object[0]);
                        assert(val instanceof Integer);
                        java.lang.reflect.Field[] fields = c.getDeclaredFields();
                        for(int i = 0; i < fields.length; i++)
                        {
                            if(java.lang.reflect.Modifier.isPublic(fields[i].getModifiers()) &&
                               fields[i].getType().equals(Integer.TYPE) &&
                               fields[i].getName().startsWith("_") &&
                               fields[i].get(null).equals(val))
                            {
                                out.print(fields[i].getName().substring(1));
                                return;
                            }
                        }
                    }
                    catch(NoSuchFieldException ex)
                    {
                    }
                    catch(NoSuchMethodException ex)
                    {
                    }
                    catch(IllegalAccessException eX)
                    {
                    }
                    catch(java.lang.reflect.InvocationTargetException ex)
                    {
                    }

                    break;
                }

                //
                // Must be struct.
                //
                if(!outer)
                {
                    out.print("{");
                }
                out.inc();
                writeFields(value, c, objectTable, out);
                out.dec();
                if(!outer)
                {
                    out.nl();
                    out.print("}");
                }
            }
        }
    }

    private static void
    writeFields(java.lang.Object obj, Class c, java.util.Map objectTable, IceUtil.OutputBase out)
    {
        if(!c.equals(java.lang.Object.class))
        {
            //
            // Write the superclass first.
            //
            writeFields(obj, c.getSuperclass(), objectTable, out);

            //
            // Write the declared fields of the given class.
            //
            java.lang.reflect.Field[] fields = c.getDeclaredFields();
            for(int i = 0; i < fields.length; i++)
            {
                //
                // Only write public, non-static fields.
                //
                int mods = fields[i].getModifiers();
                if(java.lang.reflect.Modifier.isPublic(mods) && !java.lang.reflect.Modifier.isStatic(mods))
                {
                    out.nl();
                    out.print(fields[i].getName());
                    out.print(" = ");

                    try
                    {
                        java.lang.Object value = fields[i].get(obj);
                        writeValue(value, false, objectTable, out);
                    }
                    catch(IllegalAccessException ex)
                    {
                        assert(false);
                    }
                }
            }
        }
    }
}
