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
    public static final int TOK_ADD_BOOK = 2;
    public static final int TOK_FIND_ISBN = 3;
    public static final int TOK_FIND_AUTHORS = 4;
    public static final int TOK_FIND_TITLE = 5;
    public static final int TOK_NEXT_FOUND_BOOK = 6;
    public static final int TOK_PRINT_CURRENT = 7;
    public static final int TOK_RENT_BOOK = 8;
    public static final int TOK_RETURN_BOOK = 9;
    public static final int TOK_REMOVE_CURRENT = 10;
    public static final int TOK_STRING = 11;
    public static final int TOK_SEMI = 12;
    
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
