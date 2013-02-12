// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Parser.h>

#ifdef HAVE_READLINE
#   include <readline/readline.h>
#   include <readline/history.h>
#endif

using namespace std;
using namespace Demo;

extern FILE* yyin;

Parser* parser;

void
Parser::usage()
{
    cout <<
        "help                    Print this message.\n"
        "exit, quit              Exit this program.\n"
        "add isbn title authors  Create new book.\n"
        "isbn NUMBER             Find the book with given ISBN number.\n"
        "authors NAME            Find all books by the given authors.\n"
        "next                    Set the current book to the next one that was found.\n"
        "current                 Display the current book.\n"
        "rent NAME               Rent the current book for customer NAME.\n"
        "return                  Return the currently rented book.\n"
        "remove                  Permanently remove the current book from the library.\n"
        "size SIZE               Set the evictor size for books to SIZE.\n"
        "shutdown                Shut the library server down.\n";
}

Parser::Parser(const LibraryPrx& library) :
    _library(library)
{
}

void
Parser::addBook(const list<string>& _args)
{
    if(_args.size() != 3)
    {
        error("`add' requires at exactly three arguments (type `help' for more info)");
        return;
    }

    try
    {
        list<string> args = _args;

        BookDescription desc;
        desc.isbn = args.front();
        args.pop_front();
        desc.title = args.front();
        args.pop_front();
        desc.authors = args.front();

        BookPrx book = _library->createBook(desc);
        cout << "added new book with isbn " << desc.isbn << endl;
    }
    catch(const DatabaseException& ex)
    {
        error(ex.message);
    }
    catch(const BookExistsException&)
    {
        error("the book already exists.");
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}

void
Parser::findIsbn(const list<string>& args)
{
    if(args.size() != 1)
    {
        error("`isbn' requires exactly one argument (type `help' for more info)");
        return;
    }

    try
    {
        _foundBooks.clear();
        _current = _foundBooks.begin();

        BookPrx book = _library->findByIsbn(args.front());
        if(!book)
        {
            cout << "no book with that ISBN number exists." << endl;
        }
        else
        {
            _foundBooks.push_back(book);
            _current = _foundBooks.begin();
            printCurrent();
        }
    }
    catch(const DatabaseException& ex)
    {
        error(ex.message);
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}

void
Parser::findAuthors(const list<string>& args)
{
    if(args.size() != 1)
    {
        error("`authors' requires exactly one argument (type `help' for more info)");
        return;
    }

    try
    {
        _foundBooks = _library->findByAuthors(args.front());
        _current = _foundBooks.begin();
        cout << "number of books found: " << _foundBooks.size() << endl;
        printCurrent();
    }
    catch(const DatabaseException& ex)
    {
        error(ex.message);
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}

void
Parser::nextFoundBook()
{
    if(_current != _foundBooks.end())
    {
        ++_current;
    }
    printCurrent();
}

void
Parser::printCurrent()
{
    try
    {
        if(_current != _foundBooks.end())
        {
            BookDescription desc = (*_current)->getBookDescription();
            string renter;
            try
            {
                renter = (*_current)->getRenterName();
            }
            catch(const BookNotRentedException&)
            {
            }

            cout << "current book is:" << endl;
            cout << "isbn: " << desc.isbn << endl;
            cout << "title: " << desc.title << endl;
            cout << "authors: " << desc.authors << endl;
            if(!renter.empty())
            {
                cout << "rented: " << renter << endl;
            }
        }
        else
        {
            cout << "no current book" << endl;
        }
    }
    catch(const Ice::ObjectNotExistException&)
    {
        cout << "current book no longer exists" << endl;
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}

void
Parser::rentCurrent(const list<string>& args)
{
    if(args.size() != 1)
    {
        error("`rent' requires exactly one argument (type `help' for more info)");
        return;
    }

    try
    {
        if(_current != _foundBooks.end())
        {
            (*_current)->rentBook(args.front());
            cout << "the book is now rented by `" << args.front() << "'" << endl;
        }
        else
        {
            cout << "no current book" << endl;
        }
    }
    catch(const BookRentedException&)
    {
        cout << "the book has already been rented." << endl;
    }
    catch(const Ice::ObjectNotExistException&)
    {
        cout << "current book no longer exists" << endl;
    }
    catch(const DatabaseException& ex)
    {
        error(ex.message);
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}

void
Parser::returnCurrent()
{
    try
    {
        if(_current != _foundBooks.end())
        {
            (*_current)->returnBook();
            cout << "the book has been returned." << endl;
        }
        else
        {
            cout << "no current book" << endl;
        }
    }
    catch(const BookNotRentedException&)
    {
        cout << "the book is not currently rented." << endl;
    }
    catch(const Ice::ObjectNotExistException&)
    {
        cout << "current book no longer exists" << endl;
    }
    catch(const DatabaseException& ex)
    {
        error(ex.message);
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}

void
Parser::removeCurrent()
{
    try
    {
        if(_current != _foundBooks.end())
        {
            (*_current)->destroy();
            cout << "removed current book" << endl;
        }
        else
        {
            cout << "no current book" << endl;
        }
    }
    catch(const Ice::ObjectNotExistException&)
    {
        cout << "current book no longer exists" << endl;
    }
    catch(const DatabaseException& ex)
    {
        error(ex.message);
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}

void
Parser::setEvictorSize(const list<string>& args)
{
    if(args.size() != 1)
    {
        error("`size' requires exactly one argument (type `help' for more info)");
        return;
    }

    try
    {
        _library->setEvictorSize(atoi(args.front().c_str()));
    }
    catch(const DatabaseException& ex)
    {
        error(ex.message);
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}

void
Parser::shutdown()
{
    try
    {
        _library->shutdown();
    }
    catch(const Ice::Exception& ex)
    {
        ostringstream s;
        s << ex;
        error(s.str());
    }
}

void
Parser::getInput(char* buf, int& result, int maxSize)
{
#ifdef HAVE_READLINE

    const char* prompt = parser->getPrompt();
    char* line = readline(const_cast<char*>(prompt));
    if(!line)
    {
        result = 0;
    }
    else
    {
        if(*line)
        {
            add_history(line);
        }
        
        result = strlen(line) + 1;
        if(result > maxSize)
        {
            free(line);
            error("input line too long");
            result = 0;
        }
        else
        {
            strcpy(buf, line);
            strcat(buf, "\n");
            free(line);
        }
    }
    
#else

    cout << parser->getPrompt() << flush;

    string line;
    while(true)
    {
        char c = static_cast<char>(getc(yyin));
        if(c == EOF)
        {
            if(line.size())
            {
                line += '\n';
            }
            break;
        }

        line += c;

        if(c == '\n')
        {
            break;
        }
    }

    result = static_cast<int>(line.length());
    if(result > maxSize)
    {
        error("input line too long");
        buf[0] = EOF;
        result = 1;
    }
    else
    {
        strcpy(buf, line.c_str());
    }
#endif
}

void
Parser::continueLine()
{
    _continue = true;
}

const char*
Parser::getPrompt()
{
    if(_continue)
    {
        _continue = false;
        return "(cont) ";
    }
    else
    {
        return ">>> ";
    }
}

void
Parser::error(const char* s)
{
    cerr << "error: " << s << endl;
    _errors++;
}

void
Parser::error(const string& s)
{
    error(s.c_str());
}

void
Parser::warning(const char* s)
{
    cerr << "warning: " << s << endl;
}

void
Parser::warning(const string& s)
{
    warning(s.c_str());
}

int
Parser::parse(bool debug)
{
    extern int yydebug;
    yydebug = debug ? 1 : 0;

    assert(!parser);
    parser = this;

    _errors = 0;
    yyin = stdin;
    assert(yyin);

    _continue = false;

    _foundBooks.clear();
    _current = _foundBooks.end();

    int status = yyparse();
    if(_errors)
    {
        status = EXIT_FAILURE;
    }

    parser = 0;
    return status;
}
