// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module Test
{
    module SubA
    {
        module SubSubA1
        {
            const int Value2 = 11;
        };
        module SubSubA2
        {
            const int Value1 = 30;
        };
    };

    module SubB
    {
        module SubSubB1
        {
            const int Value2 = 21;
        };
    };
};
