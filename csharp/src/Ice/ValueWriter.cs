//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System.Collections;
    using System.Collections.Generic;
    using System.Diagnostics;
    using System.Reflection;
    using IceUtilInternal;

    public sealed class ValueWriter
    {
        public static void write(object obj, OutputBase output)
        {
            writeValue(null, obj, null, output);
        }

        private static void writeValue(string name, object val, Dictionary<Ice.Object, object> objectTable, OutputBase output)
        {
            if (val == null)
            {
                writeName(name, output);
                output.print("(null)");
            }
            else
            {
                System.Type c = val.GetType();
                if (c.Equals(typeof(byte)) || c.Equals(typeof(short)) || c.Equals(typeof(int)) ||
                   c.Equals(typeof(long)) || c.Equals(typeof(double)) || c.Equals(typeof(float)) ||
                   c.Equals(typeof(bool)))
                {
                    writeName(name, output);
                    output.print(val.ToString());
                }
                else if (c.Equals(typeof(string)))
                {
                    writeName(name, output);
                    output.print("\"");
                    output.print(val.ToString());
                    output.print("\"");
                }
                else if (val is IList)
                {
                    int n = 0;
                    IEnumerator i = ((IList)val).GetEnumerator();
                    while (i.MoveNext())
                    {
                        string elem = (name != null ? name : "");
                        elem += "[" + n++ + "]";
                        writeValue(elem, i.Current, objectTable, output);
                    }
                }
                else if (val is IDictionary)
                {
                    foreach (DictionaryEntry entry in (IDictionary)val)
                    {
                        string elem = name != null ? name + "." : "";
                        writeValue(elem + "key", entry.Key, objectTable, output);
                        writeValue(elem + "value", entry.Value, objectTable, output);
                    }
                }
                else if (val is Ice.ObjectPrxHelperBase)
                {
                    writeName(name, output);
                    Ice.ObjectPrxHelperBase proxy = (Ice.ObjectPrxHelperBase)val;
                    output.print(proxy.iceReference().ToString());
                }
                else if (val is Ice.Object)
                {
                    //
                    // Check for recursion.
                    //
                    if (objectTable != null && objectTable.ContainsKey((Ice.Object)val))
                    {
                        writeName(name, output);
                        output.print("(recursive)");
                    }
                    else
                    {
                        if (objectTable == null)
                        {
                            objectTable = new Dictionary<Ice.Object, object>();
                        }
                        objectTable[(Ice.Object)val] = null;
                        writeFields(name, val, c, objectTable, output);
                    }
                }
                else if (c.IsEnum)
                {
                    writeName(name, output);
                    output.print(val.ToString());
                }
                else
                {
                    //
                    // Must be struct.
                    //
                    writeFields(name, val, c, objectTable, output);
                }
            }
        }

        private static void writeFields(string name, object obj, System.Type c, Dictionary<Ice.Object, object> objectTable,
                                        OutputBase output)
        {
            if (!c.Equals(typeof(object)))
            {
                //
                // Write the superclass first.
                //
                writeFields(name, obj, c.BaseType, objectTable, output);

                //
                // Write the declared fields of the given class.
                //
                FieldInfo[] fields =
                    c.GetFields(BindingFlags.Instance | BindingFlags.DeclaredOnly | BindingFlags.Public);

                for (int i = 0; i < fields.Length; i++)
                {
                    string fieldName = (name != null ? name + '.' + fields[i].Name : fields[i].Name);

                    try
                    {
                        object val = fields[i].GetValue(obj);
                        writeValue(fieldName, val, objectTable, output);
                    }
                    catch (System.UnauthorizedAccessException)
                    {
                        Debug.Assert(false);
                    }
                }
            }
        }

        private static void writeName(string name, OutputBase output)
        {
            if (name != null)
            {
                output.nl();
                output.print(name + " = ");
            }
        }
    }

}
