// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

module classdef // Should be escaped
{
    module break // Should be escaped
    {
        enum bitand // Should not be escaped
        {
            //
            // All of the keywords as well as a few reserved method names.
            //
            break, case, catch, classdef, continue, else, elseif, end, enumeration, events, for, function, global,
            if, methods, otherwise, parfor, persistent, properties, return, spmd, switch, try, while, abs, and, char,
            eq, length, size, xor, LAST
        }

        struct bitor // Should not be escaped
        {
            bitand case = catch;
            int continue = 1;
            int eq = 2;
        }

        class logical // Should not be escaped
        {
            bitand else = enumeration;
            bitor for;
            bool int64 = true;
            int size = 1;
        }

        class xor extends logical // Should not be escaped
        {
            int return = 1;
            int char = 2;
        }

        sequence<bitor> parfor;
        dictionary<int, bitor> switch;

        class try // Should be escaped
        {
            int while = 1;
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
            //
            // Reserved method and keyword.
            //
            int length = 1;
            logical end;
        }

        exception global extends persistent // Should be escaped
        {
            int ne = 1;
        }

        interface elseif // elseifPrx should not be escaped
        {
            void events();
            void function();
            void not();
            void round();
            void delete();
            void checkedCast();
        }

        const int methods = 1;
    }
}
