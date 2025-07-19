// Copyright (c) ZeroC, Inc.

module Test
{
    enum
    {
        A,
        B = 2,
        C = 2, // Make sure we still correctly parse inside the malformed enum.
    }

    sequence<MissingType>; // Make sure we still validate inside the malformed sequence.
    dictionary<int, MissingType>; // Make sure we still validate inside the malformed dictionary.

    const string = 45; // Make sure we still validate inside the malformed constant.
}
