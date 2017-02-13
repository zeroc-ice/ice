// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/FileUtil.h>
#include <IceUtil/Functional.h>
#include <IceUtil/StringUtil.h>
#include <IceUtil/FileUtil.h>
#include <IceUtil/ConsoleUtil.h>
#include <Slice/FileTracker.h>
#include <Gen.h>
#include <string.h>
#include <sys/types.h>

#ifdef _WIN32
#   include <direct.h>
#else
#   include <unistd.h>
#endif

#include <iterator>

#include <string>

using namespace std;
using namespace Slice;
using namespace IceUtil;
using namespace IceUtilInternal;
using namespace Confluence;

namespace Slice
{

void
generate(const UnitPtr& unit, const string& dir, const string& header, const string& footer,
         const string& indexHeader, const string& indexFooter, const string& imageDir, const string& logoURL,
         const string& searchAction, unsigned indexCount, unsigned warnSummary, const vector<string>& sortOrder)
{
    unit->mergeModules();

    //
    // I don't want the top-level module to be sorted, therefore no
    // p->sort() before or after the p->sortContents().
    //
    unit->sortContents(false);

    GeneratorBase::setOutputDir(dir);
    GeneratorBase::setFooter(footer);
    GeneratorBase::setImageDir(imageDir);
    GeneratorBase::setLogoURL(logoURL);
    GeneratorBase::setSearchAction(searchAction);
    GeneratorBase::setIndexCount(indexCount);
    GeneratorBase::warnSummary(warnSummary);
    GeneratorBase::setSortOrder(sortOrder);

    //
    // The file visitor first runs over the tree and records
    // the names of all files in this documentation set.
    // This information is used later to check whether a referenced
    // symbol is defined in this documentation set, as opposed to
    // being defined in an included file that is not part of this
    // documentation set. If the former, we can generate a link
    // to the symbol; if the latter, we cannot.
    //
    Files files;
    FileVisitor tv(files);
    unit->visit(&tv, false);

    //
    // Generate the start page.
    //
    StartPageVisitor spv(files);
    unit->visit(&spv, false);

    //
    // Generate the table of contents.
    //
    TOCVisitor tocv(files, indexHeader, indexFooter);
    unit->visit(&tocv, false);
    tocv.generate();

    //
    // Generate the individual HTML pages.
    //
    GeneratorBase::setSymbols(tocv.symbols());
    PageVisitor v(files);
    unit->visit(&v, false);
}

}


string Slice::GeneratorBase::_dir = ".";
string Slice::GeneratorBase::_header1;
string Slice::GeneratorBase::_header2;
string Slice::GeneratorBase::_footer;
string Slice::GeneratorBase::_imageDir;
string Slice::GeneratorBase::_logoURL;
string Slice::GeneratorBase::_searchAction;
size_t Slice::GeneratorBase::_indexCount = 0;
size_t Slice::GeneratorBase::_warnSummary = 0;
vector<string> Slice::GeneratorBase::_sortOrder = vector<string>();
const string Slice::GeneratorBase::MODULE_SUFFIX = " Slice API";
const string Slice::GeneratorBase::INDEX_NAME = "Slice API Reference";
ContainedList Slice::GeneratorBase::_symbols;

//
// Set the output directory, creating it if necessary.
//

void
Slice::GeneratorBase::setOutputDir(const string& dir)
{
    if(!dir.empty())
    {
        _dir = dir;
        makeDir(_dir);
    }
}

//
// Get the headers. If "header" is empty, use a default header.
// If a header file is specified, it is expected to include <body>
// and to contain a "TITLE" placeholder line (in column 1, no leading
// or trailing white space). The actual document title is later substituted
// where that TITLE placeholder appears.
//
void
Slice::GeneratorBase::setHeader(const string& header)
{
    getHeaders(header, _header1, _header2);
}

//
// Get the footer. If "footer" is empty, use a default footer.
// The footer is expected to include </body>.
//
void
Slice::GeneratorBase::setFooter(const string& footer)
{
    _footer = getFooter(footer);
}

//
// Set the directory for style sheet images.
//
void
Slice::GeneratorBase::setImageDir(const string& imageDir)
{
    _imageDir = imageDir;
}

//
// Set URL for logo image, if any.
//
void
Slice::GeneratorBase::setLogoURL(const string& logoURL)
{
    _logoURL = logoURL;
}

//
// Set search action, if any.
//
void
Slice::GeneratorBase::setSearchAction(const string& searchAction)
{
    _searchAction = searchAction;
}

//
// Set the threshold at which we start generating sub-indexes.
// If a page has fewer entries than this, we don't generate a
// sub-index. (For example, with "ic" set to 3, we generate
// a sub-index only if, say, a structure has 3 or more members.
//
void
Slice::GeneratorBase::setIndexCount(int ic)
{
    _indexCount = ic;
}

//
// If n > 0, we print a warning if a summary sentence exceeds n characters.
//
void
Slice::GeneratorBase::warnSummary(int n)
{
    _warnSummary = n;
}

void
Slice::GeneratorBase::setSymbols(const ContainedList& symbols)
{
    _symbols = symbols;
}


void
Slice::GeneratorBase::setSortOrder(const vector<string>& sortOrder)
{
    _sortOrder = sortOrder;
}

Slice::GeneratorBase::GeneratorBase(Confluence::ConfluenceOutput& o, const Files& files)
    : _out(o), _files(files)
{
}

Slice::GeneratorBase::~GeneratorBase()
{
}

//
// Open a file for writing in the output directory (the output directory
// is created if necessary) and write the HTML header into the file.
//
void
Slice::GeneratorBase::openDoc(const string& file, const string& title, const string& header, const string& footer)
{
    makeDir(_dir);
    openStream(_dir + "/" + file);

    _out.inc();
    _out.inc();
}

//
// Open an HTML file for writing for the specified construct. The
// path name of the file is relative to the output directory and
// is constructed from the Slice scoped name. Sub-directories are
// created as needed and the header is written to the file.
//
void
Slice::GeneratorBase::openDoc(const ContainedPtr& c)
{
    string path = _dir;
    StringList components = getContainer(c);
    StringList::size_type num = 0;
    for(StringList::const_iterator i = components.begin(); i != components.end(); ++i)
    {

        if (num == components.size()-1 && !components.empty())
        {
            if (path == "." || path == _dir)
            {
                // Prevent files from starting with ".-"
                // Append suffix to avoid name conflicts
                path += "/" + *i + MODULE_SUFFIX;
            }
            else
            {
                // Prepend filenames with directory names and hyphen to avoid conflicts in confluence
                path += "-" + *i;
            }
        }
        else
        {
            if (num == 0) {
                path += "/" + *i;
            } else {
                path += "-" + *i;
            }
        }
        ++num;

        // Don't make last directory, instead prepend name
        if(num < components.size() - 1)
        {
            makeDir(path);
        }
    }
    path += ".conf";

    openStream(path);
    _out.inc();
    _out.inc();
}

//
// Close an open HTML file after writing the footer.
//
void
Slice::GeneratorBase::closeDoc(const string& footer)
{
    _out.dec();
    _out.dec();
    _out << nl << (!footer.empty() ? footer : _footer);
    _out << nl;
}

void
Slice::GeneratorBase::start(const std::string& element, const std::string& classes)
{
    string s = element;
    if(!classes.empty())
    {
        s += " class=\"" + classes + "\"";
    }
    _out << Confluence::se(s);
}

void
Slice::GeneratorBase::end()
{
    _out << Confluence::ee;
}

