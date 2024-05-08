// Copyright (c) ZeroC, Inc.

using System.Reflection;

namespace Ice.Internal;

/// <summary>Provides methods that create class and exception instances from Slice type IDs.</summary>
/// <remarks>It's a simplified version of ZeroC.Slice.IActivator.</remarks>
internal interface IActivator
{
    /// <summary>Gets or creates an activator for the Slice types in the specified assembly and its referenced
    /// assemblies.</summary>
    /// <param name="assembly">The assembly.</param>
    /// <returns>An activator that activates the Slice types defined in <paramref name="assembly" /> provided this
    /// assembly contains generated code (as determined by the presence of the <see cref="SliceAttribute" /> attribute).
    /// The Slice types defined in assemblies referenced by <paramref name="assembly" /> are included as well,
    /// recursively. If a referenced assembly contains no generated code, the assemblies it references are not examined.
    /// </returns>
    internal static IActivator FromAssembly(Assembly assembly) => ActivatorFactory.Instance.Get(assembly);

    /// <summary>Gets or creates an activator for the Slice types defined in the specified assemblies and their
    /// referenced assemblies.</summary>
    /// <param name="assemblies">The assemblies.</param>
    /// <returns>An activator that activates the Slice types defined in <paramref name="assemblies" /> and their
    /// referenced assemblies. See <see cref="FromAssembly(Assembly)" />.</returns>
    internal static IActivator FromAssemblies(params Assembly[] assemblies) =>
        Internal.Activator.Merge(assemblies.Select(ActivatorFactory.Instance.Get));

    /// <summary>Creates an instance of a Slice class or exception based on a type ID.</summary>
    /// <param name="typeId">The Slice type ID.</param>
    /// <returns>A new instance of the class or exception identified by <paramref name="typeId" />, or null if the
    /// implementation cannot find the corresponding class.</returns>
    /// <remarks>This implementation of this method can also throw an exception if the class is found but the activation
    /// of an instance fails.</remarks>
    object CreateInstance(string typeId);
}
