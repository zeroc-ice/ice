// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.Reflection;

    public sealed class AssemblyUtil
    {
        public enum Runtime { DotNET, Mono };
        public enum Platform { Windows, NonWindows };

        static AssemblyUtil()
        {
            PlatformID id = Environment.OSVersion.Platform;
            if(   id == PlatformID.Win32NT
               || id == PlatformID.Win32S
               || id == PlatformID.Win32Windows
               || id == PlatformID.WinCE)
            {
                platform_ = Platform.Windows;
            }
            else
            {
                platform_ = Platform.NonWindows;
            }

            if(System.Type.GetType("Mono.Runtime") != null)
            {
                runtime_ = Runtime.Mono;
            }
            else
            {
                runtime_ = Runtime.DotNET;
            }

            System.Version v = System.Environment.Version;
            runtimeMajor_ = v.Major;
            runtimeMinor_ = v.Minor;
            runtimeBuild_ = v.Build;
            runtimeRevision_ = v.Revision;

            v = System.Environment.OSVersion.Version;

            osx_ = false;
            if (platform_ == Platform.NonWindows)
            {
                try
                {
                    Assembly a = Assembly.Load(
                        "Mono.Posix, Version=2.0.0.0, Culture=neutral, PublicKeyToken=0738eb9f132ed756");
                    Type syscall = a.GetType("Mono.Unix.Native.Syscall");
                    if(syscall != null)
                    {
                        MethodInfo method = syscall.GetMethod("uname", BindingFlags.Static | BindingFlags.Public);
                        if(method != null)
                        {
                            object[] p = new object[1];
                            method.Invoke(null, p);
                            if(p[0] != null)
                            {
                                Type utsname = a.GetType("Mono.Unix.Native.Utsname");
                                osx_ = ((string)utsname.GetField("sysname").GetValue(p[0])).Equals("Darwin");
                            }
                        }
                    }
                }
                catch(System.Exception)
                {
                }
            }
        }

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
                    Type[] types = a.GetTypes();
                    foreach(Type t in types)
                    {
                        if(t.AssemblyQualifiedName.IndexOf(prefix, StringComparison.Ordinal) == 0)
                        {
                            l.AddLast(t);
                        }
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
                    catch(System.Exception)
                    {
                        // Ignore assemblies that cannot be loaded.
                    }
                }
            }
        }

        private static Hashtable _loadedAssemblies = new Hashtable(); // <string, Assembly> pairs.
        private static Dictionary<string, Type> _typeTable = new Dictionary<string, Type>(); // <type name, Type> pairs.
        private static object _mutex = new object();

        public readonly static Runtime runtime_; // Either DotNET or Mono
        //
        // Versioning is: Major.Minor.Build.Revision. (Yes, really. It is not Major.Minor.Revision.Build, as
        // one might expect.) If a part of a version number (such as revision) is not defined, it is -1.
        //
        public readonly static int runtimeMajor_;
        public readonly static int runtimeMinor_;
        public readonly static int runtimeBuild_;
        public readonly static int runtimeRevision_;

        public readonly static Platform platform_;
        public readonly static bool osx_;
    }

}