bool
Slice::GeneratorBase::hasEnding(const std::string& fullString, const std::string& ending)
{
    if (fullString.length() > ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

std::string
Slice::GeneratorBase::getImageMarkup(const string& url, const string& title)
{
    return _out.getImageMarkup(url, title);
}

string
Slice::GeneratorBase::getLinkMarkup(const string& url, const string& text, const string& anchor, const string& tip)
{
    return _out.getLinkMarkup(url, text, anchor, tip);
}

string
Slice::GeneratorBase::getAnchorMarkup(const std::string& anchor, const std::string& text)
{
    return _out.getAnchorMarkup(anchor, text);
}

std::string
Slice::GeneratorBase::getNavMarkup(const std::string& prevLink, const std::string& nextLink)
{
    return _out.getNavMarkup(prevLink, nextLink);
}

bool
Slice::GeneratorBase::compareSymbolNames(const string& n1, const string& n2)
{
    // Uppercase versions for case-insensitive compare
    string u1 = getUpper(n1);
    string u2 = getUpper(n2);

    if (_sortOrder.empty())
    {
        return u1 < u2;
    }
    else
    {
        // Find indices
        vector<string>::const_iterator p1;
        vector<string>::const_iterator p2;

        unsigned int i1 = 0;
        unsigned int i2 = 0;
        for (p1 = _sortOrder.begin();  p1 < _sortOrder.end(); ++p1)
        {
            if (*p1 == n1)
            {
                break;
            }
            ++i1;
        }
        for (p2 = _sortOrder.begin(); p2 < _sortOrder.end(); ++p2)
        {
            if (*p2 == n2)
            {
                break;
            }
            ++i2;
        }

        if (i1 >= _sortOrder.size() && i2 >= _sortOrder.size())
        {
            // Both unmentioned; natural order
            return u1 < u2;
        }
        else if (i1 >= _sortOrder.size())
        {
            // Unmentioned values (n1) sorted to the end
            return false;
        }
        else if (i2 > _sortOrder.size())
        {
            // Unmentioned values (n2) sorted to the end
            return true;
        }
        else
        {
            // Both mentioned, order as specified
            return i1 < i2;
        }
    }
}

string
Slice::GeneratorBase::removeNewlines(string str)
{
    size_t position = str.find("\n");
    while (position != string::npos)
    {
        str.replace(position, 1, " ");
        position = str.find("\n");
    }

    position = str.find("\r");
    while (position != string::npos)
    {
        str.replace(position, 1, " ");
        position = str.find("\r");
    }
    return str;
}

string
Slice::GeneratorBase::trim(string str)
{
    string out = str.erase( str.find_last_not_of(" \n\r\t") + 1);
    out = out.erase(0 , out.find_first_not_of(" \n\r\t"));
    return out;
}


string
Slice::GeneratorBase::getUpper(const std::string& str)
{
    ostringstream oss;
    for (string::const_iterator i = str.begin(); i < str.end(); ++i)
    {
        oss << toupper((unsigned char)*i);
    }
    return oss.str();
}

void
Slice::GeneratorBase::printComment(const ContainedPtr& p, const ContainerPtr& container,
                                   const string& deprecateReason, bool forIndex)
{
#ifndef NDEBUG
    int indent = _out.currIndent();
#endif

    string comment = getComment(p, container, false, forIndex);
    StringList par = getTagged("param", comment);
    StringList ret = getTagged("return", comment);
    StringList throws = getTagged("throws", comment);
    StringList see = getTagged("see", comment);

    // Strip out any "@userImplemented" doc tags that remain
    isTagged("userImplemented", comment);

    string::size_type pos = comment.find_last_not_of(" \t\r\n");
    if(pos != string::npos)
    {
        comment.erase(pos + 1);
        _out.zeroIndent();

        _out << comment;

        _out.restoreIndent();
        _out << "\n";
    }

    if(!deprecateReason.empty())
    {
        start("p", "Deprecated");
        _out << deprecateReason;
        end();
    }

    assert(_out.currIndent() == indent);

    if(!par.empty())
    {
        start("h6");
        _out << "Parameters";
        end();
        start("dl");
        for(StringList::const_iterator q = par.begin(); q != par.end(); ++q)
        {
            string term;
            pos = q->find_first_of(" \t\r\n");
            if(pos != string::npos)
            {
                term = removeNewlines(q->substr(0, pos));
            }
            string item;
            pos = q->find_first_not_of(" \t\r\n", pos);
            if(pos != string::npos)
            {
                item = removeNewlines(q->substr(pos));
            }

            start("dt", "Symbol");
            start("tt");
            _out << term;
            end();
            end();
            start("dd");
            _out << item;
            end();
        }
        end();
    }

    if(!ret.empty())
    {
        start("h6");
        _out << "Return Value";
        end();
        start("p");
        _out << ret.front();
        end();
    }

    if(!throws.empty())
    {
        start("h6");
        _out << "Exceptions";
        end();
        start("dl");
        for(StringList::const_iterator q = throws.begin(); q != throws.end(); ++q)
        {
            string term;
            pos = q->find_first_of(" \t\r\n");
            if(pos != string::npos)
            {
                term = removeNewlines(q->substr(0, pos));
            }
            string item;
            pos = q->find_first_not_of(" \t\r\n", pos);
            if(pos != string::npos)
            {
                item = removeNewlines(q->substr(pos));
            }

            start("dt", "Symbol");
            _out << toString(toSliceID(term, container->definitionContext()->filename()), container, false, forIndex);
            end();
            start("dd");
            _out << item;
            end();
        }
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
        start("h6");
        _out << "Derived Classes and Interfaces";
        end();
        start("ul");
        for(ClassList::const_iterator q = derivedClasses.begin(); q != derivedClasses.end(); ++q)
        {
            start("li", "Symbol");
            _out << toString(*q, container, false, forIndex);
            end();
        }
        end();
        _out << "\n";
    }

    ExceptionList derivedExceptions;
    ExceptionPtr ex = ExceptionPtr::dynamicCast(p);
    if(ex)
    {
        derivedExceptions = p->unit()->findDerivedExceptions(ex);
        if(!derivedExceptions.empty())
        {
            start("h6");
            _out << "Derived Exceptions";
            end();
            start("ul");
            for(ExceptionList::const_iterator q = derivedExceptions.begin(); q != derivedExceptions.end(); ++q)
            {
                start("li", "Symbol");
                _out << toString(*q, container, false, forIndex);
                end();
            }
            end();
            _out << "\n";
        }

        ContainedList usedBy;
        usedBy = p->unit()->findUsedBy(ex);
        if(!usedBy.empty())
        {
            start("h6");
            _out << "Used By";
            end();
            start("ul");
            //
            // We first extract the symbol names from the used-by list and sort
            // them, otherwise the symbols appear in random order.
            //
            StringList sl;
            for(ContainedList::const_iterator q = usedBy.begin(); q != usedBy.end(); ++q)
            {
                sl.push_back(toString(*q, container, false, forIndex));
            }
            sl.sort();
            for(StringList::const_iterator r = sl.begin(); r != sl.end(); ++r)
            {
                start("li", "Symbol");
                _out << removeNewlines(*r);
                end();
            }
            end();
            _out << "\n";
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
        StringList strings;
        for(ContainedList::const_iterator q = usedBy.begin(); q != usedBy.end(); ++q)
        {
            strings.push_back(toString(*q, container, false, forIndex));
        }
        strings.sort();
        strings.unique();

        start("h6");
        _out << "Used By";
        end();
        start("ul");
        for(list<string>::const_iterator p = strings.begin(); p != strings.end(); ++p)
        {
            start("li", "Symbol");
            _out << *p;
            end();
        }
        end();
        _out << "\n";
    }

    if(!see.empty())
    {
        start("h6");
        _out << "See Also";
        end();
        start("ul");
        for(StringList::const_iterator q = see.begin(); q != see.end(); ++q)
        {
            start("li", "Symbol");
            _out << toString(toSliceID(*q, container->definitionContext()->filename()), container, false, forIndex);
            end();
        }
        end();
        _out << "\n";
    }

    assert(_out.currIndent() == indent);
}

bool
Slice::GeneratorBase::isUserImplemented(const ContainedPtr& p, const ContainerPtr& container)
{
    string comment = getComment(p, container, false, true);
    return isTagged("userImplemented", comment);
}

void
Slice::GeneratorBase::printMetaData(const ContainedPtr& p, bool isUserImplemented)
{
    StringList metaData = p->getMetaData();
    string DEP_MARKER = "deprecate";
    StringList userImplementedOnly;
    userImplementedOnly.push_back("cpp:const");
    userImplementedOnly.push_back("cpp:ice_print");
    userImplementedOnly.push_back("java:serialVersionUID");
    userImplementedOnly.push_back("java:UserException");
    userImplementedOnly.push_back("UserException");

    if(!metaData.empty())
    {
        string outString = "";
        StringList::const_iterator q = metaData.begin();
        while(q != metaData.end())
        {
            // If not deprecated
            if (strncmp(q->c_str(), DEP_MARKER.c_str(), strlen(DEP_MARKER.c_str())))
            {
                // For non-user-implemented items, do not print the restricted metadata
                if (!isUserImplemented)
                {
                    bool filterOutMetaData = false;
                    for(StringList::const_iterator r = userImplementedOnly.begin(); r != userImplementedOnly.end(); ++r)
                    {
                        if ( !strncmp(q->c_str(), r->c_str(), strlen(q->c_str())) )
                        {
                            filterOutMetaData = true;
                            break;
                        }
                    }
                    if (filterOutMetaData)
                    {
                        ++q;
                        continue;
                    }
                }

                string stripped = removeNewlines(*q);
                if (outString != "")
                {
                    outString += ",";
                }
                outString += " \"" + stripped + "\"";
            }
            ++q;
        }
        if (!outString.empty())
        {
            _out << "[ " << outString << " ] ";
        }
    }
}

void
Slice::GeneratorBase::printSummary(const ContainedPtr& p, const ContainerPtr& module, bool deprecated, bool forIndex)
{
    _out << getSummary(p, module, deprecated, forIndex);
}

string
Slice::GeneratorBase::getSummary(const ContainedPtr& p, const ContainerPtr& module, bool deprecated, bool forIndex)
{
    ostringstream oss;
    ContainerPtr container = ContainerPtr::dynamicCast(p);
    if(!container)
    {
        container = p->container();
    }

    if(module)
    {
        container = module;
    }

    string summary = getComment(p, container, true, forIndex);
    oss << removeNewlines(summary);

    if(deprecated)
    {
        oss << " _(Deprecated)_ \n";
    }
    return oss.str();
}

static bool
compareContained(const ContainedPtr& p1, const ContainedPtr& p2)
{
    ContainedPtr c1 = ContainedPtr::dynamicCast(p1->container());
    ContainedPtr c2 = ContainedPtr::dynamicCast(p2->container());

    if (c1 && c2) {
        //XXX HACK: since the submodule "Ice::Instumentation" is not seen as a module, add special sorting
        if (c1->name() == "Ice" && p1->name() == "Instrumentation") {
            if (c2->name() == "Instrumentation") {
                return true;
            }
            if (c2->name() == "Ice") {
                return false;
            }
            return true;
        }
        if (c2->name() == "Ice" && p2->name() == "Instrumentation") {
            if (c1->name() == "Instrumentation") {
                return false;
            }
            if (c1->name() == "Ice") {
                return true;
            }
            return false;
        }
        //XXX --- END HACK
    }


    if (!c1 && !c2)
    {
        // Both are top-level containers, compare cnames
        return Slice::GeneratorBase::compareSymbolNames(p1->name(), p2->name());
    }
    else if (!c1)
    {
        if (p1->name() == c2->name())
        {
            // Module index comes before its own contents
            return true;
        }
        // p1 is top-level, compare to p2's parent
        return Slice::GeneratorBase::compareSymbolNames(p1->name(), c2->name());
    }
    else if (!c2)
    {
        if (c1->name() == p2->name())
        {
            // Module index comes before its own contents
            return false;
        }
        // p2 is top-level, compare to p1's parent
        return Slice::GeneratorBase::compareSymbolNames(c1->name(), p2->name());
    }
    if (!c1 || !c2 || c1->name() == c2->name())
    {
        // Same container, compare names
        return Slice::GeneratorBase::getUpper(p1->name()) < Slice::GeneratorBase::getUpper(p2->name());
    }
    else
    {
        if ("Instrumentation" == p1->name() || "Instrumentation" == p2->name()) {
        }


        if (p1->name() == c2->name())
        {
            // Module index comes before its own contents
            return true;
        }
        if (p2->name() == c1->name())
        {
            // Module index comes before its own contents
            return false;
        }
        // Different containers, compare container names
        return Slice::GeneratorBase::compareSymbolNames(c1->name(), c2->name());
    }
}

void
Slice::GeneratorBase::printHeaderFooter(const ContainedPtr& c)
{
    //
    // mes: Disabling nav buttons in header/footer
    //
#if 0
    ContainerPtr container = ContainerPtr::dynamicCast(c);
    string scoped = c->scoped();
    ContainedList::const_iterator prev = _symbols.end();
    ContainedList::const_iterator pos = _symbols.begin();
    while(pos != _symbols.end())
    {
        if((*pos)->scoped() == scoped)
        {
            break;
        }
        prev = pos++;
    }
    ContainedList::const_iterator next = pos == _symbols.end() ? _symbols.end() : ++pos;

    bool isFirst = prev == _symbols.end();
    bool isLast = next == _symbols.end();
    /*bool hasParent = false;
    if(EnumPtr::dynamicCast(c))
    {
        hasParent = true;
    }
    else if(ModulePtr::dynamicCast(c))
    {
         ModulePtr m = ModulePtr::dynamicCast(c);
         if(ModulePtr::dynamicCast(m->container()))
         {
             hasParent = true;
         }
    }
    else if(ContainedPtr::dynamicCast(c))
    {
        hasParent = true;
    }*/

    bool onEnumPage = EnumPtr::dynamicCast(c);

    string prevLink;
    if(isFirst)
    {
        // Nav wraps around to last items
        prev = _symbols.end();
        --prev;
    }
    prevLink = getLinkPath(*prev, container, ModulePtr::dynamicCast(*prev), onEnumPage);
    if(ModulePtr::dynamicCast(c))
    {
        //
        // If we are writing the header/footer for a module page,
        // and the target is up from the current scope,
        // we need to step up an extra level because modules
        // are documented one directory up, at the same level as
        // the module directory.
        //
//        StringList source = getContainer(c);
//        StringList target = getContainer(*prev);
//        if(target.size() < source.size())
//        {
//                prevLink = "../" + prevLink;
//        }
    }

    string nextLink;
    if(isLast)
    {
        // Nav wraps around to first items
        next = _symbols.begin();
    }
    nextLink = getLinkPath(*next, container, ModulePtr::dynamicCast(*next), onEnumPage);
    if(ModulePtr::dynamicCast(c))
    {
        //
        // If we are writing the header/footer for a module page,
        // and the target is up from the current scope,
        // we need to step up an extra level because modules
        // are documented one directory up, at the same level as
        // the module directory.
        //
//        StringList source = getContainer(c);
//        StringList target = getContainer(*next);
//        if(target.size() < source.size())
//        {
//                nextLink = "../" + nextLink;
//        }
    }

    _out << getNavMarkup(prevLink, nextLink);
#endif
}

void
Slice::GeneratorBase::printSearch()
{
    //Do nothing. Form components will not work in confluence markup.
}

void
Slice::GeneratorBase::printLogo(const ContainedPtr& c, const ContainerPtr& container, bool forEnum)
{
    string imageDir = getImageDir();
    if(!imageDir.empty())
    {
        string path = getLinkPath(0, container, false, forEnum);
        if(!path.empty())
        {
            path += "/";
        }
        path += imageDir + "/logo.gif";
        start("table", "LogoTable");
        start("tr");
        start("td");
        if(!_logoURL.empty())
        {
            _out << getLinkMarkup(_logoURL, getImageMarkup(path, "Logo"));
        }
        else
        {
            _out << getImageMarkup(path, "Logo");
        }
        end();
        end();
        end();
    }
}

string
Slice::GeneratorBase::toString(const SyntaxTreeBasePtr& p, const ContainerPtr& container, bool asTarget, bool forIndex,
                               size_t* summarySize, bool shortName)
{
    string anchor;
    string linkpath;
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
        return removeNewlines(s);
    }

    ProxyPtr proxy = ProxyPtr::dynamicCast(p);
    if(proxy)
    {
        if(_files.find(p->definitionContext()->filename()) != _files.end())
        {
            linkpath = getLinkPath(proxy->_class()->definition(), container, forIndex);
        }
        s = getScopedMinimized(proxy->_class(), container, shortName);
    }

    ClassDeclPtr cl = ClassDeclPtr::dynamicCast(p);
    if(cl)
    {
        //
        // We must generate the id from the definition, not from the
        // declaration, provided that a definition is available.
        //
        ContainedPtr definition = cl->definition();
        if(definition && _files.find(p->definitionContext()->filename()) != _files.end())
        {
            linkpath = getLinkPath(definition, container, forIndex);
        }
        s = getScopedMinimized(cl, container, shortName);
    }

    ExceptionPtr ex = ExceptionPtr::dynamicCast(p);
    if(ex)
    {
        if(_files.find(p->definitionContext()->filename()) != _files.end())
        {
            linkpath = getLinkPath(ex, container, forIndex);
        }
        s = getScopedMinimized(ex, container, shortName);
    }

    StructPtr st = StructPtr::dynamicCast(p);
    if(st)
    {
        if(_files.find(p->definitionContext()->filename()) != _files.end())
        {
            linkpath = getLinkPath(st, container, forIndex);
        }
        s = getScopedMinimized(st, container, shortName);
    }

    EnumeratorPtr en = EnumeratorPtr::dynamicCast(p);
    if(en)
    {
        if(_files.find(p->definitionContext()->filename()) != _files.end())
        {
            anchor = getAnchor(en);
            linkpath = getLinkPath(en, container, forIndex);
        }
        s = getScopedMinimized(en, container, shortName);
    }

    OperationPtr op = OperationPtr::dynamicCast(p);
    if(op)
    {
        if(_files.find(p->definitionContext()->filename()) != _files.end())
        {
            anchor = getAnchor(op);
            linkpath = getLinkPath(op, container, forIndex);
        }
        s = getScopedMinimized(op, container, shortName);
    }

    ParamDeclPtr pd = ParamDeclPtr::dynamicCast(p);
    if(pd)
    {
        op = OperationPtr::dynamicCast(pd->container());
        assert(op);
        if(_files.find(p->definitionContext()->filename()) != _files.end())
        {
            anchor = getAnchor(op);
            linkpath = getLinkPath(op, container, forIndex);
        }
        s = getScopedMinimized(op, container, shortName);
    }

    if(s.empty())
    {
        ContainedPtr contained = ContainedPtr::dynamicCast(p);
        assert(contained);
        if(_files.find(p->definitionContext()->filename()) != _files.end())
        {
            if(!(EnumPtr::dynamicCast(p) || ModulePtr::dynamicCast(p) || ClassDeclPtr::dynamicCast(p)))
            {
                anchor = getAnchor(contained);
            }

            linkpath = getLinkPath(contained, container, forIndex);
        }
        s = getScopedMinimized(contained, container, shortName);
    }

    if(summarySize)
    {
        *summarySize = s.size();
    }

    if(linkpath.empty() && anchor.empty())
    {
        if(ProxyPtr::dynamicCast(p))
        {
            s += '*';
            if(summarySize)
            {
                ++(*summarySize);
            }
        }
        return removeNewlines(s);
    }

    string ret = "";
    if (asTarget)
    {
        ret += getAnchorMarkup(anchor, s);
    }
    else
    {
        ret += getLinkMarkup(linkpath, s, anchor);
    }

    if(ProxyPtr::dynamicCast(p))
    {
        ret += '*';
    }
    return removeNewlines(ret);
}

string
Slice::GeneratorBase::toString(const string& str, const ContainerPtr& container, bool asTarget, bool forIndex,
                               size_t* summarySize)
{

    TypeList types = container->lookupType(str, false);
    if(!types.empty())
    {
        return removeNewlines(toString(types.front(), container, asTarget, forIndex, summarySize));
    }

    ContainedList contList = container->lookupContained(str, false);
    if(!contList.empty())
    {
        return removeNewlines(toString(contList.front(), container, asTarget, forIndex, summarySize));
    }

    //
    // If we can't find the string, printing it in typewriter
    // font is the best we can do.
    //
    return "{{" + str + "}}";
}

string
Slice::GeneratorBase::getComment(const ContainedPtr& contained, const ContainerPtr& container,
                                 bool summary, bool forIndex)
{
    size_t summarySize = 0;
    string s = contained->comment();
    string comment;
    for(unsigned int i = 0; i < s.size(); ++i)
    {
        //
        // TODO: Remove old-style link processing once we no longer support the [ident] syntax for links.
        //
        if(s[i] == '\\' && i + 1 < s.size() && s[i + 1] == '[')
        {
            comment += '[';
            ++summarySize;
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
            size_t sz = 0;
            comment += Confluence::ConfluenceOutput::TEMP_ESCAPER_START;
            comment += toString(literal, container, false, forIndex, summary ? &sz : 0);
            comment += Confluence::ConfluenceOutput::TEMP_ESCAPER_END;
            summarySize += sz;

            //
            // TODO: Remove this warning once we no longer support the old javadoc syntax.
            //
            string fileName = contained->file();
            if(_warnOldCommentFiles.find(fileName) == _warnOldCommentFiles.end())
            {
                _warnOldCommentFiles.insert(fileName);
                consoleErr << fileName << ": warning: file contains old-style javadoc link syntax: `[" << literal
                           << "]'" << endl;
            }
        }
        else if(s[i] == '{')
        {
            static const string atLink = "{@link";
            string::size_type pos = s.find(atLink, i);

            comment += Confluence::ConfluenceOutput::TEMP_ESCAPER_START;
            if(pos != i)
            {
                comment += '{';
                ++summarySize;
                continue;
            }
            string::size_type endpos = s.find('}', pos);
            if(endpos == string::npos)
            {
                continue;
            }
            string literal = s.substr(pos + atLink.size(), endpos - pos - atLink.size());
            size_t sz = 0;
            comment += toString(toSliceID(literal, contained->file()), container, false, forIndex, summary ? &sz : 0);
            comment += Confluence::ConfluenceOutput::TEMP_ESCAPER_END;
            summarySize += sz;
            i = static_cast<unsigned int>(endpos);
        }
        else if(summary && s[i] == '.' && (i + 1 >= s.size() || isspace(static_cast<unsigned char>(s[i + 1]))))
        {
            comment += '.';
            ++summarySize;
            break;
        }
        else
        {
            comment += s[i];
            ++summarySize;
        }
    }

    if(summary && _warnSummary && summarySize > _warnSummary)
    {
        consoleErr << contained->file() << ": warning: summary size (" << summarySize << ") exceeds " << _warnSummary
                   << " characters: `" << comment << "'" << endl;
    }
    return trim(_out.convertCommentHTML(removeNewlines(comment)));
}

string
Slice::GeneratorBase::getAnchor(const SyntaxTreeBasePtr& p)
{
    StringList symbols = getContained(p);
    string anchor;
    for(StringList::const_iterator i = symbols.begin(); i != symbols.end(); ++i)
    {
        if(i != symbols.begin())
        {
            anchor += "::";
        }
        anchor += *i;
    }

    return anchor;
}

string
Slice::GeneratorBase::getLinkPath(const SyntaxTreeBasePtr& p, const ContainerPtr& container, bool forIndex, bool forEnum)
{
    ContainerPtr c = container;
    string path = "";

    // Sequences and dictionaries are documented on the page for
    // their enclosing module
    if (DictionaryPtr::dynamicCast(p) || SequencePtr::dynamicCast(p))
    {
        string path = ContainedPtr::dynamicCast(ContainedPtr::dynamicCast(p)->container())->name() + MODULE_SUFFIX;
        return path;
    }

    //
    // If we are in a sub-index, we need to "step up" one level, because the links all
    // point at a section in the same file.
    //
    if(forIndex && ContainedPtr::dynamicCast(container))
    {
        c = ContainedPtr::dynamicCast(c)->container();
    }

    //
    // Find the first component where the two scopes differ.
    //
    bool commonEnclosingScope = false;
    StringList target;
    EnumeratorPtr enumerator = EnumeratorPtr::dynamicCast(p);
    if(enumerator)
    {
        target = toStringList(enumerator->type());
    }
    else
    {
        target = getContainer(p);
    }
    StringList from = getContainer(c);

    string parent;
    if(!from.empty())
    {
        parent = target.front();
    }

    while(!target.empty() && !from.empty() && target.front() == from.front())
    {
        target.pop_front();
        from.pop_front();
        commonEnclosingScope = true;
    }

    if(commonEnclosingScope && target.empty())
    {
        ModulePtr module = ModulePtr::dynamicCast(p);
        if(module)
        {
            target.push_front(module->name());
        }
    }
    else if(!from.empty())
    {
        from.pop_front();
    }

    //
    // For each component in the source path, step up a level.
    //
    while(!from.empty())
    {
        from.pop_front();
    }

    //
    // Now append the scope to the target.
    //
    while(!target.empty())
    {
        if(!path.empty())
        {
            path += "-";
        }
        string name = target.front() == INDEX_NAME ? string("_index") : target.front();

        path += name;

        target.pop_front();
    }

    if((forIndex && path == parent) || (parent.empty() && path.find("-") == string::npos) ||
        DictionaryPtr::dynamicCast(p))
    {
        // Link to parent, add suffix
        path += MODULE_SUFFIX;
    }
    else // if (path != INDEX_NAME && !path.empty() && !parent.empty() && path != parent)
    {
        // Intra-package links need package name, unlike with html dir structure
        EnumeratorPtr enumerator = EnumeratorPtr::dynamicCast(p);
    if(enumerator)
    {
        target = toStringList(enumerator->type());
    }
    else
    {
        target = getContainer(p);
    }
        path = "";
        while (!target.empty()) {
            if (!path.empty()) {
                path += "-";
            }
            path += target.front();
            target.pop_front();
        }
    }

    return path;
}

string
Slice::GeneratorBase::getImageDir()
{
    return _imageDir;
}

string
Slice::GeneratorBase::getLogoURL()
{
    return _logoURL;
}

void
Slice::GeneratorBase::openStream(const string& path)
{
    _out.open(path.c_str());
    if(!_out.isOpen())
    {
        ostringstream os;
        os << "cannot open file `" << path << "': " << strerror(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addFile(path);
}

void
Slice::GeneratorBase::closeStream()
{
    _out.close();
}

string
Slice::GeneratorBase::containedToId(const ContainedPtr& contained, bool asTarget)
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
    // A link name cannot start with a period.
    //
    if(id[0] == '.')
    {
        id.erase(0, 1);
    }

    return '"' + id + '"';
}

StringList
Slice::GeneratorBase::getTagged(const string& tag, string& comment)
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

bool
Slice::GeneratorBase::isTagged(const string& tag, string& comment)
{
    string::size_type begin = comment.find("@" + tag, 0);
    if (begin != string::npos)
    {
        comment.erase(begin, tag.size() + 1);
        return true;
    }
    return false;
}


string
Slice::GeneratorBase::getScopedMinimized(const ContainedPtr& contained, const ContainerPtr& container, bool shortName)
{
    if(shortName)
    {
        return contained->name();
    }

    string s = contained->scoped();

    ContainerPtr p = container;
    ContainedPtr q = ContainedPtr::dynamicCast(p);

    if(!q) // Container is the global module
    {
        return s.substr(2);
    }


//    do
//    {
//        string s2 = q->scoped(); // Containing scope
//        s2 += "::";
//
//        if(s.find(s2) == 0)
//        {
//            if (q->scoped().find("::", 2) != string::npos)
//            {
//                return "MIN3::" + s.substr(s2.size());
//            }
//            return "MIN2::" + s.substr(2);
//        }
//
//        p = q->container();
//        q = ContainedPtr::dynamicCast(p);
//    }
//    while(q);

    string s2 = q->scoped(); // Containing scope
    s2 += "::";

    if(s.find(s2) == 0)
    {
        string after = s.substr(s2.size());
        if (after.find("::") == string::npos)
        {
            return after;
        }
//
        if (q->scoped().find("::", 2) != string::npos)
        {
            // There are at least two components above contained.
            // Locally scoped const/member/operation
            return s.substr(s2.size());
        }
        else if (s.find("::" + q->name()) == 0)
        {
            // Within this module
            return s.substr(s2.size());
        }
//
    }

    p = q->container();
    q = ContainedPtr::dynamicCast(p);

    return s.substr(2); // s
}

StringList
Slice::GeneratorBase::getContained(const SyntaxTreeBasePtr& p)
{
    StringList result;
    if(!p)
    {
        return result;
    }

    SyntaxTreeBasePtr c = p;

    do
    {
        ContainedPtr contained = ContainedPtr::dynamicCast(c);
        assert(contained);
        result.push_front(contained->name());
        c = contained->container();
    }
    while(!ContainerPtr::dynamicCast(c));
    return result;
}

StringList
Slice::GeneratorBase::getContainer(const SyntaxTreeBasePtr& p)
{
    StringList result;

    if(!p)
    {
        return result;
    }

    ContainedPtr contained = ContainedPtr::dynamicCast(p);
    while(contained &&
          !ModulePtr::dynamicCast(contained) &&
          !ExceptionPtr::dynamicCast(contained) &&
          !ClassDefPtr::dynamicCast(contained) &&
          !StructPtr::dynamicCast(contained) &&
          !EnumPtr::dynamicCast(contained))
    {
        contained = ContainedPtr::dynamicCast(contained->container());
    }

    while(contained)
    {
        result.push_front(contained->name());
        contained = ContainedPtr::dynamicCast(contained->container());
    }
    return result;
}

//
// TODO: remove warnOldStyleIdent() function once we no longer support
// old-style javadoc comments ([...] instead of {@link ...} and
// X::Y::Z instead of X.Y#Z).
//
//
void
Slice::GeneratorBase::warnOldStyleIdent(const string& str, const string& fileName)
{
    string newName;

    string::size_type next = 0;
    if(str.size() > 2 && str[0] == ':' && str[1] == ':')
    {
        next = 2;
    }

    int numIdents = 0;
    string::size_type endpos;
    while((endpos = str.find("::", next)) != string::npos)
    {
        if(numIdents != 0)
        {
            newName += ".";
        }
        newName += str.substr(next, endpos - next);
        ++numIdents;
        next = endpos;
        if(next != string::npos)
        {
            next += 2;
        }
    }

    if(numIdents != 0)
    {
        newName += ".";
    }
    newName += str.substr(next);

    if(_warnOldCommentFiles.find(fileName) == _warnOldCommentFiles.end())
    {
        _warnOldCommentFiles.insert(fileName);

        string::size_type pos;
        pos = newName.rfind('.');
        string alternateName;
        string lastName;
        if(pos != string::npos)
        {
            alternateName = newName;
            alternateName[pos] = '#';
            lastName = newName.substr(pos + 1);
        }

        consoleErr << fileName << ": warning: file contains old-style javadoc identifier syntax: `" << str << "'."
                   << " Use `'" << newName << "'";
        if(!alternateName.empty())
        {
             consoleErr << " or `" << alternateName << "' if `" << lastName << "' is a member";
        }
        consoleErr << endl;
    }
}

//
// Convert a string of the form X.Y#Z into X::Y::Z (#Z converts to Z).
// TODO: Remove the filename parameter once we no longer support old-style javadoc comments.
//
string
Slice::GeneratorBase::toSliceID(const string& str, const string& filename)
{

    const string s = IceUtilInternal::trim(str);
    string result;
    string::size_type pos;
    string::size_type next = 0;
    while((pos = s.find_first_of(".#", next)) != string::npos)
    {
        result += s.substr(next, pos - next);
        if(s[pos] != '#' || pos != 0)
        {
            result += "::";
        }
        next = ++pos;
    }
    result += s.substr(next);

    //
    // TODO: Remove the warning once we no longer support the old-style
    // javadoc syntax.
    //
    if(str.find("::") != string::npos)
    {
        warnOldStyleIdent(s, filename);
    }

    return result;
}

StringList
Slice::GeneratorBase::toStringList(const ContainedPtr& c)
{
    string scoped = c->scoped();
    assert(scoped.size() > 2);
    assert(scoped[0] == ':');
    assert(scoped[1] == ':');

    StringList ids;
    string::size_type next = 2;
    string::size_type endpos;
    while((endpos = scoped.find("::", next)) != string::npos)
    {
        ids.push_back(scoped.substr(next, endpos - next));
        next = endpos;
        if(next != string::npos)
        {
            ++next;
            ++next;
        }
    }
    ids.push_back(scoped.substr(next));

    return ids;
}

void
Slice::GeneratorBase::makeDir(const string& dir)
{
    struct stat st;
    if(!IceUtilInternal::stat(dir, &st))
    {
        if(!(st.st_mode & S_IFDIR))
        {
            ostringstream os;
            os << "failed to create package directory `" << dir
               << "': file already exists and is not a directory";
            throw FileException(__FILE__, __LINE__, os.str());
        }
        return;
    }

    if(IceUtilInternal::mkdir(dir, 0777) != 0)
    {
        ostringstream os;
        os << "cannot create directory `" << dir << "': " << strerror(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }
    FileTracker::instance()->addDirectory(dir);
}

string
Slice::GeneratorBase::readFile(const string& file)
{
    std::ifstream in(file.c_str());
    if(!in)
    {
        ostringstream os;
        os << "cannot open file `" << file << "': " << strerror(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }

    ostringstream result;
    string line;
    getline(in, line);
    while(!line.empty())
    {
        result << line << '\n';
        getline(in, line);
    }

    return result.str();
}

void
Slice::GeneratorBase::getHeaders(const string& header, string& h1, string& h2)
{
    // Do nothing. No headers in confluence markup.
}

string
Slice::GeneratorBase::getFooter(const string& footer)
{
    //Confluence markup is all body, no header/footer mechanism or tags that need cleaning up
    return "";
}

void
Slice::GeneratorBase::readFile(const string& file, string& part1, string& part2)
{
    std::ifstream in(file.c_str());
    if(!in)
    {
        ostringstream os;
        os << "cannot open file `" << file << "': " << strerror(errno);
        throw FileException(__FILE__, __LINE__, os.str());
    }

    string line;
    bool foundTitle = false;

    ostringstream p1;
    while(!foundTitle && getline(in, line))
    {
        if(line == "TITLE")
        {
            foundTitle = true;
        }
        else
        {
            p1 << line << '\n';
        }
    }
    part1 = p1.str();

    if(!foundTitle)
    {
        string err = "no TITLE marker in `" + file + "'";
        throw err;
    }

    ostringstream p2;
    p2 << endl;
    while(getline(in, line))
    {
        p2 << line << '\n';
    }
    part2 = p2.str();
}

Slice::StartPageGenerator::StartPageGenerator(const Files& files)
    : GeneratorBase(_out, files)
{
    openDoc(INDEX_NAME, "Slice API Documentation");
}

static string
getModuleName(const string& s)
{
    size_t start = s.find("[");
    size_t end = s.find("|", start);
    if (start != string::npos && end != string::npos)
    {
        start++;
        size_t len = end - start;
        string n = s.substr(start, len);
        return n;
    }
    else
    {
        return s;
    }
}

static bool
compareModulePtrs(const ModulePtr& p1, const ModulePtr& p2)
{
    return Slice::GeneratorBase::compareSymbolNames(p1->name(), p2->name());
}

static bool
compareModules(const StringPair& p1, const StringPair& p2)
{
    return Slice::GeneratorBase::compareSymbolNames(getModuleName(p1.first), getModuleName(p2.first));
}

Slice::StartPageGenerator::~StartPageGenerator()
{
    ::std::sort(_modules.begin(), _modules.end(), compareModules);

    start("h2");
    _out << "Modules";
    end();
    start("dl");
    for(ModuleDescriptions::const_iterator i = _modules.begin(); i != _modules.end(); ++i)
    {
        start("dt", "Symbol");
        _out << removeNewlines(i->first);
        end();
        start("dd");
        _out << removeNewlines(i->second);
        end();
    }
    end();

    start("hr");
    end();

    closeDoc();
}

void
Slice::StartPageGenerator::generate(const ModulePtr& m)
{
    string name = toString(m, 0, false);
    string comment = getComment(m, m, true, true);
    _modules.push_back(make_pair(name, comment));
}

void
Slice::StartPageGenerator::printHeaderFooter()
{
    // Do nothing
}

Slice::FileVisitor::FileVisitor(Files& files) :
    _files(files)
{
}

bool
Slice::FileVisitor::visitUnitStart(const UnitPtr& u)
{
    return true;
}

bool
Slice::FileVisitor::visitModuleStart(const ModulePtr& m)
{
    _files.insert(m->file());
    return true;
}

bool
Slice::FileVisitor::visitExceptionStart(const ExceptionPtr& e)
{
    _files.insert(e->file());
    return false;
}

bool
Slice::FileVisitor::visitClassDefStart(const ClassDefPtr& c)
{
    _files.insert(c->file());
    return false;
}

void
Slice::FileVisitor::visitClassDecl(const ClassDeclPtr& c)
{
    _files.insert(c->file());
}

bool
Slice::FileVisitor::visitStructStart(const StructPtr& s)
{
    _files.insert(s->file());
    return false;
}

void
Slice::FileVisitor::visitSequence(const SequencePtr& s)
{
    _files.insert(s->file());
}

void
Slice::FileVisitor::visitDictionary(const DictionaryPtr& d)
{
    _files.insert(d->file());
}

void
Slice::FileVisitor::visitEnum(const EnumPtr& e)
{
    _files.insert(e->file());
}

Slice::StartPageVisitor::StartPageVisitor(const Files& files) :
    _spg(files)
{
}

bool
Slice::StartPageVisitor::visitUnitStart(const UnitPtr& unit)
{
    return true;
}

bool
Slice::StartPageVisitor::visitModuleStart(const ModulePtr& m)
{
    _spg.generate(m);
    return false;
}

TOCGenerator::TOCGenerator(const Files& files, const string& header, const string& footer)
    : GeneratorBase(_out, files)
{
    _footer = footer;

    start("h1");
    _out << "Slice API Index";
    end();

    //ExpandCollapseButtonTable not added; no Confluence markup for buttons
}

void
TOCGenerator::generate(const ModulePtr& m)
{
    _modules.push_back(m);
}

void
TOCGenerator::writeTOC()
{
    _modules.sort(compareModulePtrs);
    _out.inc();
    for(ModuleList::const_iterator i = _modules.begin(); i != _modules.end(); ++i)
    {
        writeEntry(*i);
    }
    _out.dec();

    _symbols.sort(compareContained);
    _symbols.unique();
}

const ContainedList&
TOCGenerator::symbols() const
{
    return _symbols;
}


void
TOCGenerator::writeEntry(const ContainedPtr& c)
{
    ContainedList cl;

    ModulePtr m = ModulePtr::dynamicCast(c);
    if(m)
    {
        cl = m->contents();
    }

    EnumPtr en = EnumPtr::dynamicCast(c);
    if(en)
    {
        EnumeratorList enumerators = en->enumerators();
        for(EnumeratorList::const_iterator i = enumerators.begin(); i != enumerators.end(); ++i)
        {
            cl.push_back(*i);
        }
    }

    StructPtr s = StructPtr::dynamicCast(c);
    if(s)
    {
        DataMemberList dml = s->dataMembers();
        for(DataMemberList::const_iterator i = dml.begin(); i != dml.end(); ++i)
        {
            cl.push_back(*i);
        }
    }

    ExceptionPtr e = ExceptionPtr::dynamicCast(c);
    if(e)
    {
        DataMemberList dml = e->dataMembers();
        for(DataMemberList::const_iterator i = dml.begin(); i != dml.end(); ++i)
        {
            cl.push_back(*i);
        }
    }

    ClassDefPtr cdef = ClassDefPtr::dynamicCast(c);
    if(!cdef)
    {
        ClassDeclPtr cdec = ClassDeclPtr::dynamicCast(c);
        if(cdec)
        {
            cdef = cdec->definition();
        }
    }

    if(cdef)
    {
        DataMemberList dml = cdef->dataMembers();
        for(DataMemberList::const_iterator i = dml.begin(); i != dml.end(); ++i)
        {
            cl.push_back(*i);
        }
        OperationList ol = cdef->operations();
        for(OperationList::const_iterator j = ol.begin(); j != ol.end(); ++j)
        {
            cl.push_back(*j);
        }
    }

    start("li");
    if(!cl.empty())
    {
        cl.sort();
        cl.unique();

        _out << toString(c, 0, false, true, 0, true);
        start("ul");
        for(ContainedList::const_iterator i = cl.begin(); i != cl.end(); ++i)
        {
            writeEntry(*i);
        }
        end();
    }
    else
    {
        _out << toString(c, 0, false, true, 0, true);
    }
    if(ModulePtr::dynamicCast(c) || ExceptionPtr::dynamicCast(c) || ClassDefPtr::dynamicCast(c) ||
       StructPtr::dynamicCast(c) || EnumPtr::dynamicCast(c))
    {
        _symbols.push_back(c);
    }
    else if(ClassDeclPtr::dynamicCast(c))
    {
        ContainedPtr definition = ClassDeclPtr::dynamicCast(c)->definition();
        if(definition)
        {
            _symbols.push_back(definition);
        }
    }
    end();
}

TOCVisitor::TOCVisitor(const Files& files, const string& header, const string& footer) :
    _tg(files, header, footer)
{
}

bool
TOCVisitor::visitUnitStart(const UnitPtr&)
{
    return true;
}

bool
TOCVisitor::visitModuleStart(const ModulePtr& m)
{
    _tg.generate(m);
    return false;
}

void
TOCVisitor::generate()
{
    _tg.writeTOC();
}

const ContainedList&
TOCVisitor::symbols() const
{
    return _tg.symbols();
}

Slice::ModuleGenerator::ModuleGenerator(Confluence::ConfluenceOutput& o, const Files& files)
    : GeneratorBase(o, files)
{
}

void
Slice::ModuleGenerator::generate(const ModulePtr& m)
{
#ifndef NDEBUG
    int indent = _out.currIndent();
#endif

    openDoc(m);

    printHeaderFooter(m);
    start("hr");
    end();

    start("h1", "Symbol");
    _out << m->scoped().substr(2);
    end();

    string metadata, deprecateReason;
    if(m->findMetaData("deprecate", metadata))
    {
        deprecateReason = "This module is deprecated.";
        if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
        {
            deprecateReason = "_" + trim(metadata.substr(10)) + "_";
        }
    }

    start("h2");
    _out << "Overview";
    end();
    start("h3", "Synopsis");
    printMetaData(m, isUserImplemented(m, m));
    _out << "module " << m->name();
    end();

    printComment(m, m, deprecateReason, true);

    visitContainer(m);

    start("hr");
    end();
    printHeaderFooter(m);

    closeDoc();

    assert(_out.currIndent() == indent);
}

void
Slice::ModuleGenerator::visitContainer(const ContainerPtr& p)
{
#ifndef NDEBUG
    int indent = _out.currIndent();
#endif

    ModuleList modules = p->modules();

    if(!modules.empty())
    {
        start("h2");
        _out << "Module Index";
        end();
        start("dl");
        for(ModuleList::const_iterator q = modules.begin(); q != modules.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, p, false, true);
            end();

            string metadata;
            string summary = trim(getSummary(*q, p, (*q)->findMetaData("deprecate", metadata), false));
            if (!summary.empty())
            {
                start("dd");
                _out << summary;
                end();
            }
            else
            {
                _out << "\n";
            }
        }
        end();
    }

    assert(_out.currIndent() == indent);

    ClassList classesAndInterfaces = p->classes();
    ClassList classes;
    ClassList interfaces;
    remove_copy_if(classesAndInterfaces.begin(), classesAndInterfaces.end(), back_inserter(classes),
                   ::IceUtil::constMemFun(&ClassDef::isInterface));
    remove_copy_if(classesAndInterfaces.begin(), classesAndInterfaces.end(), back_inserter(interfaces),
                   not1(::IceUtil::constMemFun(&ClassDef::isInterface)));

    if(!classes.empty())
    {
        start("h2");
        _out << "Class Index";
        end();
        start("dl");
        for(ClassList::const_iterator q = classes.begin(); q != classes.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, p, false, true);
            end();

            string metadata;
            string summary = trim(getSummary(*q, p, (*q)->findMetaData("deprecate", metadata), true));
            if (!summary.empty())
            {
                start("dd");
                _out << summary;
                end();
            }
            else
            {
                _out << "\n";
            }
        }
        end();
        _out << "\n{ztop}\n";
    }

    assert(_out.currIndent() == indent);

    if(!interfaces.empty())
    {
        start("h2");
        _out << "Interface Index";
        end();
        start("dl");
        for(ClassList::const_iterator q = interfaces.begin(); q != interfaces.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, p, false, true);
            end();

            string metadata;
            string summary = trim(getSummary(*q, p, (*q)->findMetaData("deprecate", metadata), true));
            if (!summary.empty())
            {
                start("dd");
                _out << summary;
                end();
            }
            else
            {
                _out << "\n";
            }
        }
        end();
        _out << "\n{ztop}\n";
    }

    assert(_out.currIndent() == indent);

    ExceptionList exceptions = p->exceptions();

    if(!exceptions.empty())
    {
        start("h2");
        _out << "Exception Index";
        end();
        start("dl");
        for(ExceptionList::const_iterator q = exceptions.begin(); q != exceptions.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, p, false, true);
            end();

            string metadata;
            string summary = trim(getSummary(*q, p, (*q)->findMetaData("deprecate", metadata), true));
            if (!summary.empty())
            {
                start("dd");
                _out << summary;
                end();
            }
            else
            {
                _out << "\n";
            }
        }
        end();
        _out << "\n{ztop}\n";
    }

    assert(_out.currIndent() == indent);

    StructList structs = p->structs();

    if(!structs.empty())
    {
        start("h2");
        _out << "Structure Index";
        end();
        start("dl");
        for(StructList::const_iterator q = structs.begin(); q != structs.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, p, false, true);
            end();

            string metadata;
            string summary = trim(getSummary(*q, p, (*q)->findMetaData("deprecate", metadata), true));
            if (!summary.empty())
            {
                start("dd");
                _out << summary;
                end();
            }
            else
            {
                _out << "\n";
            }
        }
        end();
        _out << "\n{ztop}\n";
    }

    assert(_out.currIndent() == indent);

    SequenceList sequences = p->sequences();

    if(!sequences.empty())
    {
        start("h2");
        _out << "Sequence Index";
        end();
        start("dl");
        for(SequenceList::const_iterator q = sequences.begin(); q != sequences.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, p, false, true);
            end();

            string metadata;
            string summary = trim(getSummary(*q, p, (*q)->findMetaData("deprecate", metadata), true));
            if (!summary.empty())
            {
                start("dd");
                _out << summary;
                end();
            }
            else
            {
                _out << "\n";
            }
        }
        end();
        _out << "\n{ztop}\n";
    }

    assert(_out.currIndent() == indent);

    DictionaryList dictionaries = p->dictionaries();

    if(!dictionaries.empty())
    {
        start("h2");
        _out << "Dictionary Index";
        end();
        start("dl");
        for(DictionaryList::const_iterator q = dictionaries.begin(); q != dictionaries.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, p, false, true);
            end();

            string metadata;
            string summary = trim(getSummary(*q, p, (*q)->findMetaData("deprecate", metadata), true));
            if (!summary.empty())
            {
                start("dd");
                _out << summary;
                end();
            }
            else
            {
                _out << "\n";
            }
        }
        end();
        _out << "\n{ztop}\n";
    }

    assert(_out.currIndent() == indent);

    ConstList consts = p->consts();

    if(!consts.empty())
    {
        start("h2");
        _out << "Constant Index";
        end();
        start("dl");
        for(ConstList::const_iterator q = consts.begin(); q != consts.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, p, false, true);
            end();

            string metadata;
            string summary = trim(getSummary(*q, p, (*q)->findMetaData("deprecate", metadata), true));
            if (!summary.empty())
            {
                start("dd");
                _out << summary;
                end();
            }
            else
            {
                _out << "\n";
            }
        }
        end();
        _out << "\n{ztop}\n";
    }

    assert(_out.currIndent() == indent);

    EnumList enums = p->enums();

    if(!enums.empty())
    {
        start("h2");
        _out << "Enumeration Index";
        end();
        start("dl");
        for(EnumList::const_iterator q = enums.begin(); q != enums.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, p, false, true);
            end();

            string metadata;
            string summary = trim(getSummary(*q, p, (*q)->findMetaData("deprecate", metadata), true));
            if (!summary.empty())
            {
                start("dd");
                _out << summary;
                end();
            }
            else
            {
                _out << "\n";
            }
        }
        end();
        _out << "\n{ztop}\n";
    }

    assert(_out.currIndent() == indent);

    if(!sequences.empty())
    {
        start("h2");
        _out << "Sequences";
        end();
        for(SequenceList::const_iterator q = sequences.begin(); q != sequences.end(); ++q)
        {
            start("h3");
            printMetaData(*q, isUserImplemented(*q, p));
            if((*q)->isLocal())
            {
                _out << "local ";
            }
            else
            {
                _out << " ";
            }
            TypePtr type = (*q)->type();
            _out << "sequence&lt;" << toString(type, p, false, true) << "&gt; " << toString(*q, p);
            end();

            string metadata, deprecateReason;
            if((*q)->findMetaData("deprecate", metadata))
            {
                deprecateReason = "This type is deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    deprecateReason = "_" + trim(metadata.substr(10)) + "_";
                }
            }

            printComment(*q, p, deprecateReason, true);
            _out << "\n";
        }
        _out << "\n{ztop}\n";
    }

    if(!dictionaries.empty())
    {
        start("h2");
        _out << "Dictionaries";
        end();
        for(DictionaryList::const_iterator q = dictionaries.begin(); q != dictionaries.end(); ++q)
        {
            start("h3");
            printMetaData(*q, isUserImplemented(*q, p));
            if((*q)->isLocal())
            {
                _out << "local ";
            }
            else
            {
                _out << " ";
            }
            TypePtr keyType = (*q)->keyType();
            TypePtr valueType = (*q)->valueType();
            _out << "dictionary&lt;" << toString(keyType, p, false, true) << ", "
                 << toString(valueType, p, false, true) << "&gt; " << toString(*q, p);
            end();

            string metadata, deprecateReason;
            if((*q)->findMetaData("deprecate", metadata))
            {
                deprecateReason = "This type is deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    deprecateReason = "_" + trim(metadata.substr(10)) + "_";
                }
            }

            printComment(*q, p, deprecateReason, true);
        }
        _out << "\n{ztop}\n";
    }

    if(!consts.empty())
    {
        start("h2");
        _out << "Constants";
        end();
        for(ConstList::const_iterator q = consts.begin(); q != consts.end(); ++q)
        {
            start("h3");
            _out << "const " << toString((*q)->type(), p, false, true) << " " << toString(*q, p) << " = ";
            if(EnumPtr::dynamicCast((*q)->type()))
            {
                _out << trim(toString((*q)->value(), p, false, true));
            }
            else
            {
                 _out << trim((*q)->literal());
            }
            _out << ";";
            end();
            _out << "\n";

            string metadata, deprecateReason;
            if((*q)->findMetaData("deprecate", metadata))
            {
                deprecateReason = "This type is deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    deprecateReason = "_" + trim(metadata.substr(10)) + "_";
                }
            }
            printComment(*q, p, deprecateReason, true);
        }
        _out << "\n{ztop}\n";
    }
}

