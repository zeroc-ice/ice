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

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#ifdef __BCPLUSPLUS__
#  include <iterator>
#endif

using namespace std;
using namespace Slice;
using namespace IceUtil;

namespace Slice
{

void
generate(const UnitPtr& unit, const string& dir,
         const string& header, const string& footer,
         const string& indexHeader, const string& indexFooter,
         const string& imageDir, const string& logoURL, const string& searchAction, unsigned indexCount, unsigned warnSummary)
{
    unit->mergeModules();

    //
    // I don't want the top-level module to be sorted, therefore no
    // p->sort() before or after the p->sortContents().
    //
    unit->sortContents(false);

    GeneratorBase::setOutputDir(dir);
    GeneratorBase::setHeader(header);
    GeneratorBase::setFooter(footer);
    GeneratorBase::setImageDir(imageDir);
    GeneratorBase::setLogoURL(logoURL);
    GeneratorBase::setSearchAction(searchAction);
    GeneratorBase::setIndexCount(indexCount);
    GeneratorBase::warnSummary(warnSummary);

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

Slice::GeneratorBase::GeneratorBase(XMLOutput& o, const Files& files)
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

    string h1;
    string h2;
    if(header.empty())
    {
        _out << _header1;
    }
    else
    {
        getHeaders(header, h1, h2);
        _out << h1;
    }
    _out << title;
    if(header.empty())
    {
        _out << _header2;
    }
    else
    {
        _out << h2;
    }
    _indexFooter = getFooter(footer);
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
        path += "/" + *i;
        ++num;
        if(num < components.size())
        {
            makeDir(path);
        }
    }
    path += ".html";

    openStream(path);

    _out << _header1;
    _out << c->scoped().substr(2); // Text for title element.
    _out << _header2;
    _out.inc();
    _out.inc();
}

//
// Close an open HTML file after writing the footer.
//
void
Slice::GeneratorBase::closeDoc()
{
    _out.dec();
    _out.dec();
    _out << nl << (!_indexFooter.empty() ? _indexFooter : _footer);
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
    _out << se(s);
}

void
Slice::GeneratorBase::end()
{
    _out << ee;
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

    string::size_type pos = comment.find_last_not_of(" \t\r\n");
    if(pos != string::npos)
    {
        comment.erase(pos + 1);
        start("p");
        _out.zeroIndent();
        _out << nl << comment;
        _out.restoreIndent();
        end();
    }

    if(!deprecateReason.empty())
    {
        start("p", "Deprecated");
        _out << nl << deprecateReason;
        end();
    }

    assert(_out.currIndent() == indent);

    if(!par.empty())
    {
        start("h4");
        _out << "Parameters";
        end();
        start("dl");
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
            
            start("dt", "Symbol");
            _out << term;
            end();
            start("dd");
            _out << nl << item;
            end();
        }
        end();
    }

    if(!ret.empty())
    {
        start("h4");
        _out << "Return Value";
        end();
        start("p");
        _out << ret.front();
        end();
    }

    if(!throws.empty())
    {
        start("h4");
        _out << "Exceptions";
        end();
        start("dl");
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
            
            start("dt", "Symbol");
            _out << toString(term, container, false, forIndex);
            end();
            start("dd");
            _out << nl << item;
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
        start("h4");
        _out << "Derived Classes and Interfaces";
        end();
        start("dl");
        for(ClassList::const_iterator q = derivedClasses.begin(); q != derivedClasses.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, container, false, forIndex);
            end();
        }
        end();
    }

    ExceptionList derivedExceptions;
    ExceptionPtr ex = ExceptionPtr::dynamicCast(p);
    if(ex)
    {
        derivedExceptions = p->unit()->findDerivedExceptions(ex);
        if(!derivedExceptions.empty())
        {
            start("h4");
            _out << "Derived Exceptions";
            end();
            start("dl");
            for(ExceptionList::const_iterator q = derivedExceptions.begin(); q != derivedExceptions.end(); ++q)
            {
                start("dt", "Symbol");
                _out << toString(*q, container, false, forIndex);
                end();
            }
            end();
        }

        ContainedList usedBy;
        usedBy = p->unit()->findUsedBy(ex);
        if(!usedBy.empty())
        {
            start("h4");
            _out << "Used By";
            end();
            start("dl");
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
                start("dt", "Symbol");
                _out << *r;
                end();
            }
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
        StringList strings;
        for(ContainedList::const_iterator q = usedBy.begin(); q != usedBy.end(); ++q)
        {
            strings.push_back(toString(*q, container, false, forIndex));
        }
        strings.sort();
        strings.unique();

        start("h4");
        _out << "Used By";
        end();
        start("dl");
        for(list<string>::const_iterator p = strings.begin(); p != strings.end(); ++p)
        {
            start("dt", "Symbol");
            _out << *p;
            end();
        }
        end();
    }

    if(!see.empty())
    {
        start("h4");
        _out << "See Also";
        end();
        start("dl");
        for(StringList::const_iterator q = see.begin(); q != see.end(); ++q)
        {
            start("dt", "Symbol");
            _out << toString(*q, container, false, forIndex);
            end();
        }
        end();
    }

    assert(_out.currIndent() == indent);
}

