//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;

namespace ZeroC.Ice
{
    public static class AssemblyUtil
    {
        public static readonly bool IsLinux = RuntimeInformation.IsOSPlatform(OSPlatform.Linux);
        public static readonly bool IsMacOS = RuntimeInformation.IsOSPlatform(OSPlatform.OSX);
        public static readonly bool IsMono = RuntimeInformation.FrameworkDescription.Contains("Mono");
        public static readonly bool IsWindows = RuntimeInformation.IsOSPlatform(OSPlatform.Windows);

        private static readonly Dictionary<string, Assembly> _loadedAssemblies = new Dictionary<string, Assembly>();
        // <type name, Type> pairs.
        private static readonly Dictionary<string, Type> _typeTable = new Dictionary<string, Type>();
        private static readonly object _mutex = new object();

        // Register a delegate to load native libraries used by Ice assembly.
        static AssemblyUtil() =>
            NativeLibrary.SetDllImportResolver(Assembly.GetAssembly(typeof(AssemblyUtil))!, DllImportResolver);

        public static Type? FindType(string csharpId)
        {
            lock (_mutex)
            {
                if (_typeTable.TryGetValue(csharpId, out Type? t))
                {
                    return t;
                }

                LoadAssemblies(); // Lazy initialization
                foreach (Assembly a in _loadedAssemblies.Values)
                {
                    if ((t = a.GetType(csharpId)) != null)
                    {
                        _typeTable[csharpId] = t;
                        return t;
                    }
                }
            }
            return null;
        }

        public static void PreloadAssemblies()
        {
            lock (_mutex)
            {
                LoadAssemblies(); // Lazy initialization
            }
        }

        internal static string[] GetPlatformNativeLibraryNames(string name)
        {
            if (name == "bzip2")
            {
                if (IsWindows)
                {
                    return new string[] { "bzip2.dll" };
                }
                else if (IsMacOS)
                {
                    return new string[] { "libbz2.dylib" };
                }
                else
                {
                    return new string[] { "libbz2.so.1.0", "libbz2.so.1", "libbz2.so" };
                }
            }
            return Array.Empty<string>();
        }

        private static IntPtr DllImportResolver(string libraryName, Assembly assembly, DllImportSearchPath? searchPath)
        {
            DllNotFoundException? failure = null;
            foreach (string name in GetPlatformNativeLibraryNames(libraryName))
            {
                try
                {
                    return NativeLibrary.Load(name, assembly, searchPath);
                }
                catch (DllNotFoundException ex)
                {
                    failure = ex;
                }
            }
            Debug.Assert(failure != null);
            throw failure;
        }

        // Make sure that all assemblies that are referenced by this process are actually loaded. This is necessary so
        // we can use reflection on any type in any assembly because the type we are after will most likely not be in
        // the current assembly and, worse, may be in an assembly that has not been loaded yet. (Type.GetType() is no
        // good because it looks only in the calling object's assembly and mscorlib.dll.)
        private static void LoadAssemblies()
        {
            Assembly[] assemblies = AppDomain.CurrentDomain.GetAssemblies();
            var newAssemblies = new List<Assembly>();
            foreach (Assembly assembly in assemblies)
            {
                if (!_loadedAssemblies.ContainsKey(assembly.FullName!))
                {
                    newAssemblies.Add(assembly);
                    _loadedAssemblies[assembly.FullName!] = assembly;
                }
            }

            foreach (Assembly a in newAssemblies)
            {
                LoadReferencedAssemblies(a);
            }
        }

        private static void LoadReferencedAssemblies(Assembly a)
        {
            try
            {
                AssemblyName[] names = a.GetReferencedAssemblies();
                foreach (AssemblyName name in names)
                {
                    if (!_loadedAssemblies.ContainsKey(name.FullName))
                    {
                        try
                        {
                            var loadedAssembly = Assembly.Load(name);
                            // The value of name.FullName may not match that of loadedAssembly.FullName, so we record
                            // the assembly using both keys.
                            _loadedAssemblies[name.FullName] = loadedAssembly;
                            _loadedAssemblies[loadedAssembly.FullName!] = loadedAssembly;
                            LoadReferencedAssemblies(loadedAssembly);
                        }
                        catch (Exception)
                        {
                            // Ignore assemblies that cannot be loaded.
                        }
                    }
                }
            }
            catch (PlatformNotSupportedException)
            {
                // Some platforms like UWP do not support using GetReferencedAssemblies
            }
        }
    }
}
