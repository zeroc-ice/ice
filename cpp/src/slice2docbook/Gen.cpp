// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/Functional.h>
#include <Gen.h>

#ifdef __BCPLUSPLUS__
#  include <iterator>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;

Slice::Gen::Gen(const string& name, const string& file, bool standAlone, bool chapter,
                bool noIndex, bool sortFields) :
    _standAlone(standAlone),
    _noIndex(noIndex),
    _sortFields(sortFields)
{
    if(chapter)
    {
        _chapter = "chapter";
    }
    else
    {
        _chapter = "section";
    }
        
    O.open(file.c_str());
    if(!O)
    {
        cerr << name << ": can't open `" << file << "' for writing: " << strerror(errno) << endl;
        return;
    }
}

Slice::Gen::~Gen()
{
}

bool
Slice::Gen::operator!() const
{
    return !O;
}

void
Slice::Gen::generate(const UnitPtr& p)
{
    p->mergeModules();

    //
    // I don't want the top-level module to be sorted, therefore no
    // p->sort() before or after the p->sortContents().
    //
    p->sortContents(_sortFields);

    p->visit(this, false);
}

bool
Slice::Gen::visitUnitStart(const UnitPtr& p)
{
    if(_standAlone)
    {
        O << "<!DOCTYPE article PUBLIC \"-//OASIS//DTD DocBook V3.1//EN\">";
        printHeader();
        start("article");
    }
    else
    {
        printHeader();
    }

    return true;
}

void
Slice::Gen::visitUnitEnd(const UnitPtr& p)
{
    if(_standAlone)
    {
        end();
    }
}

bool
Slice::Gen::visitModuleStart(const ModulePtr& p)
{
    start(_chapter + " id=" + containedToId(p), p->scoped().substr(2));

    string metadata, deprecateReason;
    if(p->findMetaData("deprecate", metadata))
    {
        deprecateReason = "This module has been deprecated.";
        if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
        {
            deprecateReason = metadata.substr(10);
        }
    }

    start("section", "Overview", false);
    O.zeroIndent();
    O << nl << "<synopsis>";
    printMetaData(p);
    O << "module <classname>" << p->name() << "</classname></synopsis>";
    O.restoreIndent();
    printComment(p, deprecateReason);
    visitContainer(p);
    end();

    return true;
}

