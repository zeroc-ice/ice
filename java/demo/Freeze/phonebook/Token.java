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
    public static final int TOK_ADD_CONTACTS = 2;
    public static final int TOK_FIND_CONTACTS = 3;
    public static final int TOK_NEXT_FOUND_CONTACT = 4;
    public static final int TOK_PRINT_CURRENT = 5;
    public static final int TOK_SET_CURRENT_NAME = 6;
    public static final int TOK_SET_CURRENT_ADDRESS = 7;
    public static final int TOK_SET_CURRENT_PHONE = 8;
    public static final int TOK_REMOVE_CURRENT = 9;
    public static final int TOK_SET_EVICTOR_SIZE = 10;
    public static final int TOK_SHUTDOWN = 11;
    public static final int TOK_STRING = 12;
    public static final int TOK_SEMI = 13;
    
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