void
Slice::GeneratorBase::printMetaData(const ContainedPtr& p)
{
    list<string> metaData = p->getMetaData();

    if(!metaData.empty())
    {
        _out << "[";
        list<string>::const_iterator q = metaData.begin();
        while(q != metaData.end())
        {
            _out << " \"" << *q << "\"";
            if(++q != metaData.end())
            {
                _out << ",";
            }
        }
        _out << " ]" << nl;
    }
}

void
Slice::GeneratorBase::printSummary(const ContainedPtr& p, const ContainerPtr& module, bool deprecated)
{
    ContainerPtr container = ContainerPtr::dynamicCast(p);
    if(!container)
    {
        container = p->container();
    }

    if(module)
    {
        container = module;
    }

    string summary = getComment(p, container, true, module);
    _out << nl << summary;

    if(deprecated)
    {
        start("p", "Deprecated");
        _out << nl << "Deprecated.";
        end();
    }
}

void
Slice::GeneratorBase::printHeaderFooter(const ContainedPtr& c)
{
    ContainerPtr container = ModulePtr::dynamicCast(c) ? c->container() : ContainerPtr::dynamicCast(c);
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
    bool hasParent = ContainedPtr::dynamicCast(container) || EnumPtr::dynamicCast(c);

    bool onEnumPage = EnumPtr::dynamicCast(c);

    string prevLink;
    string prevClass;
    if(!isFirst)
    {
        prevLink = getLinkPath(*prev, container, false, onEnumPage) + ".html";
        prevClass = "Button";
    }
    else
    {
        prevClass = "ButtonGrey";
    }

    string nextLink;
    string nextClass;
    if(!isLast)
    {
        nextLink = getLinkPath(*next, container, false, onEnumPage) + ".html";
        nextClass = "Button";
    }
    else
    {
        nextClass = "ButtonGrey";
    }

    string upLink;
    string upClass;
    if(hasParent)
    {
        upLink = getLinkPath(c->container(), container, ModulePtr::dynamicCast(c), onEnumPage) + ".html";
        upClass = "Button";
    }
    else
    {
        upClass = "ButtonGrey";
    }

    string homeLink = getLinkPath(0, container, ModulePtr::dynamicCast(c), onEnumPage);
    if(!homeLink.empty())
    {
        homeLink += "/";
    }
    homeLink += "index.html";

    string indexLink = getLinkPath(0, container, ModulePtr::dynamicCast(c), onEnumPage);
    if(!indexLink.empty())
    {
        indexLink += "/";
    }
    indexLink += "_sindex.html";

    string imageDir = getImageDir();

    string prevImage = imageDir.empty() ? "Previous" : (isFirst ? "prevx.gif" : "prev.gif");
    string nextImage = imageDir.empty() ? "Next" : (isLast ? "nextx.gif" : "next.gif");
    string upImage = imageDir.empty()? "Up" : (hasParent ? "up.gif" : "upx.gif");

    string homeImage = imageDir.empty() ? "Home" : "home.gif";
    string indexImage = imageDir.empty() ? "Index" : "index.gif";

    if(!imageDir.empty())
    {
        string path = getLinkPath(0, container, ModulePtr::dynamicCast(c), onEnumPage);
        if(!path.empty())
        {
            path += "/";
        }
        path += imageDir + "/";

        prevImage = "<img class=\"" + prevClass + "\" src=\"" + path + prevImage + "\" alt=\"Previous\"/>";
        nextImage = "<img class=\"" + nextClass + "\" src=\"" + path + nextImage + "\" alt=\"Next\"/>";
        upImage = "<img class=\"" + upClass + "\" src=\"" + path + upImage + "\" alt=\"Up\"/>";
        homeImage = "<img class=\"Button\" src=\"" + path + homeImage + "\" alt=\"Home\"/>";
        indexImage = "<img class=\"Button\" src=\"" + path + indexImage + "\" alt=\"Index\"/>";
    }

    _out << nl << "<!-- SwishCommand noindex -->";

    start("div", "HeaderFooter");

    start("table", "ButtonTable");
    start("tr");

    start("td");
    _out << "<a href=\"" << homeLink << "\">" << homeImage << "</a>";
    end();

    if(!imageDir.empty() || !isFirst)
    {
        start("td");
        if(!isFirst)
        {
            _out << "<a href=\"" << prevLink << "\">";
        }
        _out << prevImage;
        if(!isFirst)
        {
            _out << "</a>";
        }
        end();
    }

    if(!imageDir.empty() || hasParent)
    {
        start("td");
        if(hasParent)
        {
            _out << "<a href=\"" << upLink << "\">";
        }
        _out << upImage;
        if(hasParent)
        {
            _out << "</a>";
        }
        end();
    }

    if(!imageDir.empty() || !isLast)
    {
        start("td");
        if(!isLast)
        {
            _out << "<a href=\"" << nextLink << "\">";
        }
        _out << nextImage;
        if(!isLast)
        {
            _out << "</a>";
        }
        end();
    }

    start("td");
    _out << "<a href=\"" << indexLink << "\">" << indexImage << "</a>";
    end();

    end();
    end();

    printSearch();

    printLogo(c, container, onEnumPage);

    _out << nl << "<!-- SwishCommand index -->";

    end();
}

