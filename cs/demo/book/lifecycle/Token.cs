// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

class Token
{
    public const int TOK_HELP = 0;
    public const int TOK_EXIT = 1;
    public const int TOK_SEMI = 2;
    public const int TOK_LIST = 3;
    public const int TOK_LIST_RECURSIVE = 4;
    public const int TOK_CREATE_FILE = 5;
    public const int TOK_CREATE_DIR = 6;
    public const int TOK_PWD = 7;
    public const int TOK_CD = 8;
    public const int TOK_CAT = 9;
    public const int TOK_WRITE = 10;
    public const int TOK_RM = 11;
    public const int TOK_STRING = 12;

    internal int type;
    internal string @value;

    internal Token(int t)
    {
        type = t;
        @value = null;
    }

    internal Token(int t, string v)
    {
        type = t;
        @value = v;
    }
}
