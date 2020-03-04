//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_STORM_PARSER_H
#define ICE_STORM_PARSER_H

#include <IceStorm/IceStorm.h>
#include <list>
#include <stdio.h>

//
// Stuff for flex and bison
//

#define YYSTYPE std::list<std::string>
#define YY_DECL int yylex(YYSTYPE* yylvalp)
YY_DECL;

//
// I must set the initial stack depth to the maximum stack depth to
// disable bison stack resizing. The bison stack resizing routines use
// simple malloc/alloc/memcpy calls, which do not work for the
// YYSTYPE, since YYSTYPE is a C++ type, with constructor, destructor,
// assignment operator, etc.
//
#define YYMAXDEPTH  10000
#define YYINITDEPTH YYMAXDEPTH // Initial depth is set to max depth, for the reasons described above.

//
// Newer bison versions allow to disable stack resizing by defining
// yyoverflow.
//
#define yyoverflow(a, b, c, d, e, f) yyerror(a)

namespace IceStorm
{

class Parser
{
public:

    Parser(std::shared_ptr<Ice::Communicator>, std::shared_ptr<TopicManagerPrx>,
           std::map<Ice::Identity, std::shared_ptr<TopicManagerPrx>>);

    void usage();

    void create(const std::list<std::string>&);
    void destroy(const std::list<std::string>&);
    void link(const std::list<std::string>&);
    void unlink(const std::list<std::string>&);
    void links(const std::list<std::string>&);
    void topics(const std::list<std::string>&);
    void replica(const std::list<std::string>&);
    void subscribers(const std::list<std::string>&);
    void current(const std::list<std::string>&);

    void showBanner();

    //
    // With older flex version <= 2.5.35 YY_INPUT second
    // paramenter is of type int&, in newer versions it
    // changes to size_t&
    //
    void getInput(char*, int&, size_t);
    void getInput(char*, size_t&, size_t);

    void continueLine();
    const char* getPrompt();

    void error(const char*);
    void error(const std::string&);

    void warning(const char*);
    void warning(const std::string&);

    void invalidCommand(const std::string&);

    int parse(FILE*, bool);
    int parse(const std::string&, bool);

private:

    std::shared_ptr<TopicManagerPrx> findManagerById(const std::string&, std::string&) const;
    std::shared_ptr<TopicManagerPrx> findManagerByCategory(const std::string&) const;
    std::shared_ptr<TopicPrx> findTopic(const std::string&) const;

    void exception(std::exception_ptr, bool = false);

    const std::shared_ptr<Ice::Communicator> _communicator;
    std::shared_ptr<TopicManagerPrx> _defaultManager;
    const std::map<Ice::Identity, std::shared_ptr<TopicManagerPrx>> _managers;
    std::string _commands;
    bool _continue;
    int _errors;
};

extern Parser* parser; // The current parser for bison/flex

} // End namespace IceStorm

#endif