void
Slice::GeneratorBase::printSearch()
{
    if(!_searchAction.empty())
    {
        _out << nl << "<div style=\"text-align: center;\">";
        _out.inc();
        start("table", "SearchTable");
        start("tr");
        start("td");
        _out << nl << "<form method=\"get\" action=\"" << _searchAction << "\""
             << " enctype=\"application/x-www-form-urlencoded\" class=\"form\">";
        _out.inc();
        start("div");
        _out << nl << "<input maxlength=\"100\" value=\"\" type=\"text\" name=\"query\">";
        _out << nl << "<input type=\"submit\" value=\"Search\" name=\"submit\">";
        end();
        _out.dec();
        _out << nl << "</form>";
        end();
        end();
        end();
        _out.dec();
        _out << nl << "</div>";
    }
}

void
Slice::GeneratorBase::printLogo(const ContainedPtr& c, const ContainerPtr& container, bool forEnum)
{
    string imageDir = getImageDir();
    if(!imageDir.empty())
    {
        string path = getLinkPath(0, container, ModulePtr::dynamicCast(c), forEnum);
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
            _out << "<a href=\"" + _logoURL + "\">";
        }
        _out << "<img class=\"Logo\" src=\"" + path + "\" alt=\"Logo\"/>";
        if(!_logoURL.empty())
        {
            _out << "</a>";
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
        return s;
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

            //
            // Sequences and dictionaries are documented on the page for their
            // enclosing module.
            //
            if(SequencePtr::dynamicCast(p) || DictionaryPtr::dynamicCast(p))
            {
                linkpath = getLinkPath(contained->container(), container, forIndex);
            }
            else
            {
                linkpath = getLinkPath(contained, container, forIndex);
            }
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
        return s;
    }

    string ret = "<a class=\"Symbol\" ";
    ret += (asTarget ? "name" : "href");
    ret += "=\"";
    if(asTarget)
    {
        ret += anchor;
    }
    else
    {
        ret += linkpath;
        if(!linkpath.empty())
        {
            ret += ".html";
        }
        if(!anchor.empty())
        {
            ret += "#" + anchor;
        }
    }
    ret += "\">";
    ret += s;
    ret += "</a>";
    if(ProxyPtr::dynamicCast(p))
    {
        ret += '*';
    }
    return ret;
}

