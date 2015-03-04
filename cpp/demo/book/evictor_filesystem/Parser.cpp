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
using namespace Filesystem;

extern FILE* yyin;

Parser* parser;

Parser::Parser(const DirectoryPrx& root)
{
    _dirs.push_front(root);
}

void
Parser::usage()
{
    cout <<
        "help                    Print this message.\n"
        "pwd                     Print current directory (/ = root).\n"
        "cd [DIR]                Change directory (/ or empty = root).\n"
        "ls                      List current directory.\n"
        "lr                      Recursively list current directory.\n"
        "mkdir DIR [DIR...]      Create directories DIR in current directory.\n"
        "mkfile FILE [FILE...]   Create files FILE in current directory.\n"
        "rm NAME [NAME...]       Delete directory or file NAME (rm * to delete all).\n"
        "cat FILE                List the contents of FILE.\n"
        "write FILE [STRING...]  Write STRING to FILE.\n"
        "exit, quit              Exit this program.\n";
}

// Print the contents of directory "dir". If recursive is true,
// print in tree fashion. 
// For files, show the contents of each file. The "depth"
// parameter is the current nesting level (for indentation).

void
Parser::list(bool recursive)
{
    list(_dirs.front(), recursive, 0);
}

void
Parser::list(const DirectoryPrx& dir, bool recursive, int depth)
{
    string indent(depth++, '\t');

    NodeDescSeq contents = dir->list();

    for(NodeDescSeq::const_iterator i = contents.begin(); i != contents.end(); ++i)
    {
        DirectoryPrx d = i->type == DirType ? DirectoryPrx::uncheckedCast(i->proxy) : (DirectoryPrx)0;
        cout << indent << i->name << (d ? " (directory)" : " (file)");
        if(d && recursive)
        {
            cout << ":" << endl;
            list(d, true, depth);
        }
        else
        {
            cout << endl;
        }
    }
}

void
Parser::createFile(const std::list<string>& names)
{
    DirectoryPrx dir = _dirs.front();

    for(std::list<string>::const_iterator i = names.begin(); i != names.end(); ++i)
    {
        if(*i == "..")
        {
            cout << "Cannot create a file named `..'" << endl;
            continue;
        }

        try
        {
            dir->createFile(*i);
        }
        catch(const NameInUse&)
        {
            cout << "`" << *i << "' exists already" << endl;
        }
    }
}

void
Parser::createDir(const std::list<string>& names)
{
    DirectoryPrx dir = _dirs.front();

    for(std::list<string>::const_iterator i = names.begin(); i != names.end(); ++i)
    {
        if(*i == "..")
        {
            cout << "Cannot create a directory named `.'" << endl;
            continue;
        }

        try
        {
            dir->createDirectory(*i);
        }
        catch(const NameInUse&)
        {
            cout << "`" << *i << "' exists already" << endl;
        }
    }
}

void
Parser::pwd()
{
    if(_dirs.size() == 1)
    {
        cout << "/";
    }
    else
    {
        // COMPILERFIX: Should be const_reverse_iterator, but that won't compile with
        // gcc because operator!= is missing.
        //
        std::list<DirectoryPrx>::reverse_iterator i = _dirs.rbegin();
        ++i;
        while(i != _dirs.rend())
        {
            cout << "/" << (*i)->name();
            ++i;
        }
    }
    cout << endl;
}

void
Parser::cd(const string& name)
{
    if(name == "/")
    {
       while(_dirs.size() > 1)
       {
           _dirs.pop_front();
       }
       return;
    }

    if(name == "..")
    {
       if(_dirs.size() > 1)
       {
           _dirs.pop_front();
       }
       return;
    }

    DirectoryPrx dir = _dirs.front();
    NodeDesc d;
    try
    {
        d = dir->find(name);
    }
    catch(const NoSuchName&)
    {
        cout << "`" << name << "': no such directory" << endl;
        return;
    }
    if(d.type == FileType)
    {
        cout << "`" << name << "': not a directory" << endl;
        return;
    }
    _dirs.push_front(DirectoryPrx::uncheckedCast(d.proxy));
}

void
Parser::cat(const string& name)
{
    DirectoryPrx dir = _dirs.front();
    NodeDesc d;
    try
    {
        d = dir->find(name);
    }
    catch(const NoSuchName&)
    {
        cout << "`" << name << "': no such file" << endl;
        return;
    }
    if(d.type == DirType)
    {
        cout << "`" << name << "': not a file" << endl;
        return;
    }
    FilePrx f = FilePrx::uncheckedCast(d.proxy);
    Lines l = f->read();
    for(Lines::const_iterator i = l.begin(); i != l.end(); ++i)
    {
        cout << *i << endl;
    }
}

void
Parser::write(std::list<string>& args)
{
    DirectoryPrx dir = _dirs.front();
    string name = args.front();
    args.pop_front();
    NodeDesc d;
    try
    {
        d = dir->find(name);
    }
    catch(const NoSuchName&)
    {
        cout << "`" << name << "': no such file" << endl;
        return;
    }
    if(d.type == DirType)
    {
        cout << "`" << name << "': not a file" << endl;
        return;
    }
    FilePrx f = FilePrx::uncheckedCast(d.proxy);

    Lines l;
    for(std::list<string>::const_iterator i = args.begin(); i != args.end(); ++i)
    {
        l.push_back(*i);
    }
    f->write(l);
}

void
Parser::destroy(const std::list<string>& names)
{
    DirectoryPrx dir = _dirs.front();

    for(std::list<string>::const_iterator i = names.begin(); i != names.end(); ++i)
    {
        if(*i == "*")
        {
            NodeDescSeq nodes = dir->list();
            for(NodeDescSeq::iterator j = nodes.begin(); j != nodes.end(); ++j)
            {
                try
                {
                    j->proxy->destroy();
                }
                catch(const PermissionDenied& ex)
                {
                    cout << "cannot remove `" << j->name << "': " << ex.reason << endl;
                }
            }
            return;
        }
        else
        {
            NodeDesc d;
            try
            {
                d = dir->find(*i);
            }
            catch(const NoSuchName&)
            {
                cout << "`" << *i << "': no such file or directory" << endl;
                return;
            }
            try
            {
                d.proxy->destroy();
            }
            catch(const PermissionDenied& ex)
            {
                cout << "cannot remove `" << *i << "': " << ex.reason << endl;
            }
        }
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
        return "> ";
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

    int status = yyparse();
    if(_errors)
    {
        status = EXIT_FAILURE;
    }

    parser = 0;
    return status;
}
