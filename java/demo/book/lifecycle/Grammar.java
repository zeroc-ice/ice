// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class Grammar
{
    Grammar(Parser p)
    {
        _parser = p;
        _scanner = new Scanner(_parser);
    }

    void
    parse()
    {
        while(true)
        {
            try
            {
                _token = _scanner.nextToken();
                if(_token == null)
                {
                    return;
                }
                else if(_token.type == Token.TOK_SEMI)
                {
                    // Continue
                }
                else if(_token.type == Token.TOK_HELP)
                {
                    _token = _scanner.nextToken();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.usage();
                }
                else if(_token.type == Token.TOK_EXIT)
                {
                    _token = _scanner.nextToken();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    return;
                }
                else if(_token.type == Token.TOK_LIST)
                {
                    _token = _scanner.nextToken();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.list(false);
                }
                else if(_token.type == Token.TOK_LIST_RECURSIVE)
                {
                    _token = _scanner.nextToken();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.list(true);
                }
                else if(_token.type == Token.TOK_CREATE_FILE)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    if(s.size() == 0)
                    {
                        throw new ParseError("usage: mkfile FILE [FILE...]");
                    }
                    _parser.createFile(s);
                }
                else if(_token.type == Token.TOK_CREATE_DIR)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    if(s.size() == 0)
                    {
                        throw new ParseError("usage: mkdir DIR [DIR...]");
                    }
                    _parser.createDir(s);
                }
                else if(_token.type == Token.TOK_PWD)
                {
                    _token = _scanner.nextToken();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.pwd();
                }
                else if(_token.type == Token.TOK_CD)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    if(s.size() > 1)
                    {
                        throw new ParseError("usage: cd [DIR]");
                    }
                    else if(s.size() == 0)
                    {
                        _parser.cd("/");
                    }
                    else
                    {
                        _parser.cd((String)s.get(0));
                    }
                }
                else if(_token.type == Token.TOK_CAT)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    if(s.size() != 1)
                    {
                        throw new ParseError("usage: cat FILE");
                    }
                    _parser.cat((String)s.get(0));
                }
                else if(_token.type == Token.TOK_WRITE)
                {
                    java.util.LinkedList<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    if(s.size() == 0)
                    {
                        throw new ParseError("usage: write FILE [STRING...]");
                    }
                    _parser.write(s);
                }
                else if(_token.type == Token.TOK_RM)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    if(s.size() == 0)
                    {
                        throw new ParseError("usage: rm NAME [NAME...]");
                    }
                    _parser.destroy(s);
                }
                else
                {
                    _parser.error("parse error");
                }
            }
            catch(ParseError e)
            {
                _parser.error("Parse error: " + e.getMessage());
            }
        }
    }

    private java.util.LinkedList<String>
    strings()
    {
        java.util.LinkedList<String> l = new java.util.LinkedList<String>();
        while(true)
        {
            _token = _scanner.nextToken();
            if(_token.type != Token.TOK_STRING)
            {
                return l;
            }
            l.add(_token.value);
        }
    }

    static private class ParseError extends RuntimeException
    {
        ParseError(String msg)
        {
            super(msg);
        }
    }

    private Parser _parser;
    private Scanner _scanner;
    private Token _token;
}
