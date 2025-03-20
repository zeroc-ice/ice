// Copyright (c) ZeroC, Inc.

using System.Collections;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Ice.Internal;

public static class AssemblyUtil
{
    public static readonly bool isWindows = RuntimeInformation.IsOSPlatform(OSPlatform.Windows);
    public static readonly bool isMacOS = RuntimeInformation.IsOSPlatform(OSPlatform.OSX);
    public static readonly bool isLinux = RuntimeInformation.IsOSPlatform(OSPlatform.Linux);

    public static object createInstance(Type t)
    {
        try
        {
            return System.Activator.CreateInstance(t);
        }
        catch (MemberAccessException)
        {
            return null;
        }
    }

    public static void preloadAssemblies()
    {
        lock (_mutex)
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
        foreach (Assembly a in assemblies)
        {
            if (!_loadedAssemblies.Contains(a.FullName))
            {
                newAssemblies ??= new List<Assembly>();
                newAssemblies.Add(a);
                _loadedAssemblies[a.FullName] = a;
            }
        }
        if (newAssemblies != null)
        {
            foreach (Assembly a in newAssemblies)
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
            foreach (AssemblyName name in names)
            {
                if (!_loadedAssemblies.ContainsKey(name.FullName))
                {
                    try
                    {
                        var ra = Assembly.Load(name);
                        //
                        // The value of name.FullName may not match that of ra.FullName, so
                        // we record the assembly using both keys.
                        //
                        _loadedAssemblies[name.FullName] = ra;
                        _loadedAssemblies[ra.FullName] = ra;
                        loadReferencedAssemblies(ra);
                    }
                    catch (System.Exception)
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

    private static readonly Hashtable _loadedAssemblies = []; // <string, Assembly> pairs.
    private static readonly object _mutex = new();
}