string
Slice::GeneratorBase::toString(const string& str, const ContainerPtr& container, bool asTarget, bool forIndex,
                               size_t* summarySize)
{
    string s = str;

    TypeList types = container->lookupType(s, false);
    if(!types.empty())
    {
        return toString(types.front(), container, asTarget, forIndex, summarySize);
    }

    ContainedList contList = container->lookupContained(s, false);
    if(!contList.empty())
    {
        return toString(contList.front(), container, asTarget, forIndex, summarySize);
    }

    //
    // If we can't find the string, printing it in typewriter
    // font is the best we can do.
    //
    return "<tt>" + s + "</tt>";
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
            comment += toString(literal, container, false, forIndex, summary ? &sz : 0);
            summarySize += sz;
        }
        else if(summary && s[i] == '.' && (i + 1 >= s.size() || isspace(s[i + 1])))
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
        cerr << contained->definitionContext()->filename() << ": summary size (" << summarySize << ") exceeds "
            << _warnSummary << " characters: `" << comment << "'" << endl;
    }

    return comment;
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
    string path;
    while(!from.empty())
    {
        if(!path.empty())
        {
            path += "/";
        }
        path += "..";
        from.pop_front();
    }

    //
    // Now append the scope to the target.
    //
    while(!target.empty())
    {
        if(!path.empty())
        {
            path += "/";
        }
        path += target.front() == "index" ? string("_index") : target.front();
        target.pop_front();
    }

    if(forEnum)
    {
        if(!path.empty())
        {
            path = "../" + path;
        }
        else
        {
            path = "..";
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
        string err = "cannot open `" + path + "' for writing";
        throw err;
    }
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
    int rc = stat(dir.c_str(), &st);
    if(rc == 0)
    {
        return;
    }
#ifdef _WIN32
    rc = mkdir(dir.c_str());
#else
    rc = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
    if(rc != 0)
    {
        string err = "cannot create directory `" + dir + "'";
        throw err;
    }
}

