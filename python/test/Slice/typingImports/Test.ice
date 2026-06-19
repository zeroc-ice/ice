// Copyright (c) ZeroC, Inc.

#pragma once

["python:identifier:generated.test.Slice.typingImports.Test"]
module Test
{
    ["python:numpy.ndarray"] sequence<int> IntSeq;

    // IntSeq is only used in operation signatures, so the generated code for this interface
    // only needs numpy for type hints: the numpy import must be typing-only.
    interface TypingImports
    {
        IntSeq op(IntSeq values);
    }
}
