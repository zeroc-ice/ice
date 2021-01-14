//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Test
{
    enum Enum1 : varint {A, B, C}

    enum Enum2 : long {A, B , C}

    struct S
    {
        int x;
    }

    enum Enum3 : S {A, B = -1, C}
}
