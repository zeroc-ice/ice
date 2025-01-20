// Copyright (c) ZeroC, Inc.

#ifndef ICESTORM_PARSER_H
#define ICESTORM_PARSER_H

#include "IceStorm/IceStorm.h"

#include <list>
#include <stdio.h>

namespace IceStorm
{
    class Parser
    {
    public:
        Parser(Ice::CommunicatorPtr, TopicManagerPrx, std::map<Ice::Identity, TopicManagerPrx>);

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
        TopicManagerPrx findManagerById(const std::string&, std::string&) const;
        [[nodiscard]] TopicManagerPrx findManagerByCategory(const std::string&) const;
        [[nodiscard]] std::optional<TopicPrx> findTopic(const std::string&) const;

        void exception(std::exception_ptr, bool = false);

        const Ice::CommunicatorPtr _communicator;
        TopicManagerPrx _defaultManager;
        const std::map<Ice::Identity, TopicManagerPrx> _managers;
        std::string _commands;
        bool _continue;
        int _errors;
    };

    extern Parser* parser; // The current parser for bison/flex

} // End namespace IceStorm

#endif
