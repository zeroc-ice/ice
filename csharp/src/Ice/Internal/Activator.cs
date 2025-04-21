// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Collections.Concurrent;
using System.Collections.Immutable;
using System.Globalization;
using System.Reflection;

namespace Ice.Internal;

/// <summary>Represents the actual implementation of <see cref="AssemblySliceLoader"/>.</summary>
internal class Activator
{
    internal static Activator Empty { get; } = new Activator(ImmutableDictionary<string, Type>.Empty);

    private readonly IReadOnlyDictionary<string, Type> _dict;

    /// <summary>Merge activators into a single activator; duplicate entries are ignored.</summary>
    internal static Activator Merge(IEnumerable<Activator> activators)
    {
        if (activators.Count() == 1)
        {
            return activators.First();
        }
        else
        {
            var dict = new Dictionary<string, Type>();

            foreach (Activator activator in activators)
            {
                foreach ((string typeId, Type factory) in activator._dict)
                {
                    dict[typeId] = factory;
                }
            }
            return dict.Count == 0 ? Empty : new Activator(dict);
        }
    }

    internal Activator(IReadOnlyDictionary<string, Type> dict) => _dict = dict;

    internal object? CreateInstance(string typeId) =>
        _dict.TryGetValue(typeId, out Type? type) ? System.Activator.CreateInstance(type) : null;
}

/// <summary>Creates activators from assemblies by processing types in those assemblies.</summary>
internal class ActivatorFactory
{
    internal static ActivatorFactory Instance { get; } = new ActivatorFactory();

    private readonly ConcurrentDictionary<Assembly, Activator> _cache = new();

    internal Activator Get(Assembly assembly)
    {
        if (_cache.TryGetValue(assembly, out Activator? activator))
        {
            return activator;
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

                    // Merge with the activators of the referenced assemblies (recursive call).
                    return Activator.Merge(
                        assembly.GetReferencedAssemblies().Select(
                            assemblyName => Get(AppDomain.CurrentDomain.Load(assemblyName))).Append(
                                new Activator(dict)));
                });
        }
        else
        {
            // We don't cache an assembly with no Slice attribute, and don't load/process its referenced assemblies.
            return Activator.Empty;
        }
    }

    private ActivatorFactory()
    {
        // ensures it's a singleton.
    }
}
