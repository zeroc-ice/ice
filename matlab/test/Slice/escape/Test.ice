//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module classdef // Should be escaped
{
    module break // Should be escaped
    {
        enum bitand // Should not be escaped
        {
            //
            // All of the keywords.
            //
            break, case, catch, classdef, continue, else, elseif, end, enumeration, events, for, function, global,
            if, methods, otherwise, parfor, persistent, properties, return, spmd, switch, try, while,
            //
            // Plus a few method names reserved for enumerators because the enumeration class derives from uint8
            // or int32.
            //
            abs, and, char, eq, length, size, xor,
            LAST
        }

        struct bitor // Should not be escaped
        {
            bitand case = catch;
            int continue = 1;
            int eq = 2;
            int ne = 3;
        }

        class logical // Should not be escaped
        {
            bitand else = enumeration;
            bitor for;
            bool int64 = true; // Should not be escaped
        }

        class xor extends logical // Should not be escaped
        {
            int return = 1;
        }

        sequence<bitor> parfor;
        dictionary<int, bitor> switch;

        class try // Should be escaped
        {
            int while = 1;
            int delete = 2; // Should not be escaped
        }

        class properties extends try // Should be escaped
        {
            int if = 2;
            xor catch;
            parfor spmd;
            switch otherwise;
        }

        exception persistent // Should be escaped
        {
            //
            // These symbols clash with members of MException and should be escaped.
            //
            string identifier = "1";
            string message = "2";
            string stack = "3";
            string cause = "4";
            string type = "5";

            logical end;
        }

        exception global extends persistent // Should be escaped
        {
            int enumeration = 1;
        }

        interface elseif // elseifPrx should not be escaped
        {
            void events();
            void function();
            void delete();
            void checkedCast();
        }

        const int methods = 1;
    }
}
