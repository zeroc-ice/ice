//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

module Underscore_Test
{
    interface Test_Interface_A
    {
        void op_A();
    }

    class Test_Class_B implements Test_Interface_A
    {
        void op_B();

        int int_member;
    }

    struct Test_Struct
    {
        string string_member;
    }

    exception Test_Exception
    {
        short short_member;
    }

    sequence<string> String_Sequence;
    dictionary<string, string> String_Dictionary;

    const int Const_Int = 99;
}