void
Slice::Gen::visitContainer(const ContainerPtr& p)
{
    ModuleList modules = p->modules();
    modules.erase(remove_if(modules.begin(), modules.end(), ::IceUtil::constMemFun(&Contained::includeLevel)),
                  modules.end());

    if(!modules.empty() && !_noIndex)
    {
        start("section", "Module Index", false);
        start("variablelist");
        for(ModuleList::const_iterator q = modules.begin(); q != modules.end(); ++q)
        {
            start("varlistentry");
            start("term");
            O << toString(*q, p);
            end();
            start("listitem");
            string metadata;
            printSummary(*q, (*q)->findMetaData("deprecate", metadata));
            end();
            end();
        }
        end();
        end();
    }

    ClassList classesAndInterfaces = p->classes();
    classesAndInterfaces.erase(remove_if(classesAndInterfaces.begin(), classesAndInterfaces.end(),
                                         ::IceUtil::constMemFun(&Contained::includeLevel)),
                               classesAndInterfaces.end());
    ClassList classes;
    ClassList interfaces;
    remove_copy_if(classesAndInterfaces.begin(), classesAndInterfaces.end(), back_inserter(classes),
                   ::IceUtil::constMemFun(&ClassDef::isInterface));
    remove_copy_if(classesAndInterfaces.begin(), classesAndInterfaces.end(), back_inserter(interfaces),
                   not1(::IceUtil::constMemFun(&ClassDef::isInterface)));

    if(!classes.empty() && !_noIndex)
    {
        start("section", "Class Index", false);
        start("variablelist");
        for(ClassList::const_iterator q = classes.begin(); q != classes.end(); ++q)
        {
            start("varlistentry");
            start("term");
            O << toString(*q, p);
            end();
            start("listitem");
            string metadata;
            printSummary(*q, (*q)->findMetaData("deprecate", metadata));
            end();
            end();
        }
        end();
        end();
    }

    if(!interfaces.empty() && !_noIndex)
    {
        start("section", "Interface Index", false);
        start("variablelist");
        for(ClassList::const_iterator q = interfaces.begin(); q != interfaces.end(); ++q)
        {
            start("varlistentry");
            start("term");
            O << toString(*q, p);
            end();
            start("listitem");
            string metadata;
            printSummary(*q, (*q)->findMetaData("deprecate", metadata));
            end();
            end();
        }
        end();
        end();
    }

    ExceptionList exceptions = p->exceptions();
    exceptions.erase(remove_if(exceptions.begin(), exceptions.end(), ::IceUtil::constMemFun(&Contained::includeLevel)),
                     exceptions.end());

    if(!exceptions.empty() && !_noIndex)
    {
        start("section", "Exception Index", false);
        start("variablelist");
        for(ExceptionList::const_iterator q = exceptions.begin(); q != exceptions.end(); ++q)
        {
            start("varlistentry");
            start("term");
            O << toString(*q, p);
            end();
            start("listitem");
            string metadata;
            printSummary(*q, (*q)->findMetaData("deprecate", metadata));
            end();
            end();
        }
        end();
        end();
    }

    StructList structs = p->structs();
    structs.erase(remove_if(structs.begin(), structs.end(), ::IceUtil::constMemFun(&Contained::includeLevel)),
                  structs.end());

    if(!structs.empty() && !_noIndex)
    {
        start("section", "Struct Index", false);
        start("variablelist");
        for(StructList::const_iterator q = structs.begin(); q != structs.end(); ++q)
        {
            start("varlistentry");
            start("term");
            O << toString(*q, p);
            end();
            start("listitem");
            string metadata;
            printSummary(*q, (*q)->findMetaData("deprecate", metadata));
            end();
            end();
        }
        end();
        end();
    }

    SequenceList sequences = p->sequences();
    sequences.erase(remove_if(sequences.begin(), sequences.end(), ::IceUtil::constMemFun(&Contained::includeLevel)),
                    sequences.end());

    if(!sequences.empty() && !_noIndex)
    {
        start("section", "Sequence Index", false);
        start("variablelist");
        for(SequenceList::const_iterator q = sequences.begin(); q != sequences.end(); ++q)
        {
            start("varlistentry");
            start("term");
            O << toString(*q, p);
            end();
            start("listitem");
            string metadata;
            printSummary(*q, (*q)->findMetaData("deprecate", metadata));
            end();
            end();
        }
        end();
        end();
    }

    DictionaryList dictionaries = p->dictionaries();
    dictionaries.erase(remove_if(dictionaries.begin(), dictionaries.end(),
                                 ::IceUtil::constMemFun(&Contained::includeLevel)),
                       dictionaries.end());

    if(!dictionaries.empty() && !_noIndex)
    {
        start("section", "Dictionary Index", false);
        start("variablelist");
        for(DictionaryList::const_iterator q = dictionaries.begin(); q != dictionaries.end(); ++q)
        {
            start("varlistentry");
            start("term");
            O << toString(*q, p);
            end();
            start("listitem");
            string metadata;
            printSummary(*q, (*q)->findMetaData("deprecate", metadata));
            end();
            end();
        }
        end();
        end();
    }

    EnumList enums = p->enums();
    enums.erase(remove_if(enums.begin(), enums.end(), ::IceUtil::constMemFun(&Contained::includeLevel)),
                enums.end());

    if(!enums.empty() && !_noIndex)
    {
        start("section", "Enum Index", false);
        start("variablelist");
        for(EnumList::const_iterator q = enums.begin(); q != enums.end(); ++q)
        {
            start("varlistentry");
            start("term");
            O << toString(*q, p);
            end();
            start("listitem");
            string metadata;
            printSummary(*q, (*q)->findMetaData("deprecate", metadata));
            end();
            end();
        }
        end();
        end();
    }

    end();

    {
        for(SequenceList::const_iterator q = sequences.begin(); q != sequences.end(); ++q)
        {
            start("section id=" + containedToId(*q), (*q)->name());
            O.zeroIndent();
            O << nl << "<synopsis>";
            printMetaData(*q);
            if((*q)->isLocal())
            {
                O << "local ";
            }
            TypePtr type = (*q)->type();
            O << "sequence&lt;" << toString(type, p) << "&gt; <type>" << (*q)->name() << "</type>;</synopsis>";
            O.restoreIndent();

            string metadata, deprecateReason;
            if((*q)->findMetaData("deprecate", metadata))
            {
                deprecateReason = "This type has been deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    deprecateReason = metadata.substr(10);
                }
            }

            printComment(*q, deprecateReason);
            end();
        }
    }
    
    {
        for(DictionaryList::const_iterator q = dictionaries.begin(); q != dictionaries.end(); ++q)
        {
            start("section id=" + containedToId(*q), (*q)->name());
            O.zeroIndent();
            O << nl << "<synopsis>";
            printMetaData(*q);
            if((*q)->isLocal())
            {
                O << "local ";
            }
            TypePtr keyType = (*q)->keyType();
            TypePtr valueType = (*q)->valueType();
            O << "dictionary&lt;" << toString(keyType, p) << ", " << toString(valueType, p) << "&gt; <type>"
              << (*q)->name() << "</type>;</synopsis>";
            O.restoreIndent();

            string metadata, deprecateReason;
            if((*q)->findMetaData("deprecate", metadata))
            {
                deprecateReason = "This type has been deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    deprecateReason = metadata.substr(10);
                }
            }

            printComment(*q, deprecateReason);
            end();
        }
    }
}

