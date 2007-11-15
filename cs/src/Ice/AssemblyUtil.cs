// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System;
    using System.Collections;
    using System.Diagnostics;
    using System.Reflection;
    using System.Threading;

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
        }

        public static Type findType(string csharpId)
        {
            loadAssemblies(); // Lazy initialization

            lock(_mutex)
            {
                Type t = (Type)_typeTable[csharpId];
                if(t != null)
                {
                    return t;
                }
                foreach(Assembly a in _loadedAssemblies.Values)
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
            IceUtil.LinkedList l = new IceUtil.LinkedList();

            loadAssemblies(); // Lazy initialization

            lock(_mutex)
            {
                foreach(Assembly a in _loadedAssemblies.Values)
                {
                    Type[] types = a.GetTypes();
                    foreach(Type t in types)
                    {
                        if(t.AssemblyQualifiedName.IndexOf(prefix) == 0)
                        {
                            l.Add(t);
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
            ConstructorInfo[] constructors = t.GetConstructors();

            if(constructors.Length == 0)
            {
                return null;
            }

            ParameterInfo[] firstConstructor = constructors[0].GetParameters();

            int paramCount = firstConstructor.Length;
            Type[] constructor = new Type[paramCount];
            for(int i = 0; i < paramCount; i++)
            {
                constructor[i] = firstConstructor[i].ParameterType;
            }

            return t.GetConstructor(constructor).Invoke(new object[]{});
        }

        //
        // Make sure that all assemblies that are referenced by this process
        // are actually loaded. This is necessary so we can use reflection
        // on any type in any assembly (because the type we are after will
        // most likely not be in the current assembly and, worse, may be
        // in an assembly that has not been loaded yet. (Type.GetType()
        // is no good because it looks only in the calling object's assembly
        // and mscorlib.dll.)
        //
        private static void loadAssemblies()
        {
            lock(_mutex)
            {
                if(!_assembliesLoaded)
                {
                    Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();
                    foreach(Assembly a in assemblies)
                    {
                        _loadedAssemblies[a.FullName] = a;
                    }
                    foreach(Assembly a in assemblies)
                    {
                        loadReferencedAssemblies(a);
                    }
                    _assembliesLoaded = true;
                }
            }
        }

        private static void loadReferencedAssemblies(Assembly a)
        {
            AssemblyName[] names = a.GetReferencedAssemblies();
            foreach(AssemblyName name in names)
            {
                if(!_loadedAssemblies.Contains(name.FullName))
                {
                    Assembly ra = Assembly.Load(name);
                    _loadedAssemblies[ra.FullName] = ra;
                    loadReferencedAssemblies(ra);
                }
            }
        }

        private static bool _assembliesLoaded = false;
        private static Hashtable _loadedAssemblies = new Hashtable(); // <string, Assembly> pairs.
        private static Hashtable _typeTable = new Hashtable(); // <type name, Type> pairs.
        private static Mutex _mutex = new Mutex();

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
    }

}