Slice::ExceptionGenerator::ExceptionGenerator(Confluence::ConfluenceOutput& o, const Files& files)
    : GeneratorBase(o, files)
{
}

void
Slice::ExceptionGenerator::generate(const ExceptionPtr& e)
{
#ifndef NDEBUG
    int indent = _out.currIndent();
#endif

    openDoc(e);

    printHeaderFooter(e);
    start("hr");
    end();

    start("h1", "Symbol");
    _out << e->scoped().substr(2);
    end();

    string metadata, deprecateReason;
    bool deprecatedException = e->findMetaData("deprecate", metadata);
    if(deprecatedException)
    {
        deprecateReason = "This module is deprecated.";
        if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
        {
            deprecateReason = "_" + trim(metadata.substr(10)) + "_";
        }
    }

    start("h2");
    _out << "Overview";
    end();

    start("h3", "Synopsis");
    printMetaData(e, isUserImplemented(e, e));
    if(e->isLocal())
    {
        _out << "local ";
    }
    _out << "exception " << e->name();
    ExceptionPtr base = e->base();
    if(base)
    {
        _out.inc();
        _out << " extends ";
        _out.inc();
        _out << toString(base, e, false);
        _out.dec();
        _out.dec();
    }
    end();

    printComment(e, e, deprecateReason);

    DataMemberList dataMembers = e->dataMembers();

    if(_indexCount > 0 && dataMembers.size() >= _indexCount)
    {
        start("h2");
        _out << "Data Member Index";
        end();
        start("dl");
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, e, false);
            end();

            string metadata;
            string summary = trim(getSummary(*q, e, (*q)->findMetaData("deprecate", metadata), false));
            if (!summary.empty())
            {
                start("dd");
                _out << summary;
                end();
            }
            else
            {
                _out << "\n";
            }
        }
        end();
        _out << "\n{ztop}\n";
    }

    if(!dataMembers.empty())
    {
        start("h2");
        _out << "Data Members";
        end();
        start("p");
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("h3");
            printMetaData(*q, isUserImplemented(*q, e));
            TypePtr type = (*q)->type();
            _out << toString(type, e, false) << " " << trim(toString(*q, e)) << ";";
            end();
            _out << "\n";

            string reason;
            metadata.clear();
            if(deprecatedException || (*q)->findMetaData("deprecate", metadata))
            {
                reason = "This member is deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    reason = metadata.substr(10);
                }
            }
            printComment(*q, e, reason);
        }
        end();
        _out << "\n{ztop}\n";
    }

    start("hr");
    end();
    printHeaderFooter(e);

    closeDoc();

    assert(_out.currIndent() == indent);
}