bool
Slice::Gen::visitClassDefStart(const ClassDefPtr& p)
{
    start(_chapter + " id=" + containedToId(p), p->scoped().substr(2));

    start("section", "Overview", false);

    string metadata, deprecateReason;
    bool deprecatedClass = p->findMetaData("deprecate", metadata);
    if(deprecatedClass)
    {
        deprecateReason = "This type has been deprecated.";
        if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
        {
            deprecateReason = metadata.substr(10);
        }
    }

    O.zeroIndent();
    O << nl << "<synopsis>";
    printMetaData(p);
    if(p->isLocal())
    {
        O << "local ";
    }
    if(p->isInterface())
    {
        O << "interface";
    }
    else
    {
        O << "class";
    }
    O << " <classname>" << p->name() << "</classname>";
    ClassList bases = p->bases();
    if(!bases.empty() && !bases.front()->isInterface())
    {
        O.inc();
        O << nl << "extends ";
        O.inc();
        O << nl << toString(bases.front(), p);
        bases.pop_front();
        O.dec();
        O.dec();
    }
    if(!bases.empty())
    {
        O.inc();
        if(p->isInterface())
        {
            O << nl << "extends ";
        }
        else
        {
            O << nl << "implements ";
        }
        O.inc();
        ClassList::const_iterator q = bases.begin();
        while(q != bases.end())
        {
            O << nl << toString(*q, p);
            if(++q != bases.end())
            {
                O << ",";
            }
        }
        O.dec();
        O.dec();
    }
    O << "</synopsis>";
    O.restoreIndent();
    printComment(p, deprecateReason);

    OperationList operations = p->operations();
    if(!operations.empty() && !_noIndex)
    {
        start("section", "Operation Index", false);
        start("variablelist");
        for(OperationList::const_iterator q = operations.begin(); q != operations.end(); ++q)
        {
            start("varlistentry");
            start("term");
            O << toString(*q, p);
            end();
            start("listitem");
            printSummary(*q, deprecatedClass || (*q)->findMetaData("deprecate", metadata));
            end();
            end();
        }
        end();
        end();
    }

    DataMemberList dataMembers = p->dataMembers();
    if(!dataMembers.empty() && !_noIndex)
    {
        start("section", "Data Member Index", false);
        start("variablelist");
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("varlistentry");
            start("term");
            O << toString(*q, p);
            end();
            start("listitem");
            printSummary(*q, deprecatedClass || (*q)->findMetaData("deprecate", metadata));
            end();
            end();
        }
        end();
        end();
    }

    end();

    {
        for(OperationList::const_iterator q = operations.begin(); q != operations.end(); ++q)
        {
            start("section id=" + containedToId(*q), (*q)->name());
            O.zeroIndent();
            O << nl << "<synopsis>";
            printMetaData(*q);
            TypePtr returnType = (*q)->returnType();
            O << (returnType ? toString(returnType, p) : string("<type>void</type>")) << " <function>" << (*q)->name()
              << "</function>(";
            O.inc();
            ParamDeclList paramList = (*q)->parameters();
            ParamDeclList::const_iterator r = paramList.begin();
            while(r != paramList.end())
            {
                if((*r)->isOutParam())
                {
                    O << "out ";
                }
                O << toString((*r)->type(), *q) << " <parameter>";
                O << (*r)->name() << "</parameter>";
                if(++r != paramList.end())
                {
                    O << ',';
                    O << nl;
                }
            }
            O << ')';
            O.dec();
            ExceptionList throws = (*q)->throws();
            if(!throws.empty())
            {
                O.inc();
                O << nl << "throws";
                O.inc();
                ExceptionList::const_iterator t = throws.begin();
                while(t != throws.end())
                {
                    O << nl << toString(*t, p);
                    if(++t != throws.end())
                    {
                        O << ',';
                    }
                }
                O.dec();
                O.dec();
            }
            O << ";</synopsis>";
            O.restoreIndent();

            string reason;
            metadata.clear();
            if(deprecatedClass || (*q)->findMetaData("deprecate", metadata))
            {
                reason = "This operation has been deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    reason = metadata.substr(10);
                }
            }

            printComment(*q, reason);
            end();
        }
    }

    {
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("section id=" + containedToId(*q), (*q)->name());
            O.zeroIndent();
            O << nl << "<synopsis>";
            printMetaData(*q);
            TypePtr type = (*q)->type();
            O << toString(type, p) << " <structfield>" << (*q)->name() << "</structfield>;</synopsis>";
            O.restoreIndent();

            string reason;
            metadata.clear();
            if(deprecatedClass || (*q)->findMetaData("deprecate", metadata))
            {
                reason = "This member has been deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    reason = metadata.substr(10);
                }
            }

            printComment(*q, reason);
            end();
        }
    }
        
    end();

    return true;
}

