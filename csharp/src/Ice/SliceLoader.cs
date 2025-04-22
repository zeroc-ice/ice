// Copyright (c) ZeroC, Inc.

#nullable enable

using Ice.Internal;
using System.Reflection;

namespace Ice;

/// <summary>Creates class and exception instances from Slice type IDs.</summary>
public interface SliceLoader
{
    /// <summary>Creates an instance of a class mapped from a Slice class or exception based on a Slice type ID.
    /// </summary>
    /// <param name="typeId">The Slice type ID or compact type ID.</param>
    /// <returns>A new instance of the class or exception identified by <paramref name="typeId"/>, or <c>null</c> if the
    /// implementation cannot find the corresponding class.</returns>
    /// <exception cref="MarshalException">Thrown when the corresponding class was found but its instantiation failed.
    /// </exception>
    object? newInstance(string typeId);

    /// <summary>Retrieves the <see cref="SliceLoader"/> for the generated classes in the specified assemblies.
    /// </summary>
    /// <param name="assemblies">The assemblies to search.</param>
    /// <returns>A Slice loader for the specified assemblies.</returns>
    /// <remarks>The Slice classes defined in assemblies referenced by <paramref name="assemblies" /> are included,
    /// recursively, provided they contain generated code (as determined by the presence of the
    /// <see cref="SliceAttribute" /> attribute).</remarks>
    public static SliceLoader fromAssemblies(params Assembly[] assemblies) =>
        AssemblySliceLoader.Merge(assemblies.Select(AssemblySliceLoaderFactory.Instance.Get));

    /// <summary>Retrieves the <see cref="SliceLoader"/> for the generated classes in the specified assembly.
    /// </summary>
    /// <param name="assembly">The assembly to search.</param>
    /// <returns>A Slice loader for the specified assembly.</returns>
    /// <remarks>The Slice classes defined in assemblies referenced by <paramref name="assembly" /> are included,
    /// recursively, provided they contain generated code (as determined by the presence of the
    /// <see cref="SliceAttribute" /> attribute).</remarks>
    public static SliceLoader fromAssembly(Assembly assembly) => fromAssemblies(assembly);
}
