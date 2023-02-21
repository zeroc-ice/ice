//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System;
    using System.Runtime.InteropServices;
    using System.Collections;
    using System.Collections.Generic;
    using System.Reflection;

    public sealed class AssemblyUtil
    {
#if NETSTANDARD2_0
        public static readonly bool isWindows = RuntimeInformation.IsOSPlatform(OSPlatform.Windows);
        public static readonly bool isMacOS = RuntimeInformation.IsOSPlatform(OSPlatform.OSX);
        public static readonly bool isLinux = RuntimeInformation.IsOSPlatform(OSPlatform.Linux);
        public static readonly bool isMono = RuntimeInformation.FrameworkDescription.Contains("Mono");
#else
        public static readonly bool isWindows = true;
        public static readonly bool isMacOS = false;
        public static readonly bool isLinux = false;
        public static readonly bool isMono = false;
#endif
        public static Type findType(Instance instance, string csharpId)
        {
            lock(_mutex)
            {
                Type t;
                if (_typeTable.TryGetValue(csharpId, out t))
                {
                    return t;
                }

                loadAssemblies(); // Lazy initialization
                foreach (Assembly a in _loadedAssemblies.Values)
                {
                    if((t = a.GetType(csharpId)) != null)
                    {
                        _typeTable[csharpId] = t;
                        return t;
                    }
                }
            }
            return null;
        }

        public static Type[] findTypesWithPrefix(string prefix)
        {
            LinkedList<Type> l = new LinkedList<Type>();

            lock(_mutex)
            {
                loadAssemblies(); // Lazy initialization
                foreach(Assembly a in _loadedAssemblies.Values)
                {
                    try
                    {
                        Type[] types = a.GetTypes();
                        foreach(Type t in types)
                        {
                            if(t.AssemblyQualifiedName.IndexOf(prefix, StringComparison.Ordinal) == 0)
                            {
                                l.AddLast(t);
                            }
                        }
                    }
                    catch(ReflectionTypeLoadException)
                    {
                        // Failed to load types from the assembly, ignore and continue
                    }
                }
            }

            Type[] result = new Type[l.Count];
            if(l.Count > 0)
            {
                l.CopyTo(result, 0);
            }
            return result;
        }

        public static object createInstance(Type t)
        {
            try
            {
                return Activator.CreateInstance(t);
            }
            catch(MemberAccessException)
            {
                return null;
            }
        }

        public static void preloadAssemblies()
        {
            lock(_mutex)
            {
                loadAssemblies(); // Lazy initialization
            }
        }

        //
        // Make sure that all assemblies that are referenced by this process
        // are actually loaded. This is necessary so we can use reflection
        // on any type in any assembly because the type we are after will
        // most likely not be in the current assembly and, worse, may be
        // in an assembly that has not been loaded yet. (Type.GetType()
        // is no good because it looks only in the calling object's assembly
        // and mscorlib.dll.)
        //
        private static void loadAssemblies()
        {
            Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();
            List<Assembly> newAssemblies = null;
            foreach(Assembly a in assemblies)
            {
                if(!_loadedAssemblies.Contains(a.FullName))
                {
                    if(newAssemblies == null)
                    {
                        newAssemblies = new List<Assembly>();
                    }
                    newAssemblies.Add(a);
                    _loadedAssemblies[a.FullName] = a;
                }
            }
            if(newAssemblies != null)
            {
                foreach(Assembly a in newAssemblies)
                {
                    loadReferencedAssemblies(a);
                }
            }
        }

        private static void loadReferencedAssemblies(Assembly a)
        {
            try
            {
                AssemblyName[] names = a.GetReferencedAssemblies();
                foreach(AssemblyName name in names)
                {
                    if(!_loadedAssemblies.ContainsKey(name.FullName))
                    {
                        try
                        {
                            Assembly ra = Assembly.Load(name);
                            //
                            // The value of name.FullName may not match that of ra.FullName, so
                            // we record the assembly using both keys.
                            //
                            _loadedAssemblies[name.FullName] = ra;
                            _loadedAssemblies[ra.FullName] = ra;
                            loadReferencedAssemblies(ra);
                        }
                        catch(Exception)
                        {
                            // Ignore assemblies that cannot be loaded.
                        }
                    }
                }
            }
            catch(PlatformNotSupportedException)
            {
                // Some platforms like UWP do not support using GetReferencedAssemblies
            }
        }

        private static Hashtable _loadedAssemblies = new Hashtable(); // <string, Assembly> pairs.
        private static Dictionary<string, Type> _typeTable = new Dictionary<string, Type>(); // <type name, Type> pairs.
        private static object _mutex = new object();
    }
}
