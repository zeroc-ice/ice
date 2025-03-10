// Copyright (c) ZeroC, Inc.

module Test
{
    /// Test that operation specific tags don't work on non-operations.
    /// @param foo Not real.
    /// @deprecated This one is fine.
    /// @throws Exception never.
    /// @exception Exception synonym for @throws.
    /// @return Nothing because it's an enum.
    enum NotAnOperation {
        /// @see CommentDummy
        Okay,
        /// @see
        MissingLinkTarget,
        /// @see CommentDummy.
        EndsWithInvalidPeriod,
    }

    /// This is a test overview.
    /// @remarks: This is an unknown comment tag which spans 1 line.
    /// @see: CommentDummy
    ///       But then we write a 2nd line, which isn't allowed for 'see' tags.
    /// @bad: Another unknown comment tag, but this will span 2 lines.
    ///       This 2nd line should be ignored, but won't trigger another error.
    class CommentDummy {}
}
