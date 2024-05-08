// Copyright (c) ZeroC, Inc.

using System.Collections.Concurrent;
using System.Collections.Immutable;
using System.Globalization;
using System.Linq.Expressions;
using System.Reflection;

namespace Ice.Internal;

// Instantiates a Slice class or exception.
internal delegate object ActivateObject();

/// <summary>The default implementation of <see cref="IActivator" />, which uses a dictionary.</summary>
internal class Activator : IActivator
{
    internal static Activator Empty { get; } =
        new Activator(ImmutableDictionary<string, Lazy<ActivateObject>>.Empty);

    private readonly IReadOnlyDictionary<string, Lazy<ActivateObject>> _dict;

    public object CreateInstance(string typeId) =>
        _dict.TryGetValue(typeId, out Lazy<ActivateObject> factory) ? factory.Value() : null;

    /// <summary>Merge activators into a single activator; duplicate entries are ignored.</summary>
    internal static Activator Merge(IEnumerable<Activator> activators)
    {
        if (activators.Count() == 1)
        {
            return activators.First();
        }
        else
        {
            var dict = new Dictionary<string, Lazy<ActivateObject>>();

            foreach (Activator activator in activators)
            {
                foreach ((string typeId, Lazy<ActivateObject> factory) in activator._dict)
                {
                    dict[typeId] = factory;
                }
            }
            return dict.Count == 0 ? Empty : new Activator(dict);
        }
    }

    internal Activator(IReadOnlyDictionary<string, Lazy<ActivateObject>> dict) => _dict = dict;
}

/// <summary>Creates activators from assemblies by processing types in those assemblies.</summary>
internal class ActivatorFactory
{
    internal static ActivatorFactory Instance { get; } = new ActivatorFactory();

    private readonly ConcurrentDictionary<Assembly, Activator> _cache = new();

    internal Activator Get(Assembly assembly)
    {
        if (_cache.TryGetValue(assembly, out Activator activator))
        {
            return activator;
        }
        else if (assembly.GetCustomAttributes<SliceAttribute>().Any())
        {
            return _cache.GetOrAdd(
                assembly,
                assembly =>
                {
                    var dict = new Dictionary<string, Lazy<ActivateObject>>();

                    foreach (Type type in assembly.GetTypes())
                    {
                        // We're only interested in generated Slice classes and exceptions.
                        if (type.IsClass && type.GetSliceTypeId() is string typeId)
                        {
                            var lazy = new Lazy<ActivateObject>(() => CreateActivateObject(type));

                            dict.Add(typeId, lazy);

                            if (type.GetCompactSliceTypeId() is int compactTypeId)
                            {
                                dict.Add(compactTypeId.ToString(CultureInfo.InvariantCulture), lazy);
                            }
                        }
                    }

                    // Merge with the activators of the referenced assemblies (recursive call)
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

        static ActivateObject CreateActivateObject(Type type)
        {
            ConstructorInfo constructor = type.GetConstructor(
                BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic,
                null,
                [],
                null) ?? throw new InvalidOperationException($"Cannot find parameterless constructor for '{type}'.");

            return Expression.Lambda<ActivateObject>(Expression.New(constructor)).Compile();
        }
    }

    private ActivatorFactory()
    {
        // ensures it's a singleton.
    }
}
