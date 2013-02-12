// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;

class Grammar
{
    internal Grammar(Parser p)
    {
        _parser = p;
        _scanner = new Scanner(_parser);
    }

    internal virtual void parse()
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
                    IList s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    if(s.Count == 0)
                    {
                        throw new ParseError("usage: mkfile FILE [FILE...]");
                    }
                    _parser.createFile(s);
                }
                else if(_token.type == Token.TOK_CREATE_DIR)
                {
                    IList s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    if(s.Count == 0)
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
                    IList s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    if(s.Count > 1)
                    {
                        throw new ParseError("usage: cd [DIR]");
                    }
                    else if(s.Count == 0)
                    {
                        _parser.cd("/");
                    }
                    else
                    {
                        _parser.cd((string) s[0]);
                    }
                }
                else if(_token.type == Token.TOK_CAT)
                {
                    IList s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    if(s.Count != 1)
                    {
                        throw new ParseError("usage: cat FILE");
                    }
                    _parser.cat((string) s[0]);
                }
                else if(_token.type == Token.TOK_WRITE)
                {
                    IList s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    if(s.Count == 0)
                    {
                        throw new ParseError("usage: write FILE [STRING...]");
                    }
                    _parser.write(s);
                }
                else if(_token.type == Token.TOK_RM)
                {
                    IList s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    if(s.Count == 0)
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
                _parser.error("Parse error: " + e.Message);
            }
        }
    }

    private IList strings()
    {
        ArrayList l = new ArrayList();
        while(true)
        {
            _token = _scanner.nextToken();
            if(_token.type != Token.TOK_STRING)
            {
                return l;
            }
            l.Add(_token.@value);
        }
    }

    private class ParseError : SystemException
    {
        internal ParseError(string msg)
            : base(msg)
        {
        }
    }

    private Parser _parser;
    private Scanner _scanner;
    private Token _token;
}
