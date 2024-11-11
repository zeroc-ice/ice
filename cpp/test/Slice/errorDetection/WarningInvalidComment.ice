
module Test
{
    /// This is a test overview.
    /// @remarks: This is an unknown comment tag which spans 1 line.
    /// @see: CommentDummy
    ///       But then we write a 2nd line, which isn't allowed for 'see' tags.
    /// @bad: Another unknown comment tag, but this will span 2 lines.
    ///       This 2nd line should be ignored, but won't trigger another error.
    class CommentDummy {}
}