Slice::ClassGenerator::ClassGenerator(Confluence::ConfluenceOutput& o, const Files& files)
    : GeneratorBase(o, files)
{
}

void
Slice::ClassGenerator::generate(const ClassDefPtr& c)
{
#ifndef NDEBUG
    int indent = _out.currIndent();
#endif

    openDoc(c);

    printHeaderFooter(c);
    start("hr");
    end();

    start("h1", "Symbol");
    _out << c->scoped().substr(2);
    end();

    string metadata, deprecateReason;
    bool deprecatedClass = c->findMetaData("deprecate", metadata);
    if(deprecatedClass)
    {
        deprecateReason = "This ";
        deprecateReason += c->isInterface() ? "interface" : "class";
        deprecateReason += " is deprecated.";
        if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
        {
            deprecateReason = "_" + trim(metadata.substr(10)) + "_";
        }
    }

    start("h2");
    _out << "Overview";
    end();
    start("h3", "Synopsis");
    printMetaData(c, isUserImplemented(c, c));
    if(c->isLocal())
    {
        _out << "local ";
    }
    _out << (c->isInterface() ? "interface" : "class" )<< " " << c->name();

    ClassList bases = c->bases();
    if(!bases.empty() && !bases.front()->isInterface())
    {
        _out << " extends " << toString(bases.front(), c, false);
        bases.pop_front();
    }

    if(!bases.empty())
    {
        _out << (c->isInterface() ? " extends " : " implements ");
        ClassList::const_iterator q = bases.begin();
        while(q != bases.end())
        {
            _out << toString(*q, c, false);
            if(++q != bases.end())
            {
                _out << ", ";
            }
        }
    }
    end();

    printComment(c, c, deprecateReason);

    OperationList operations = c->operations();

    if(_indexCount > 0 && operations.size() >= _indexCount)
    {
        start("h2");
        _out << "Operation Index";
        end();
        start("dl");
        for(OperationList::const_iterator q = operations.begin(); q != operations.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, c, false);
            end();
            string summary = trim(getSummary(*q, c, (*q)->findMetaData("deprecate", metadata), false));
            if (!summary.empty())
            {
                start("dd");
                string metadata;
                _out << summary;
                end();
            }
            else
            {
                _out << "\n";
            }
        }
        end();
        _out << "\n{ztop}\n";
    }

    DataMemberList dataMembers = c->dataMembers();

    if(_indexCount > 0 && dataMembers.size() >= _indexCount)
    {
        start("h2");
        _out << "Data Member Index";
        end();
        start("dl");
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, c, false);
            end();
            string summary = trim(getSummary(*q, c, (*q)->findMetaData("deprecate", metadata), false));
            if (!summary.empty())
            {
                start("dd");
                string metadata;
                _out << summary;
                end();
            }
            else
            {
                _out << "\n";
            }
        }
        end();
        _out << "\n{ztop}\n";
    }

    if(!operations.empty())
    {
        start("h2");
        _out << "Operations";
        end();
        for(OperationList::const_iterator q = operations.begin(); q != operations.end(); ++q)
        {
            start("h3", "Synopsis");
            printMetaData(*q, isUserImplemented(*q, c));
            TypePtr returnType = (*q)->returnType();
            _out << (returnType ? toString(returnType, c, false) : string("void"))
                 << " " << trim(toString(*q, c)) << "(";
            ParamDeclList params = (*q)->parameters();
            ParamDeclList::const_iterator r = params.begin();
            while(r != params.end())
            {
                if((*r)->isOutParam())
                {
                    _out << "out ";
                }
                _out << toString((*r)->type(), ContainedPtr::dynamicCast(*q)->container(), false)
                     << " " << (*r)->name();
                if(++r != params.end())
                {
                    _out << ", ";
                }
            }
            _out << ")";
            ExceptionList throws = (*q)->throws();
            if(!throws.empty())
            {
                _out << " throws ";
                ExceptionList::const_iterator t = throws.begin();
                while(t != throws.end())
                {
                    _out << toString(*t, c, false);
                    if(++t != throws.end())
                    {
                        _out << ", ";
                    }
                }
            }
            end();

            string reason;
            metadata.clear();
            if(deprecatedClass || (*q)->findMetaData("deprecate", metadata))
            {
                reason = "This operation is deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    reason = "_" + metadata.substr(10) + "_";
                }
            }
            printComment(*q, c, reason);
        }

        _out << "\n{ztop}\n";
    }

    if(!dataMembers.empty())
    {
        start("h2");
        _out << "Data Members";
        end();
        start("p");
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("h3");
            printMetaData(*q, isUserImplemented(*q, c));
            TypePtr type = (*q)->type();
            _out << toString(type, c, false) << " " << trim(toString(*q, c)) << ";";
            end();
            _out << "\n";

            string reason;
            metadata.clear();
            if(deprecatedClass || (*q)->findMetaData("deprecate", metadata))
            {
                reason = "This member is deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    reason = metadata.substr(10);
                }
            }
            printComment(*q, c, reason);
        }
        end();
        _out << "\n{ztop}\n";
    }

    start("hr");
    end();
    printHeaderFooter(c);

    closeDoc();

    assert(_out.currIndent() == indent);
}