string
Slice::GeneratorBase::readFile(const string& file)
{
    ifstream in(file.c_str());
    if(!in)
    {
        string err = "cannot open `" + file + "' for reading";
        throw err;
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
    if(header.empty())
    {
        ostringstream hdr1;
        XMLOutput O1(hdr1);
        O1 << "<!-- Generated by Ice version " << ICE_STRING_VERSION << " -->";
        O1 << sp;
        O1 << nl << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">";
        O1 << se("html");
        O1 << se("head");
        O1 << se("title") << nl;
        h1 = hdr1.str();

        // _header1 and _header2 store the bit preceding and following the title.
        // _header1, the title text, and _header2 are written by openDoc().

        ostringstream hdr2;
        XMLOutput O2(hdr2);
        O2.inc();
        O2.inc();
        O2 << nl << "</title>";
        O2.dec();
        O2 << nl << "</head>";
        O2 << nl << "<body>";
        h2 = hdr2.str();
    }
    else
    {
        readFile(header, h1, h2);
    }
}

string
Slice::GeneratorBase::getFooter(const string& footer)
{
    ostringstream ftr;
    XMLOutput O(ftr);
    if(footer.empty())
    {
        O << "    </body>";
    }
    else
    {
        O << readFile(footer);
    }
    O << nl << "</html>";
    return ftr.str();
}

void
Slice::GeneratorBase::readFile(const string& file, string& part1, string& part2)
{
    ifstream in(file.c_str());
    if(!in)
    {
        string err = "cannot open `" + file + "' for reading";
        throw err;
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
    openDoc("index.html", "Slice API Documentation");
}

Slice::StartPageGenerator::~StartPageGenerator()
{
    ::std::sort(_modules.begin(), _modules.end());

    printHeaderFooter();

    _out << nl << "<!-- SwishCommand noindex -->";

    _out << nl << "<hr>";

    start("h1");
    _out << "Slice API Documentation";
    end();
    start("h2");
    _out << "Modules";
    end();
    start("dl");
    for(ModuleDescriptions::const_iterator i = _modules.begin(); i != _modules.end(); ++i)
    {
        start("dt", "Symbol");
        _out << i->first;
        end();
        start("dd");
        _out << i->second;
        end();
    }
    end();

    _out << nl << "<hr>";
    printHeaderFooter();

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
    start("div", "HeaderFooter");

    start("table", "ButtonTable");
    start("tr");
    start("td");
    string imageDir = getImageDir();
    if(imageDir.empty())
    {
        _out << "<a href=\"_sindex.html\">Index</a>";
    }
    else
    {
        string src = imageDir + "/index.gif";
        _out << "<a href=\"_sindex.html\"><img class=\"Button\" src=\"" + src + "\" alt=\"Index Button\"/></a>";
    }
    end();
    end();
    end();

    printSearch();

    if(!imageDir.empty())
    {
        start("table", "LogoTable");
        start("tr");
        start("td");
        string logoURL = getLogoURL();
        if(!logoURL.empty())
        {
            _out << "<a href=\"" + logoURL + "\">";
        }
        _out << "<img class=\"Logo\" src=\"" + imageDir + "/logo.gif\" alt=\"Logo\"/>";
        if(!logoURL.empty())
        {
            _out << "</a>";
        }
        end();
        end();
        end();
    }

    end();
}

Slice::FileVisitor::FileVisitor(Files& files)
    : _files(files)
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
    _files.insert(m->definitionContext()->filename());
    return true;
}

bool
Slice::FileVisitor::visitExceptionStart(const ExceptionPtr& e)
{
    _files.insert(e->definitionContext()->filename());
    return false;
}

bool
Slice::FileVisitor::visitClassDefStart(const ClassDefPtr& c)
{
    _files.insert(c->definitionContext()->filename());
    return false;
}

void
Slice::FileVisitor::visitClassDecl(const ClassDeclPtr& c)
{
    _files.insert(c->definitionContext()->filename());
}

bool
Slice::FileVisitor::visitStructStart(const StructPtr& s)
{
    _files.insert(s->definitionContext()->filename());
    return false;
}

void
Slice::FileVisitor::visitSequence(const SequencePtr& s)
{
    _files.insert(s->definitionContext()->filename());
}

void
Slice::FileVisitor::visitDictionary(const DictionaryPtr& d)
{
    _files.insert(d->definitionContext()->filename());
}

void
Slice::FileVisitor::visitEnum(const EnumPtr& e)
{
    _files.insert(e->definitionContext()->filename());
}

Slice::StartPageVisitor::StartPageVisitor(const Files& files)
    : _spg(files)
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
    openDoc("_sindex.html", "Slice API Index", header, footer);

    start("h1");
    _out << "Slice API Index";
    end();

    start("table", "ExpandCollapseButtonTable");
    start("tbody");
    start("tr");

    start("td");
    _out << "<button type=\"button\" id=\"ExpandAllButton\">Expand All</button>";
    end();

    start("td");
    _out << "<button type=\"button\" id=\"CollapseAllButton\">Collapse All</button>";
    end();

    end();
    end();
    end();
}

void
TOCGenerator::generate(const ModulePtr& m)
{
    _modules.push_back(m);
}