bool
Slice::Gen::visitExceptionStart(const ExceptionPtr& p)
{
    start(_chapter + " id=" + containedToId(p), p->scoped().substr(2));

    start("section", "Overview", false);

    string metadata, deprecateReason;
    bool deprecatedException = p->findMetaData("deprecate", metadata);
    if(deprecatedException)
    {
        deprecateReason = "This type has been deprecated.";
        if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
        {
            deprecateReason = metadata.substr(10);
        }
    }

    O.zeroIndent();
    O << nl << "<synopsis>";
    printMetaData(p);
    if(p->isLocal())
    {
        O << "local ";
    }
    O << "exception <classname>" << p->name() << "</classname>";
    ExceptionPtr base = p->base();
    if(base)
    {
        O.inc();
        O << nl << "extends ";
        O.inc();
        O << nl << toString(base, p);
        O.dec();
        O.dec();
    }
    O << "</synopsis>";
    O.restoreIndent();
    printComment(p, deprecateReason);

    DataMemberList dataMembers = p->dataMembers();
    if(!dataMembers.empty() && !_noIndex)
    {
        start("section", "Data Member Index", false);
        start("variablelist");
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("varlistentry");
            start("term");
            O << toString(*q, p);
            end();
            start("listitem");
            printSummary(*q, deprecatedException || (*q)->findMetaData("deprecate", metadata));
            end();
            end();
        }
        end();
        end();
    }

    end();

    {
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("section id=" + containedToId(*q), (*q)->name());
            O.zeroIndent();
            O << nl << "<synopsis>";
            printMetaData(*q);
            TypePtr type = (*q)->type();
            O << toString(type, p) << " <structfield>" << (*q)->name() << "</structfield>;</synopsis>";
            O.restoreIndent();

            string reason;
            metadata.clear();
            if(deprecatedException || (*q)->findMetaData("deprecate", metadata))
            {
                reason = "This member has been deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    reason = metadata.substr(10);
                }
            }

            printComment(*q, reason);
            end();
        }
    }
        
    end();

    return true;
}