Slice::StructGenerator::StructGenerator(Confluence::ConfluenceOutput& o, const Files& files)
    : GeneratorBase(o, files)
{
}

void
Slice::StructGenerator::generate(const StructPtr& s)
{
#ifndef NDEBUG
    int indent = _out.currIndent();
#endif

    openDoc(s);

    printHeaderFooter(s);
    start("hr");
    end();

    start("h1", "Symbol");
    _out << s->scoped().substr(2);
    end();

    string metadata, deprecateReason;
    bool deprecatedException = s->findMetaData("deprecate", metadata);
    if(deprecatedException)
    {
        deprecateReason = "This module is deprecated.";
        if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
        {
            deprecateReason = "_" + trim(metadata.substr(10)) + "_";
        }
    }

    start("h2");
    _out << "Overview";
    end();

    start("h3", "Synopsis");
    printMetaData(s, isUserImplemented(s, s));
    if(s->isLocal())
    {
        _out << "local ";
    }
    _out << "struct " << s->name();
    end();

    printComment(s, s, deprecateReason);

    DataMemberList dataMembers = s->dataMembers();

    if(_indexCount > 0 && dataMembers.size() >= _indexCount)
    {
        start("h2");
        _out << "Data Member Index";
        end();
        start("dl");
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, s, false);
            end();
            string metadata;
            string summary = trim(getSummary(*q, s, (*q)->findMetaData("deprecate", metadata), false));
            if (!summary.empty())
            {
                start("dd");
                printSummary(*q, s, (*q)->findMetaData("deprecate", metadata), false);
                end();
            }
            else
            {
                _out << "\n";
            }
        }
        end();
        _out << "\n{ztop}\n";
    }

    if(!dataMembers.empty())
    {
        start("h2");
        _out << "Data Members";
        end();
        start("p");
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("h3");
            printMetaData(*q, isUserImplemented(*q, s));
            TypePtr type = (*q)->type();
            _out << toString(type, s, false) << " " << trim(toString(*q, s)) << ";";
            end();
            _out << "\n";

            string reason;
            metadata.clear();
            if(deprecatedException || (*q)->findMetaData("deprecate", metadata))
            {
                reason = "This member is deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    reason = metadata.substr(10);
                }
            }
            printComment(*q, s, reason);
        }
        end();
        _out << "\n{ztop}\n";
    }

    start("hr");
    end();
    printHeaderFooter(s);

    closeDoc();

    assert(_out.currIndent() == indent);
}