void
TOCGenerator::writeTOC()
{
    _modules.sort();

    _out << nl << "<ul id=\"SymbolTree\">";
    _out.inc();
    for(ModuleList::const_iterator i = _modules.begin(); i != _modules.end(); ++i)
    {
        writeEntry(*i);
    }
    _out.dec();
    _out << nl << "</ul>";

    _symbols.sort();
    _symbols.unique();

    closeDoc();
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
        EnumeratorList enumerators = en->getEnumerators();
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

        _out << nl << toString(c, 0, false, true, 0, true);
        start("ul");
        for(ContainedList::const_iterator i = cl.begin(); i != cl.end(); ++i)
        {
            writeEntry(*i);
        }
        end();
    }
    else
    {
        _out << nl << toString(c, 0, false, true, 0, true);
    }
    if(ModulePtr::dynamicCast(c) || ExceptionPtr::dynamicCast(c) || ClassDefPtr::dynamicCast(c) ||
       StructPtr::dynamicCast(c) || EnumPtr::dynamicCast(c))
    {
        _symbols.push_back(c);
    }
    else if(ClassDeclPtr::dynamicCast(c))
    {
        _symbols.push_back(ClassDeclPtr::dynamicCast(c)->definition());
    }
    end();
}

TOCVisitor::TOCVisitor(const Files& files, const string& header, const string& footer)
    : _tg(files, header, footer)
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

Slice::ModuleGenerator::ModuleGenerator(XMLOutput& o, const Files& files)
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
    _out << nl << "<hr>";

    start("h1", "Symbol");
    _out << toString(m, m->container(), true);
    end();

    string metadata, deprecateReason;
    if(m->findMetaData("deprecate", metadata))
    {
        deprecateReason = "This module is deprecated.";
        if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
        {
            deprecateReason = metadata.substr(10);
        }
    }
    start("h2");
    _out << "Overview";
    end();
    start("h3", "Synopsis");
    printMetaData(m);
    _out << "module " << m->name();
    end();

    printComment(m, m, deprecateReason, true);

    visitContainer(m);

    _out << nl << "<hr>";
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
            start("dd");
            string metadata;
            printSummary(*q, p, (*q)->findMetaData("deprecate", metadata));
            end();
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
            start("dd");
            string metadata;
            printSummary(*q, p, (*q)->findMetaData("deprecate", metadata));
            end();
        }
        end();
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
            start("dd");
            string metadata;
            printSummary(*q, p, (*q)->findMetaData("deprecate", metadata));
            end();
        }
        end();
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
            start("dd");
            string metadata;
            printSummary(*q, p, (*q)->findMetaData("deprecate", metadata));
            end();
        }
        end();
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
            start("dd");
            string metadata;
            printSummary(*q, p, (*q)->findMetaData("deprecate", metadata));
            end();
        }
        end();
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
            start("dd");
            string metadata;
            printSummary(*q, p, (*q)->findMetaData("deprecate", metadata));
            end();
        }
        end();
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
            start("dd");
            string metadata;
            printSummary(*q, p, (*q)->findMetaData("deprecate", metadata));
            end();
        }
        end();
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
            start("dd");
            string metadata;
            printSummary(*q, p, (*q)->findMetaData("deprecate", metadata));
            end();
        }
        end();
    }

    assert(_out.currIndent() == indent);

    if(!sequences.empty())
    {
        start("h2");
        _out << "Sequences";
        end();
        for(SequenceList::const_iterator q = sequences.begin(); q != sequences.end(); ++q)
        {
            start("dl");
            start("dt");
            start("span", "Synopsis");
            printMetaData(*q);
            if((*q)->isLocal())
            {
                _out << "local ";
            }
            TypePtr type = (*q)->type();
            _out << "sequence&lt;" << toString(type, p, false, true) << "&gt; " << toString(*q, p);
            end();
            end();

            start("dd");
            string metadata, deprecateReason;
            if((*q)->findMetaData("deprecate", metadata))
            {
                deprecateReason = "This type is deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    deprecateReason = metadata.substr(10);
                }
            }

            printComment(*q, p, deprecateReason, true);
            end();
            end();
        }
    }

    if(!dictionaries.empty())
    {
        start("h2");
        _out << "Dictionaries";
        end();
        for(DictionaryList::const_iterator q = dictionaries.begin(); q != dictionaries.end(); ++q)
        {
            start("dl");
            start("dt");
            start("span", "Synopsis");
            printMetaData(*q);
            if((*q)->isLocal())
            {
                _out << "local ";
            }
            TypePtr keyType = (*q)->keyType();
            TypePtr valueType = (*q)->valueType();
            _out << "dictionary&lt;" << toString(keyType, p, false, true) << ", "
                 << toString(valueType, p, false, true) << "&gt; " << toString(*q, p);
            end();
            end();

            start("dd");
            string metadata, deprecateReason;
            if((*q)->findMetaData("deprecate", metadata))
            {
                deprecateReason = "This type is deprecated.";
                if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
                {
                    deprecateReason = metadata.substr(10);
                }
            }

            printComment(*q, p, deprecateReason, true);
            end();
            end();
        }
    }
}

