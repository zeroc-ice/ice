// Copyright (c) ZeroC, Inc.

// Mark some generated server-side classes as EditorBrowsableState.Never so that docfx does not generate documentation
// pages for them.

using System.ComponentModel;

namespace Ice;

#pragma warning disable SA1601 // PartialElementsMustBeDocumented

[EditorBrowsable(EditorBrowsableState.Never)]
public partial interface LoggerAdmin
{
}

[EditorBrowsable(EditorBrowsableState.Never)]
public abstract partial class LoggerAdminDisp_
{
}

[EditorBrowsable(EditorBrowsableState.Never)]
public partial interface Process
{
}

[EditorBrowsable(EditorBrowsableState.Never)]
public abstract partial class ProcessDisp_
{
}
