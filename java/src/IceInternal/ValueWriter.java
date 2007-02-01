// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class ValueWriter
{
    public static void
    write(java.lang.Object obj, IceUtil.OutputBase out)
    {
        writeValue(null, obj, null, out);
    }

    private static void
    writeValue(String name, java.lang.Object value, java.util.Map objectTable, IceUtil.OutputBase out)
    {
        if(value == null)
        {
            writeName(name, out);
            out.print("(null)");
        }
        else
        {
            Class c = value.getClass();
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
                java.util.Map map = (java.util.Map)value;
                java.util.Iterator i = map.entrySet().iterator();
                while(i.hasNext())
                {
                    java.util.Map.Entry entry = (java.util.Map.Entry)i.next();
                    String elem = (name != null ? name + "." : "");
                    writeValue(elem + "key", entry.getKey(), objectTable, out);
                    writeValue(elem + "value", entry.getValue(), objectTable, out);
                }
            }
            else if(value instanceof Ice.ObjectPrxHelperBase)
            {
                writeName(name, out);
                Ice.ObjectPrxHelperBase proxy = (Ice.ObjectPrxHelperBase)value;
                out.print(proxy.__reference().toString());
            }
            else if(value instanceof Ice.Object)
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
                        objectTable = new java.util.IdentityHashMap();
                    }
                    objectTable.put(value, null);
                    writeFields(name, value, c, objectTable, out);
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
                                writeName(name, out);
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
                writeFields(name, value, c, objectTable, out);
            }
        }
    }

    private static void
    writeFields(String name, java.lang.Object obj, Class c, java.util.Map objectTable, IceUtil.OutputBase out)
    {
        if(!c.equals(java.lang.Object.class))
        {
            //
            // Write the superclass first.
            //
            writeFields(name, obj, c.getSuperclass(), objectTable, out);

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
                    String fieldName = (name != null ? name + '.' + fields[i].getName() : fields[i].getName());

                    try
                    {
                        java.lang.Object value = fields[i].get(obj);
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
    writeName(String name, IceUtil.OutputBase out)
    {
        if(name != null)
        {
            out.nl();
            out.print(name + " = ");
        }
    }
}