Slice::ExceptionGenerator::ExceptionGenerator(XMLOutput& o, const Files& files)
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
    _out << nl << "<hr>";

    start("h1", "Symbol");
    _out << toString(e, e, true);
    end();

    string metadata, deprecateReason;
    bool deprecatedException = e->findMetaData("deprecate", metadata);
    if(deprecatedException)
    {
        deprecateReason = "This module is deprecated.";
        if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
        {
            deprecateReason = metadata.substr(10);
        }
    }

    start("h2");
    _out << "Overview";
    end();

    start("h3", "Synopsis");
    printMetaData(e);
    if(e->isLocal())
    {
        _out << "local ";
    }
    _out << "exception " << e->name();
    ExceptionPtr base = e->base();
    if(base)
    {
        _out.inc();
        _out << nl << "extends ";
        _out.inc();
        _out << nl << toString(base, e);
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
            start("dd");
            string metadata;
            printSummary(*q, e, (*q)->findMetaData("deprecate", metadata));
            end();
        }
        end();
    }

    if(!dataMembers.empty())
    {
        start("h2");
        _out << "Data Members";
        end();
        start("dl");
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("dt", "Symbol");
            printMetaData(*q);
            TypePtr type = (*q)->type();
            _out << toString(type, e) << " " << toString(*q, e) << ";";
            end();

            start("dd");
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
            end();
        }
        end();
    }
        
    _out << nl << "<hr>";
    printHeaderFooter(e);

    closeDoc();

    assert(_out.currIndent() == indent);
}

Slice::ClassGenerator::ClassGenerator(XMLOutput& o, const Files& files)
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
    _out << nl << "<hr>";

    start("h1", "Symbol");
    _out << toString(c, c, true);
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
            deprecateReason = metadata.substr(10);
        }
    }

    start("h2");
    _out << "Overview";
    end();
    start("h3", "Synopsis");
    printMetaData(c);
    if(c->isLocal())
    {
        _out << "local ";
    }
    _out << (c->isInterface() ? "interface" : "class" )<< " " << c->name();

    ClassList bases = c->bases();
    if(!bases.empty() && !bases.front()->isInterface())
    {
        _out << " extends " << toString(bases.front(), c);
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
            start("dd");
            string metadata;
            printSummary(*q, c, (*q)->findMetaData("deprecate", metadata));
            end();
        }
        end();
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
            start("dd");
            string metadata;
            printSummary(*q, c, (*q)->findMetaData("deprecate", metadata));
            end();
        }
        end();
    }

    if(!operations.empty())
    {
        start("h2");
        _out << "Operations";
        end();
        for(OperationList::const_iterator q = operations.begin(); q != operations.end(); ++q)
        {
            start("h3", "Synopsis");
            TypePtr returnType = (*q)->returnType();
            _out << (returnType ? toString(returnType, c, false) : string("void")) << " "
                 << toString(*q, c) << "(";
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
                    reason = metadata.substr(10);
                }
            }
            printComment(*q, c, reason);
        }
    }

    if(!dataMembers.empty())
    {
        start("h2");
        _out << "Data Members";
        end();
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("h3", "Synopsis");
            printMetaData(*q);
            TypePtr type = (*q)->type();
            _out << toString(type, c, false) << " " << toString(*q, c) << ";";
            end();

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
    }

    _out << nl << "<hr>";
    printHeaderFooter(c);

    closeDoc();

    assert(_out.currIndent() == indent);
}