Slice::EnumGenerator::EnumGenerator(Confluence::ConfluenceOutput& o, const Files& files)
    : GeneratorBase(o, files)
{
}

void
Slice::EnumGenerator::generate(const EnumPtr& e)
{
#ifndef NDEBUG
    int indent = _out.currIndent();
#endif

    openDoc(e);

    printHeaderFooter(e);
    start("hr");
    end();

    start("h1", "Symbol");
    _out << e->scoped().substr(2);
    end();

    string metadata, deprecateReason;
    bool deprecatedException = e->findMetaData("deprecate", metadata);
    if(deprecatedException)
    {
        deprecateReason = "This enumeration is deprecated.";
        if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
        {
            deprecateReason = "_" + trim(metadata.substr(10)) + "_";
        }
    }

    start("h2");
    _out << "Overview";
    end();

    start("h3", "Synopsis");
    printMetaData(e, isUserImplemented(e, e->container()));
    if(e->isLocal())
    {
        _out << "local ";
    }
    _out << "enum " << e->name();
    end();

    printComment(e, e->container(), deprecateReason, false);

    EnumeratorList enumerators = e->enumerators();

    if(!enumerators.empty())
    {
        start("h2");
        _out << "Enumerator Index";
        end();
        start("dl");
        for(EnumeratorList::const_iterator q = enumerators.begin(); q != enumerators.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, e->container(), false, true);
            end();
            string summary = trim(getSummary(*q, e->container(), false, true));
            if (!summary.empty())
            {
                start("dd");
                _out << summary;
                end();
            }
            else
            {
                _out << "\n";
            }
        }
        end();
        _out << "\n{ztop}\n";

    }

    if(!enumerators.empty())
    {
        start("h2");
        _out << "Enumerators";
        end();
        start("p");
        for(EnumeratorList::const_iterator q = enumerators.begin(); q != enumerators.end(); ++q)
        {
            start("h3");
            printMetaData(*q, isUserImplemented(*q, e->container()));
            _out << toString(*q, e->container(), true, true);
            end();
            _out << "\n";

            string reason;
            //
            // Enumerators do not support metadata.
            //
            printComment(*q, e->container(), reason, false);
        }
        end();
        _out << "\n{ztop}\n";
    }


    closeDoc();

    start("hr");
    end();
    printHeaderFooter(e);

    assert(_out.currIndent() == indent);
}