bool
Slice::Gen::visitStructStart(const StructPtr& p)
{
    start(_chapter + " id=" + containedToId(p), p->scoped().substr(2));

    start("section", "Overview", false);

    string metadata, deprecateReason;
    bool deprecatedStruct = p->findMetaData("deprecate", metadata);
    if(deprecatedStruct)
    {
        deprecateReason = "This type has been deprecated.";
        if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
        {
            deprecateReason = metadata.substr(10);
        }
    }

    O.zeroIndent();
    O << nl << "<synopsis>";
    printMetaData(p);
    if(p->isLocal())
    {
        O << "local ";
    }
    O << "struct <structname>" << p->name() << "</structname>";
    O << "</synopsis>";
    O.restoreIndent();
    printComment(p, deprecateReason);

    DataMemberList dataMembers = p->dataMembers();
    if(!dataMembers.empty() && !_noIndex)
    {
        start("section", "Data Member Index", false);
        start("variablelist");
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("varlistentry");
            start("term");
            O << toString(*q, p);
            end();
            start("listitem");
            printSummary(*q, deprecatedStruct || (*q)->findMetaData("deprecate", metadata));
            end();
            end();
        }
        end();
        end();
    }

    end();

    {
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("section id=" + containedToId(*q), (*q)->name());
            O.zeroIndent();
            O << nl << "<synopsis>";
            printMetaData(*q);
            TypePtr type = (*q)->type();
            O << toString(type, p) << " <structfield>" << (*q)->name() << "</structfield>;</synopsis>";
            O.restoreIndent();

            string reason;
            metadata.clear();
            if(deprecatedStruct || (*q)->findMetaData("deprecate", metadata))
            {
                reason = "This member has been deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    reason = metadata.substr(10);
                }
            }

            printComment(*q, reason);
            end();
        }
    }
        
    end();

    return true;
}

void
Slice::Gen::visitEnum(const EnumPtr& p)
{
    start(_chapter + " id=" + containedToId(p), p->scoped().substr(2));
    start("section", "Overview", false);

    string metadata, deprecateReason;
    bool deprecatedEnum = p->findMetaData("deprecate", metadata);
    if(deprecatedEnum)
    {
        deprecateReason = "This type has been deprecated.";
        if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
        {
            deprecateReason = metadata.substr(10);
        }
    }

    O.zeroIndent();
    O << nl << "<synopsis>";
    printMetaData(p);
    if(p->isLocal())
    {
        O << "local ";
    }
    O << "enum <type>" << p->name() << "</type>";
    O << "</synopsis>";
    O.restoreIndent();
    printComment(p, deprecateReason);

    EnumeratorList enumerators = p->getEnumerators();
    if(!enumerators.empty() && !_noIndex)
    {
        start("section", "Enumerator Index", false);
        start("variablelist");
        for(EnumeratorList::const_iterator q = enumerators.begin(); q != enumerators.end(); ++q)
        {
            start("varlistentry");
            start("term");
            O << toString(*q, p->container());
            end();
            start("listitem");
            printSummary(*q, deprecatedEnum || (*q)->findMetaData("deprecate", metadata));
            end();
            end();
        }
        end();
        end();
    }

    end();

    {
        for(EnumeratorList::const_iterator q = enumerators.begin(); q != enumerators.end(); ++q)
        {
            start("section id=" + containedToId(*q), (*q)->name());
            O.zeroIndent();
            O << nl << "<synopsis>";
            printMetaData(*q);
            O << "<constant>" << (*q)->name() << "</constant></synopsis>";
            O.restoreIndent();

            //
            // Enumerators do not support metadata.
            //
            string reason;
            if(deprecatedEnum)
            {
                reason = "This enumerator has been deprecated.";
            }

            printComment(*q, reason);
            end();
        }
    }
        
    end();
}

void
Slice::Gen::visitConst(const ConstPtr& p)
{
    // TODO: Deal with constant definition here
}

void
Slice::Gen::printHeader()
{
    static const char* header =
"<!--\n"
" **********************************************************************\n"
"\n"
" Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.\n"
"\n"
" This copy of Ice is licensed to you under the terms described in the\n"
" ICE_LICENSE file included in this distribution.\n"
"\n"
" **********************************************************************\n"
"-->";
    
    O.zeroIndent();
    O << header;
    O << "\n<!-- Ice version " << ICE_STRING_VERSION << " -->";
    O.restoreIndent();
}

string
Slice::Gen::getComment(const ContainedPtr& contained, const ContainerPtr& container, bool summary)
{
    string s = contained->comment();
    string comment;
    for(unsigned int i = 0; i < s.size(); ++i)
    {
        if(s[i] == '\\' && i + 1 < s.size() && s[i + 1] == '[')
        {
            comment += '[';
            ++i;
        }
        else if(s[i] == '[')
        {
            string literal;
            for(++i; i < s.size(); ++i)
            {
                if(s[i] == ']')
                {
                    break;
                }
                
                literal += s[i];
            }
            comment += toString(literal, container);
        }
        else if(summary && s[i] == '.' && (i + 1 >= s.size() || isspace(s[i + 1])))
        {
            comment += '.';
            break;
        }
        else
        {
            comment += s[i];
        }

    }
    return comment;
}

