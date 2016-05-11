// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;

namespace TestCommon
{
    public abstract class TestApp
    {
        protected static void test(bool b)
        {
            if(!b)
            {
                Debug.Assert(false);
                throw new Exception();
            }
        }


        public static void Write(string msg)
        {
            Console.Out.Write(msg);
        }

        public static void WriteLine(string msg)
        {
            Console.Out.WriteLine(msg);
        }

        public static void Flush()
        { 
            Console.Out.Flush();
        }
    }
}
