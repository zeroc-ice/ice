// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_PARSER_H
#define ICE_GRID_PARSER_H

#include <IceUtil/Handle.h>
#include <IceGrid/Admin.h>
#include <IceGrid/Registry.h>
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
#define YYMAXDEPTH  10000 // 10000 should suffice. Bison default is 10000 as maximum.
#define YYINITDEPTH YYMAXDEPTH // Initial depth is set to max depth, for the reasons described above.

//
// Newer bison versions allow to disable stack resizing by defining
// yyoverflow.
//
#define yyoverflow(a, b, c, d, e, f) yyerror(a)

namespace IceGrid
{

class Parser;
typedef ::IceUtil::Handle<Parser> ParserPtr;

}

namespace IceGrid
{

class Parser : public ::IceUtil::SimpleShared, IceUtil::Monitor<IceUtil::Mutex>
{
public:

    static ParserPtr createParser(const Ice::CommunicatorPtr&, const AdminSessionPrx&, const AdminPrx&, bool);

    void usage();
    void usage(const std::string&, const std::string& = std::string());
    void usage(const std::string&, const std::list<std::string>&);

    void interrupt();
    bool interrupted() const;
    void resetInterrupt();
    void checkInterrupted();

    void addApplication(const std::list<std::string>&);
    void removeApplication(const std::list<std::string>&);
    void describeApplication(const std::list<std::string>&);
    void diffApplication(const std::list<std::string>&);
    void updateApplication(const std::list<std::string>&);
    void patchApplication(const std::list<std::string>&);
    void listAllApplications(const std::list<std::string>&);

    void describeServerTemplate(const std::list<std::string>&);
    void describeServiceTemplate(const std::list<std::string>&);
    void instantiateServerTemplate(const std::list<std::string>&);

    void describeNode(const std::list<std::string>&);
    void pingNode(const std::list<std::string>&);
    void printLoadNode(const std::list<std::string>&);
    void printNodeProcessorSockets(const std::list<std::string>&);
    void shutdownNode(const std::list<std::string>&);
    void listAllNodes(const std::list<std::string>&);

    void describeRegistry(const std::list<std::string>&);
    void pingRegistry(const std::list<std::string>&);
    void shutdownRegistry(const std::list<std::string>&);
    void listAllRegistries(const std::list<std::string>&);

    void removeServer(const std::list<std::string>&);
    void startServer(const std::list<std::string>&);
    void stopServer(const std::list<std::string>&);
    void patchServer(const std::list<std::string>&);
    void signalServer(const std::list<std::string>&);
    void writeMessage(const std::list<std::string>&, int fd);
    void describeServer(const std::list<std::string>&);
    void stateServer(const std::list<std::string>&);
    void enableServer(const std::list<std::string>&, bool);
    void pidServer(const std::list<std::string>&);
    void propertiesServer(const std::list<std::string>&, bool);
    void listAllServers(const std::list<std::string>&);

    void startService(const std::list<std::string>&);
    void stopService(const std::list<std::string>&);
    void propertiesService(const std::list<std::string>&, bool);
    void describeService(const std::list<std::string>&);
    void listServices(const std::list<std::string>&);

    void endpointsAdapter(const std::list<std::string>&);
    void removeAdapter(const std::list<std::string>&);
    void listAllAdapters(const std::list<std::string>&);

    void addObject(const std::list<std::string>&);
    void removeObject(const std::list<std::string>&);
    void findObject(const std::list<std::string>&);
    void describeObject(const std::list<std::string>&);
    void listObject(const std::list<std::string>&);

    void show(const std::string&, const std::list<std::string>&);

    void showBanner();
    void showCopying();
    void showWarranty();

    //
    // With older flex version <= 2.5.35 YY_INPUT second
    // paramenter is of type int&, in newer versions it
    // changes to size_t&
    //
    void getInput(char*, int&, size_t);
    void getInput(char*, size_t&, size_t);
    void continueLine();
    const char* getPrompt();

    void invalidCommand(const char*);
    void invalidCommand(const std::string&);
    void invalidCommand(const std::string&, const std::string&);

    void invalidCommand(const std::list<std::string>&);

    std::string patchFailed(const Ice::StringSeq&);
    void error(const char*);
    void error(const std::string&);

    void warning(const char*);
    void warning(const std::string&);

    int parse(FILE*, bool);
    int parse(const std::string&, bool);

private:

    Parser(const Ice::CommunicatorPtr&, const AdminSessionPrx&, const AdminPrx&, bool);
    void exception(const Ice::Exception&);

    void showFile(const std::string&, const std::string&, const std::string&, bool, bool, bool, int);
    void showLog(const std::string&, const std::string&, bool, bool, int);

    std::string _commands;
    Ice::CommunicatorPtr _communicator;
    AdminSessionPrx _session;
    AdminPrx _admin;
    bool _continue;
    bool _interrupted;
    int _errors;
    bool _interactive;
    std::map< std::string, std::map<std::string, std::string> > _helpCommands;
};

extern Parser* parser; // The current parser for bison/flex

}

#endif