StringList
Slice::Gen::getTagged(const string& tag, string& comment)
{
    StringList result;
    string::size_type begin = 0;
    while(begin < comment.size())
    {
        begin = comment.find("@" + tag, begin);
        if(begin == string::npos)
        {
            return result;
        }
        
        string::size_type pos1 = comment.find_first_not_of(" \t\r\n", begin + tag.size() + 1);
        if(pos1 == string::npos)
        {
            comment.erase(begin);
            return result;
        }
        
        string::size_type pos2 = comment.find('@', pos1);
        string line = comment.substr(pos1, pos2 - pos1);
        comment.erase(begin, pos2 - 1 - begin);

        string::size_type pos3 = line.find_last_not_of(" \t\r\n");
        if(pos3 != string::npos)
        {
            line.erase(pos3 + 1);
        }
        result.push_back(line);
    }

    return result;
}

void
Slice::Gen::printMetaData(const ContainedPtr& p)
{
    list<string> metaData = p->getMetaData();

    if(!metaData.empty())
    {
        O << "[";
        list<string>::const_iterator q = metaData.begin();
        while(q != metaData.end())
        {
            O << " \"" << *q << "\"";
            if(++q != metaData.end())
            {
                O << ",";
            }
        }
        O << " ]" << nl;
    }
}

void
Slice::Gen::printComment(const ContainedPtr& p, const string& deprecateReason)
{
    ContainerPtr container = ContainerPtr::dynamicCast(p);
    if(!container)
    {
        container = p->container();
    }

    string comment = getComment(p, container, false);
    StringList par = getTagged("param", comment);
    StringList ret = getTagged("return", comment);
    StringList throws = getTagged("throws", comment);
    StringList see = getTagged("see", comment);

    start("para");

    string::size_type pos = comment.find_last_not_of(" \t\r\n");
    if(pos != string::npos)
    {
        comment.erase(pos + 1);
        O.zeroIndent();
        O << nl << comment;
        O.restoreIndent();
    }

    end();

    if(!deprecateReason.empty())
    {
        start("caution");
        start("title");
        O << nl << "Deprecated";
        end();
        start("para");
        O << nl << deprecateReason;
        end();
        end();
    }

    if(!par.empty())
    {
        start("section", "Parameters", false);
        start("variablelist");
        for(StringList::const_iterator q = par.begin(); q != par.end(); ++q)
        {
            string term;
            pos = q->find_first_of(" \t\r\n");
            if(pos != string::npos)
            {
                term = q->substr(0, pos);
            }
            string item;
            pos = q->find_first_not_of(" \t\r\n", pos);
            if(pos != string::npos)
            {
                item = q->substr(pos);
            }
            
            start("varlistentry");
            start("term");
            O << "<parameter>" << term << "</parameter>";
            end();
            start("listitem");
            start("para");
            O << nl << item;
            end();
            end();
            end();
        }
        end();
        end();
    }

    if(!ret.empty())
    {
        start("section", "Return Value", false);
        start("para");
        O << nl << ret.front();
        end();
        end();
    }

    if(!throws.empty())
    {
        start("section", "Exceptions", false);
        start("variablelist");
        for(StringList::const_iterator q = throws.begin(); q != throws.end(); ++q)
        {
            string term;
            pos = q->find_first_of(" \t\r\n");
            if(pos != string::npos)
            {
                term = q->substr(0, pos);
            }
            string item;
            pos = q->find_first_not_of(" \t\r\n", pos);
            if(pos != string::npos)
            {
                item = q->substr(pos);
            }
            
            start("varlistentry");
            start("term");
            O << toString(term, container);
            end();
            start("listitem");
            start("para");
            O << nl << item;
            end();
            end();
            end();
        }
        end();
        end();
    }

    ClassList derivedClasses;
    ClassDefPtr def = ClassDefPtr::dynamicCast(p);
    if(def)
    {
        derivedClasses = p->unit()->findDerivedClasses(def);
    }
    if(!derivedClasses.empty())
    {
        start("section", "Derived Classes and Interfaces", false);
        O << nl << "<para>";
        start("simplelist type=\"inline\"");
        for(ClassList::const_iterator q = derivedClasses.begin(); q != derivedClasses.end(); ++q)
        {
            start("member");
            O << toString(*q, container);
            end();
        }
        end();
        O << "</para>";
        end();
    }

    ExceptionList derivedExceptions;
    ExceptionPtr ex = ExceptionPtr::dynamicCast(p);
    if(ex)
    {
        derivedExceptions = p->unit()->findDerivedExceptions(ex);
        if(!derivedExceptions.empty())
        {
            start("section", "Derived Exceptions", false);
            O << nl << "<para>";
            start("simplelist type=\"inline\"");
            for(ExceptionList::const_iterator q = derivedExceptions.begin(); q != derivedExceptions.end(); ++q)
            {
                start("member");
                O << toString(*q, container);
                end();
            }
            end();
            O << "</para>";
            end();
        }
        ContainedList usedBy;
        usedBy = p->unit()->findUsedBy(ex);
        if(!usedBy.empty())
        {
            start("section", "Used By", false);
            O << nl << "<para>";
            start("simplelist type=\"inline\"");
            for(ContainedList::const_iterator q = usedBy.begin(); q != usedBy.end(); ++q)
            {
                start("member");
                O << toString(*q, container);
                end();
            }
            end();
            O << "</para>";
            end();
        }
    }

    ContainedList usedBy;
    ConstructedPtr constructed;
    if(def)
    {
        constructed = def->declaration();
    }
    else
    {
        constructed = ConstructedPtr::dynamicCast(p);
    }
    if(constructed)
    {
        usedBy = p->unit()->findUsedBy(constructed);
    }
    if(!usedBy.empty())
    {
        //
        // We first accumulate the strings in a list instead of printing
        // each stringified entry in the usedBy list. This is necessary because
        // the usedBy list can contain operations and parameters. But toString()
        // on a parameter returns the string for the parameter's operation, so
        // we can end up printing the same operation name more than once.
        //
        list<string> strings;
        for(ContainedList::const_iterator q = usedBy.begin(); q != usedBy.end(); ++q)
        {
            strings.push_back(toString(*q, container));
        }
        strings.sort();
        strings.unique();

        start("section", "Used By", false);
        O << nl << "<para>";
        start("simplelist type=\"inline\"");
        for(list<string>::const_iterator p = strings.begin(); p != strings.end(); ++p)
        {
            start("member");
            O << *p;
            end();
        }
        end();
        O << "</para>";
        end();
    }

    if(!see.empty())
    {
        start("section", "See Also", false);
        O << nl << "<para>";
        start("simplelist type=\"inline\"");
        for(StringList::const_iterator q = see.begin(); q != see.end(); ++q)
        {
            start("member");
            O << toString(*q, container);
            end();
        }
        end();
        O << "</para>";
        end();
    }
}