Slice::StructGenerator::StructGenerator(XMLOutput& o, const Files& files)
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
    _out << nl << "<hr>";

    start("h1", "Symbol");
    _out << toString(s, s, true);
    end();

    string metadata, deprecateReason;
    bool deprecatedException = s->findMetaData("deprecate", metadata);
    if(deprecatedException)
    {
        deprecateReason = "This module is deprecated.";
        if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
        {
            deprecateReason = metadata.substr(10);
        }
    }

    start("h2");
    _out << "Overview";
    end();

    start("h3", "Synopsis");
    printMetaData(s);
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
            start("dd");
            string metadata;
            printSummary(*q, s, (*q)->findMetaData("deprecate", metadata));
            end();
        }
        end();
    }

    if(!dataMembers.empty())
    {
        start("h2");
        _out << "Data Members";
        end();
        start("dl");
        for(DataMemberList::const_iterator q = dataMembers.begin(); q != dataMembers.end(); ++q)
        {
            start("dt", "Symbol");
            printMetaData(*q);
            TypePtr type = (*q)->type();
            _out << toString(type, s, false) << " " << toString(*q, s) << ";";
            end();

            start("dd");
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
            end();
        }
        end();
    }
        
    _out << nl << "<hr>";
    printHeaderFooter(s);

    closeDoc();

    assert(_out.currIndent() == indent);
}

Slice::EnumGenerator::EnumGenerator(XMLOutput& o, const Files& files)
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
    _out << nl << "<hr>";

    start("h1", "Symbol");
    _out << toString(e, e->container(), true);
    end();

    string metadata, deprecateReason;
    bool deprecatedException = e->findMetaData("deprecate", metadata);
    if(deprecatedException)
    {
        deprecateReason = "This enumeration is deprecated.";
        if(metadata.find("deprecate:") == 0 && metadata.size() > 10)
        {
            deprecateReason = metadata.substr(10);
        }
    }

    start("h2");
    _out << "Overview";
    end();

    start("h3", "Synopsis");
    printMetaData(e);
    if(e->isLocal())
    {
        _out << "local ";
    }
    _out << "enum " << e->name();
    end();

    printComment(e, e->container(), deprecateReason, false);

    EnumeratorList enumerators = e->getEnumerators();
    if(!enumerators.empty())
    {
        start("h2");
        _out << "Enumerators";
        end();
        start("dl");
        for(EnumeratorList::const_iterator q = enumerators.begin(); q != enumerators.end(); ++q)
        {
            start("dt", "Symbol");
            _out << (*q)->name();
            end();

            start("dd");
            string reason;
            //
            // Enumerators do not support metadata.
            //
            printComment(*q, e->container(), reason, false);
            end();
        }
        end();
    }
        
    closeDoc();

    _out << nl << "<hr>";
    printHeaderFooter(e);

    assert(_out.currIndent() == indent);
}

Slice::PageVisitor::PageVisitor(const Files& files)
    : _files(files)
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
    XMLOutput O;
    ModuleGenerator mg(O, _files);
    mg.generate(m);
    return true;
}

bool
Slice::PageVisitor::visitExceptionStart(const ExceptionPtr& e)
{
    XMLOutput O;
    ExceptionGenerator eg(O, _files);
    eg.generate(e);
    return true;
}

bool
Slice::PageVisitor::visitClassDefStart(const ClassDefPtr& c)
{
    XMLOutput O;
    ClassGenerator cg(O, _files);
    cg.generate(c);
    return true;
}

bool
Slice::PageVisitor::visitStructStart(const StructPtr& s)
{
    XMLOutput O;
    StructGenerator sg(O, _files);
    sg.generate(s);
    return true;
}

void
Slice::PageVisitor::visitEnum(const EnumPtr& e)
{
    XMLOutput O;
    EnumGenerator eg(O, _files);
    eg.generate(e);
}
