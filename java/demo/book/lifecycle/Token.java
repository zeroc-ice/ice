// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class Token
{
    public static final int TOK_HELP = 0;
    public static final int TOK_EXIT = 1;
    public static final int TOK_SEMI = 2;
    public static final int TOK_LIST = 3;
    public static final int TOK_LIST_RECURSIVE = 4;
    public static final int TOK_CREATE_FILE = 5;
    public static final int TOK_CREATE_DIR = 6;
    public static final int TOK_PWD = 7;
    public static final int TOK_CD = 8;
    public static final int TOK_CAT = 9;
    public static final int TOK_WRITE = 10;
    public static final int TOK_RM = 11;
    public static final int TOK_STRING = 12;

    int type;
    String value;

    Token(int t)
    {
        type = t;
        value = null;
    }

    Token(int t, String v)
    {
        type = t;
        value = v;
    }
}