void
Slice::Gen::printSummary(const ContainedPtr& p, bool deprecated)
{
    ContainerPtr container = ContainerPtr::dynamicCast(p);
    if(!container)
    {
        container = p->container();
    }

    string summary = getComment(p, container, true);
    start("para");
    O.zeroIndent();
    O << nl << summary;
    if(deprecated)
    {
        O << nl << "<emphasis>Deprecated.</emphasis>";
    }
    O.restoreIndent();
    end();
}

void
Slice::Gen::start(const std::string& element)
{
    O << se(element);
}

void
Slice::Gen::start(const std::string& element, const std::string& title, bool asLiteral)
{
    O << se(element);
    static const string titleElement("title");
    O << se(titleElement);
    if(asLiteral)
    {
        O << "<literal>";
    }
    O << title;
    if(asLiteral)
    {
        O << "</literal>";
    }
    end();
}

void
Slice::Gen::end()
{
    O << ee;
}

string
Slice::Gen::containedToId(const ContainedPtr& contained)
{
    assert(contained);

    string scoped = contained->scoped();
    if(scoped[0] == ':')
    {
        scoped.erase(0, 2);
    }

    string id;
    id.reserve(scoped.size());

    for(unsigned int i = 0; i < scoped.size(); ++i)
    {
        if(scoped[i] == ':')
        {
            id += '.';
            ++i;
        }
        else
        {
            id += scoped[i];
        }
    }

    //
    // TODO: At present, docbook tools limit link names (NAMELEN) to
    // 44 characters.
    //
    if(id.size() > 44)
    {
        id.erase(0, id.size() - 44);
        assert(id.size() == 44);
    }

    //
    // A link name cannot start with a period.
    //
    if(id[0] == '.')
    {
        id.erase(0, 1);
    }

    return '"' + id + '"';
}

