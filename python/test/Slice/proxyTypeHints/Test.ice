// Copyright (c) ZeroC, Inc.

#pragma once

["python:identifier:generated.test.Slice.proxyTypeHints.Test"]
module Test
{
    sequence<int> IntSeq;

    interface ProxyHints
    {
        // The "python:numpy.ndarray" metadata is on the parameter, not the sequence definition, so it is
        // only reflected in the generated code if the parameter's metadata is taken into account. Both the
        // method signature and the docstring "Parameters" section must include the corresponding type hint,
        // matching the servant side.

        /// Does something with the values.
        /// @param values the values to use.
        void op(["python:numpy.ndarray"] IntSeq values);
    }
}
