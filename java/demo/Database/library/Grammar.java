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
                else if(_token.type == Token.TOK_ADD_BOOK)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.addBook(s);
                }
                else if(_token.type == Token.TOK_FIND_ISBN)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.findIsbn(s);
                }
                else if(_token.type == Token.TOK_FIND_AUTHORS)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.findAuthors(s);
                }
                else if(_token.type == Token.TOK_FIND_TITLE)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.findTitle(s);
                }
                else if(_token.type == Token.TOK_NEXT_FOUND_BOOK)
                {
                    _token = _scanner.nextToken();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }

                    _parser.nextFoundBook();
                }
                else if(_token.type == Token.TOK_PRINT_CURRENT)
                {
                    _token = _scanner.nextToken();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }

                    _parser.printCurrent();
                }
                else if(_token.type == Token.TOK_RENT_BOOK)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.rentCurrent(s);
                }
                else if(_token.type == Token.TOK_RETURN_BOOK)
                {
                    _token = _scanner.nextToken();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.returnCurrent();
                }
                else if(_token.type == Token.TOK_REMOVE_CURRENT)
                {
                    _token = _scanner.nextToken();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }

                    _parser.removeCurrent();
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

    private java.util.List<String>
    strings()
    {
        java.util.List<String> l = new java.util.ArrayList<String>();
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
