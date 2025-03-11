// Copyright (c) ZeroC, Inc.

// Mark some generated server-side classes as EditorBrowsableState.Never so that docfx does not generate documentation
// pages for them.

using System.ComponentModel;

namespace IceStorm;

#pragma warning disable SA1601 // PartialElementsMustBeDocumented
[EditorBrowsable(EditorBrowsableState.Never)]
public partial interface Finder
{
}

[EditorBrowsable(EditorBrowsableState.Never)]
public abstract partial class FinderDisp_
{
}

[EditorBrowsable(EditorBrowsableState.Never)]
public partial interface Topic
{
}

[EditorBrowsable(EditorBrowsableState.Never)]
public abstract partial class TopicDisp_
{
}

[EditorBrowsable(EditorBrowsableState.Never)]
public partial interface TopicManager
{
}

[EditorBrowsable(EditorBrowsableState.Never)]
public abstract partial class TopicManagerDisp_
{
}