string
Slice::Gen::getScopedMinimized(const ContainedPtr& contained, const ContainerPtr& container)
{
    string s = contained->scoped();
    ContainerPtr p = container;
    ContainedPtr q = ContainedPtr::dynamicCast(p);

    if(!q) // Container is the global module
    {
        return s.substr(2);
    }
    
    do
    {
        string s2 = q->scoped();
        s2 += "::";

        if(s.find(s2) == 0)
        {
            return s.substr(s2.size());
        }

        p = q->container();
        q = ContainedPtr::dynamicCast(p);
    }
    while(q);

    return s;
}

string
Slice::Gen::toString(const SyntaxTreeBasePtr& p, const ContainerPtr& container, bool withLink)
{
    string tag;
    string linkend;
    string s;

    static const char* builtinTable[] =
    {
        "byte",
        "bool",
        "short",
        "int",
        "long",
        "float",
        "double",
        "string",
        "Object",
        "Object*",
        "LocalObject"
    };

    BuiltinPtr builtin = BuiltinPtr::dynamicCast(p);
    if(builtin)
    {
        s = builtinTable[builtin->kind()];
        tag = "type";
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(p);
    if(proxy)
    {
        if(withLink && proxy->_class()->includeLevel() == 0)
        {
            linkend = containedToId(proxy->_class());
        }
        s = getScopedMinimized(proxy->_class(), container);
        s += "*";
        tag = "classname";
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(p);
    if(cl)
    {
        //
        // We must generate the id from the definition, not from the
        // declaration, provided that a definition is available.
        //
        ContainedPtr definition = cl->definition();
        if(withLink && definition && definition->includeLevel() == 0)
        {
            linkend = containedToId(definition);
        }
        s = getScopedMinimized(cl, container);
        tag = "classname";
    }

    ExceptionPtr ex = ExceptionPtr::dynamicCast(p);
    if(ex)
    {
        if(withLink && ex->includeLevel() == 0)
        {
            linkend = containedToId(ex);
        }
        s = getScopedMinimized(ex, container);
        tag = "classname";
    }

    StructPtr st = StructPtr::dynamicCast(p);
    if(st)
    {
        if(withLink && st->includeLevel() == 0)
        {
            linkend = containedToId(st);
        }
        s = getScopedMinimized(st, container);
        tag = "structname";
    }

    EnumeratorPtr en = EnumeratorPtr::dynamicCast(p);
    if(en)
    {
        if(withLink && en->includeLevel() == 0)
        {
            linkend = containedToId(en);
        }
        s = getScopedMinimized(en, container);
        tag = "constant";
    }

    OperationPtr op = OperationPtr::dynamicCast(p);
    if(op)
    {
        if(withLink && op->includeLevel() == 0)
        {
            linkend = containedToId(op);
        }
        s = getScopedMinimized(op, container);
        tag = "function";
    }

    ParamDeclPtr pd = ParamDeclPtr::dynamicCast(p);
    if(pd)
    {
        op = OperationPtr::dynamicCast(pd->container());
        assert(op);
        if(withLink && pd->includeLevel() == 0)
        {
            linkend = containedToId(op);
        }
        s = getScopedMinimized(op, container);
        tag = "function";
    }

    if(s.empty())
    {
        ContainedPtr contained = ContainedPtr::dynamicCast(p);
        assert(contained);
        if(withLink && contained->includeLevel() == 0)
        {
            linkend = containedToId(contained);
        }
        s = getScopedMinimized(contained, container);
        tag = "type";
    }

    if(linkend.empty())
    {
        return "<" + tag + ">" + s + "</" + tag + ">";
    }
    else
    {
        return "<link linkend=" + linkend + "><" + tag + ">" + s + "</" + tag + "></link>";
    }
}

string
Slice::Gen::toString(const string& str, const ContainerPtr& container, bool withLink)
{
    string s = str;

    TypeList types = container->lookupType(s, false);
    if(!types.empty())
    {
        return toString(types.front(), container, withLink);
    }

    ContainedList contList = container->lookupContained(s, false);
    if(!contList.empty())
    {
        return toString(contList.front(), container, withLink);
    }

    //
    // If we can't find the string, printing it as "literal" is the
    // best we can do.
    //
    return "<literal>" + s + "</literal>";
}
