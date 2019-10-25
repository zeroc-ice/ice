//
// Copyright (c) ZeroC, Inc. All rights reserved.
//
namespace Ice
{
    public partial interface ObjectFactory
    {
        /// <summary>
        /// Create a new object for a given object type.
        ///
        /// <para>ObjectFactory has been deprecated, use ValueFactory instead.</para>
        ///
        /// The type is the
        /// absolute Slice type id, i.e., the id relative to the
        /// unnamed top-level Slice module. For example, the absolute
        /// Slice type id for interfaces of type Bar in the module
        /// Foo is "::Foo::Bar".
        ///
        /// The leading "::" is required.
        ///
        /// </summary>
        /// <param name="type">The object type.
        ///
        /// </param>
        /// <returns>The object created for the given type, or nil if the
        /// factory is unable to create the object.</returns>
        [global::System.Obsolete("ObjectFactory has been deprecated, use ValueFactory instead.")]
        Value create(string type);

        /// <summary>
        /// Called when the factory is removed from the communicator, or if
        /// the communicator is destroyed.
        ///
        /// <para>ObjectFactory has been deprecated, use ValueFactory instead.</para>
        ///
        /// </summary>
        [global::System.Obsolete("ObjectFactory has been deprecated, use ValueFactory instead.")]
        void destroy();
    }
}
