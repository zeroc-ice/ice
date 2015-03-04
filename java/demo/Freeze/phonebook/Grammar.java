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
                else if(_token.type == Token.TOK_ADD_CONTACTS)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.addContacts(s);
                }
                else if(_token.type == Token.TOK_FIND_CONTACTS)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.findContacts(s);
                }
                else if(_token.type == Token.TOK_NEXT_FOUND_CONTACT)
                {
                    _token = _scanner.nextToken();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }

                    _parser.nextFoundContact();
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
                else if(_token.type == Token.TOK_SET_CURRENT_NAME)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.setCurrentName(s);
                }
                else if(_token.type == Token.TOK_SET_CURRENT_ADDRESS)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.setCurrentAddress(s);
                }
                else if(_token.type == Token.TOK_SET_CURRENT_PHONE)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.setCurrentPhone(s);
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
                else if(_token.type == Token.TOK_SET_EVICTOR_SIZE)
                {
                    java.util.List<String> s = strings();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }
                    _parser.setEvictorSize(s);
                }
                else if(_token.type == Token.TOK_SHUTDOWN)
                {
                    _token = _scanner.nextToken();
                    if(_token.type != Token.TOK_SEMI)
                    {
                        throw new ParseError("Expected ';'");
                    }

                    _parser.shutdown();
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
