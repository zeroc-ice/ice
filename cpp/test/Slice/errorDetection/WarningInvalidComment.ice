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
    /// @something: This is an unknown comment tag which spans 1 line.
    /// @see: CommentDummy
    ///       But then we write a 2nd line, which isn't allowed for 'see' tags.
    /// @bad: Another unknown comment tag, but this will span 2 lines.
    ///       This 2nd line should be ignored, but won't trigger another error.
    exception CommentDummy {}

    /// @deprecated Message1
    /// @deprecated Message2
    exception DerivedFromDummy extends CommentDummy {}

    exception SomeOtherException {}

    interface MyInterface
    {
        /// @param NoParam should give a no-matching-parameter warning.
        /// @return Nothing because it's void.
        /// @throws FakeException should give a not-an-exception warning.
        /// @throws CommentDummy should give a not-thrown-by-this-operation warning.
        void voidOp();

        /// @param value no warning, because this ia a real parameter.
        /// @param value should give a duplicate-tag warning.
        /// @param namess should give a no-matching-parameter warning.
        /// @return Something because it's non-void.
        /// @return Should give a duplicate-tag warning.
        /// @throws DerivedFromDummy should be fine, since this is a sub-exception of `CommentDummy`.
        /// @throws CommentDummy should be fine, this matches the exception specification.
        /// @throws SomeOtherException should give a not-thrown-by-this-operation warning.
        /// @throws CommentDummy should give a duplicate-tag warning.
        string stringOp(string name, int value) throws CommentDummy;
    }
}
