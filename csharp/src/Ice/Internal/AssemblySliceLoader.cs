// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Collections.Concurrent;
using System.Collections.Immutable;
using System.Globalization;
using System.Reflection;

namespace Ice.Internal;

/// <summary>Implements <see cref="SliceLoader" /> by searching for generated classes in the specified assemblies and
/// their referenced assemblies.</summary>
internal sealed class AssemblySliceLoader : SliceLoader
{
    internal static AssemblySliceLoader Empty { get; } =
        new AssemblySliceLoader(ImmutableDictionary<string, Type>.Empty);

    private readonly IReadOnlyDictionary<string, Type> _dict;

    /// <inheritdoc />
    public object? newInstance(string typeId)
    {
        if (_dict.TryGetValue(typeId, out Type? type))
        {
            try
            {
                return Activator.CreateInstance(type);
            }
            catch (System.Exception exception)
            {
                throw new MarshalException(
                    $"Failed to create an instance of class '{type.FullName}' for type ID '{typeId}'.", exception);
            }
        }
        else
        {
            return null;
        }
    }

    /// <summary>Merge loaders into a single loader; duplicate entries are ignored.</summary>
    internal static AssemblySliceLoader Merge(IEnumerable<AssemblySliceLoader> loaders)
    {
        if (loaders.Count() == 1)
        {
            return loaders.First();
        }
        else
        {
            var dict = new Dictionary<string, Type>();

            foreach (AssemblySliceLoader loader in loaders)
            {
                foreach ((string typeId, Type factory) in loader._dict)
                {
                    dict[typeId] = factory;
                }
            }
            return dict.Count == 0 ? Empty : new AssemblySliceLoader(dict);
        }
    }

    internal AssemblySliceLoader(IReadOnlyDictionary<string, Type> dict) => _dict = dict;
}

/// <summary>Creates Slice loaders from assemblies by mapping types in these assemblies.</summary>
internal sealed class AssemblySliceLoaderFactory
{
    internal static AssemblySliceLoaderFactory Instance { get; } = new AssemblySliceLoaderFactory();

    private readonly ConcurrentDictionary<Assembly, AssemblySliceLoader> _cache = new();

    internal AssemblySliceLoader Get(Assembly assembly)
    {
        if (_cache.TryGetValue(assembly, out AssemblySliceLoader? loader))
        {
            return loader;
        }
        else if (assembly.GetCustomAttributes<SliceAttribute>().Any())
        {
            return _cache.GetOrAdd(
                assembly,
                assembly =>
                {
                    var dict = new Dictionary<string, Type>();

                    foreach (Type type in assembly.GetTypes())
                    {
                        // We're only interested in generated Slice classes and exceptions.
                        if (type.IsClass && type.GetSliceTypeId() is string typeId)
                        {
                            dict.Add(typeId, type);

                            if (type.GetCompactSliceTypeId() is int compactTypeId)
                            {
                                dict.Add(compactTypeId.ToString(CultureInfo.InvariantCulture), type);
                            }
                        }
                    }

                    // Merge with the loaders of the referenced assemblies (recursive call).
                    return AssemblySliceLoader.Merge(
                        assembly.GetReferencedAssemblies().Select(
                            assemblyName => Get(AppDomain.CurrentDomain.Load(assemblyName))).Append(
                                new AssemblySliceLoader(dict)));
                });
        }
        else
        {
            // We don't cache an assembly with no Slice attribute, and don't load/process its referenced assemblies.
            return AssemblySliceLoader.Empty;
        }
    }

    private AssemblySliceLoaderFactory()
    {
        // ensures it's a singleton.
    }
}
