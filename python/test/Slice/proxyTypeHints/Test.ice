// Copyright (c) ZeroC, Inc.

#pragma once

["python:identifier:generated.test.Slice.proxyTypeHints.Test"]
module Test
{
    sequence<int> IntSeq;

    interface ProxyHints
    {
        // The "python:numpy.ndarray" metadata is on the parameter, not the sequence definition, so it is
        // only reflected in the generated code if the parameter's metadata is taken into account. The proxy
        // method signature must include the corresponding type hint, matching the servant side.
        void op(["python:numpy.ndarray"] IntSeq values);
    }
}
