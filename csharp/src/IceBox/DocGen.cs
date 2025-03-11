// Copyright (c) ZeroC, Inc.

// Mark some generated server-side classes as EditorBrowsableState.Never so that docfx does not generate documentation
// pages for them.

using System.ComponentModel;

namespace IceBox;

#pragma warning disable SA1601 // PartialElementsMustBeDocumented

[EditorBrowsable(EditorBrowsableState.Never)]
public partial interface ServiceManager
{
}

[EditorBrowsable(EditorBrowsableState.Never)]
public abstract partial class ServiceManagerDisp_
{
}