Slice::PageVisitor::PageVisitor(const Files& files) :
    _files(files)
{
}

bool
Slice::PageVisitor::visitUnitStart(const UnitPtr& unit)
{
    return true;
}

bool
Slice::PageVisitor::visitModuleStart(const ModulePtr& m)
{
    Confluence::ConfluenceOutput O;
    ModuleGenerator mg(O, _files);
    mg.generate(m);
    return true;
}

bool
Slice::PageVisitor::visitExceptionStart(const ExceptionPtr& e)
{
    Confluence::ConfluenceOutput O;
    ExceptionGenerator eg(O, _files);
    eg.generate(e);
    return true;
}

bool
Slice::PageVisitor::visitClassDefStart(const ClassDefPtr& c)
{
    Confluence::ConfluenceOutput O;
    ClassGenerator cg(O, _files);
    cg.generate(c);
    return true;
}

bool
Slice::PageVisitor::visitStructStart(const StructPtr& s)
{
    Confluence::ConfluenceOutput O;
    StructGenerator sg(O, _files);
    sg.generate(s);
    return true;
}

void
Slice::PageVisitor::visitEnum(const EnumPtr& e)
{
    Confluence::ConfluenceOutput O;
    EnumGenerator eg(O, _files);
    eg.generate(e);
}
