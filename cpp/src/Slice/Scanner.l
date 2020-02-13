
%top{
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/ScannerConfig.h>

}

%{

#include <Slice/GrammarUtil.h>  // Before Grammer.h, so that YYSTYPE is defined
#include <Slice/Grammar.h>
#include <IceUtil/InputUtil.h>

#include <iomanip>

#include <stdlib.h>
#include <math.h>

#if defined(_MSC_VER)
// '<' : signed/unsigned mismatch
#   pragma warning(disable:4018)
// 'initializing' : conversion from '__int64' to 'int', possible loss of data
#   pragma warning(disable:4244)

#   if defined(ICE_64)
// '=' : conversion from 'size_t' to 'int', possible loss of data
// The result of fread() is a size_t and gets inserted into an int
#       pragma warning(disable:4267)
#   endif
#endif

#if defined(__GNUC__)
#   pragma GCC diagnostic ignored "-Wsign-compare"
#endif

// Avoid clang conversion warnings
#if defined(__clang__)
#   pragma clang diagnostic ignored "-Wconversion"
#   pragma clang diagnostic ignored "-Wsign-conversion"
#endif

#ifdef _MSC_VER
#   ifdef slice_wrap
#      undef slice_wrap
#      define slice_wrap() 1
#   endif
#   define YY_NO_UNISTD_H
#endif

#ifdef __SUNPRO_CC
#   ifdef slice_wrap
#      undef slice_wrap
#      define slice_wrap() 1
#   endif
#   ifdef ICE_64
#       pragma error_messages(off,truncwarn)
#   endif
#endif

using namespace std;
using namespace Slice;

namespace Slice
{

// Definitions for the case-insensitive keyword-token map.
static std::map<std::string, int> keywordMap;

void initScanner();
void preAction();
int checkKeyword(string&);
int checkIdentifier(string&);

}

#define YY_USER_INIT initScanner();
#define YY_USER_ACTION preAction();

%}

  /* Changes the default prefix of 'yy' to 'slice_' for functions and variables in the generated code. */
%option prefix="slice_"
  /* Instructs flex to not suppress any warnings when generating the scanner. */
%option warn
  /* By default flex will 'default match' any text it encounters that doesn't match any specified rules. This
   * option disables default-matching (it throws 'syntax error' instead) to make grammar holes more obvious. */
%option nodefault

  /* Enables the use of flex's built int start-condition state stack. */
%option stack
  /* Ensures flex generates a scanner that supports reading 8-bit characters. */
%option 8bit
  /* Directs flex to generate lookup tables that are better aligned in memory to
   * improve access speeds, even if this means allocating larger tables. */
%option align
  /* Enables batching for improved performance. */
%option batch
  /* Directs flex to store matched text as 'char *' instead of char arrays, for improved performance. */
%option pointer
  /* Disables the scanner's interactive modes for improved performance. */
%option never-interactive

  /* Disables the generation of functions we don't use to reduce clutter, and possibly improve performance. */
%option noyy_top_state noinput nounput
%option noyy_scan_buffer noyy_scan_bytes noyy_scan_string
%option noyyget_extra noyyset_extra noyyget_leng noyyget_text
%option noyyget_in noyyset_in noyyget_out noyyset_out
%option noyyget_lineno noyyset_lineno noyyget_lloc noyyset_lloc
%option noyyget_lval noyyset_lval noyyget_debug noyyset_debug
%option noyywrap

  /* List of start-condition states the scanner can be in. This lets the scanning be context dependent. */
%s SLICE
%x PREPROCESS
%x C_COMMENT
%x QUOTED_STRING
  /* The scanner also has a built in 'INITIAL' start-condition state, which is the state the scanner is initialized in.
   * We use it solely to check for and consume any BOMs at the start of files. See Bug 3140. */

oct                 [0-7]
dec                 [0-9]
hex                 [0-9a-fA-F]

preprocessor_prefix [[:blank:]]*#[[:blank:]]*
preprocessor_lineno {preprocessor_prefix}(line[[:blank:]]+)?{dec}+

identifier          ((::)?\\?[[:alpha:]_][[:alnum:]_]*)+

integer_constant    (\+|-)?((0[0-7]+)|(0x{hex}+)|({dec}+))
fractional_constant (\+|-)?(({dec}*\.{dec}+)|({dec}+\.))
exponent_part       (e|E)(\+|-)?{dec}+
floating_literal    (({fractional_constant}{exponent_part}?)|((\+|-)?{dec}+{exponent_part}))[fF]?

bom                 ^"\357\273\277"

%%

  /* ========== Preprocessor Statements ========== */

  /* Matches the empty preprocessor directive. */
^{preprocessor_prefix} {
    yy_push_state(PREPROCESS);
}

  /* Matches a line preprocessor directive, but missing a line number. */
^{preprocessor_prefix}line[[:blank:]]* {
    yy_push_state(PREPROCESS);
    unit->error("missing line number in line preprocessor directive");
}

  /* Matches a line preprocessor directive (optionally with a file specified afterwards). */
^{preprocessor_lineno}                         |
^{preprocessor_lineno}[[:blank:]]+\"[^\"\n]*\" {
    yy_push_state(PREPROCESS);
    unit->scanPosition(yytext);
}

  /* Matches any non white-space character. This is a catch-all to report any invalid characters
   * found while scanning the directive. */
<PREPROCESS>[[:^space:]]+ {
    unit->error("encountered unexpected token while scanning preprocessor directive: `" + string(yytext) + "'");
}

  /* Matches a new-line character. This signals the end of the preprocessor statement. */
<PREPROCESS>\n {
    yy_pop_state();
    if(unit->currentLine() == 0)
    {
        BEGIN(INITIAL);
    }
    unit->nextLine();
}

  /* ========== Comments ========== */

  /* Matches and consumes a C++ style comment. All characters after the
   * starting '//' and until the next new line are matched and discarded. */
"//".* {}

  /* Matches the start of a C style comment. */
"/*" {
    yy_push_state(C_COMMENT);
}

  /* TODO: use Flex's in-built line counter and merge this rule with the ones underneath it. */
<C_COMMENT>\n {
    yymore();
    unit->nextLine();
}

  /* Matches either a single literal '*' or as many other (non-'*') characters as it can (except newlines). */
<C_COMMENT>"*"     |
<C_COMMENT>[^*\n]+ {
    yymore();
}

  /* Matches the end of a C style comment. Flex always prefers to match the longest string it can,
   * so this rule will beat the previous one, which can only match a single '*' at a time. */
<C_COMMENT>"*/" {
    yy_pop_state();
    string comment(yytext);

    // The last 2 characters are the '*/' matched by this rule.
    unit->setComment(comment.substr(0, (comment.length() - 2)));
}

  /* Matches EOF, but only while parsing a C style comment. */
<C_COMMENT><<EOF>> {
    yy_pop_state();
    unit->warning(All, "encountered EOF while scanning a comment");
    unit->setComment(yytext);
}

  /* ========== Metadata ========== */

"[" {
    return ICE_METADATA_OPEN;
}

"]" {
    return ICE_METADATA_CLOSE;
}

"[[" {
    return ICE_GLOBAL_METADATA_OPEN;
}

"]]" {
    return ICE_GLOBAL_METADATA_CLOSE;
}

  /* ========== Identifiers and Keywords ========== */

{identifier}[[:space:]]*"(" {
    StringTokPtr ident = new StringTok;
    ident->v = *yytext == '\\' ? yytext + 1 : yytext;
    ident->v.erase(ident->v.find_first_of(" \t\v\n\r\f("));
    *yylvalp = ident;
    if(*yytext == '\\')
    {
        if(checkIdentifier(ident->v) == ICE_SCOPED_IDENTIFIER)
        {
            unit->error("Operation identifiers cannot be scoped: `" + (ident->v) + "'");
        }
        return ICE_IDENT_OPEN;
    }
    int st = checkKeyword(ident->v);
    if(st == ICE_IDENTIFIER)
    {
        return ICE_IDENT_OPEN;
    }
    else if(st == ICE_SCOPED_IDENTIFIER)
    {
        unit->error("Operation identifiers cannot be scoped: `" + (ident->v) + "'");
        return ICE_IDENT_OPEN;
    }
    else if(st == ICE_TAG)
    {
        return ICE_TAG_OPEN;
    }
    else if(st == ICE_OPTIONAL)
    {
        return ICE_OPTIONAL_OPEN;
    }
    else
    {
        return ICE_KEYWORD_OPEN;
    }
}

{identifier} {
    StringTokPtr ident = new StringTok;
    ident->v = *yytext == '\\' ? yytext + 1 : yytext;
    *yylvalp = ident;
    return *yytext == '\\' ? checkIdentifier(ident->v) : checkKeyword(ident->v);
}

  /* ========== Literals ========== */

  /* Matches the start of a double-quoted string literal. */
\" {
    yy_push_state(QUOTED_STRING);
    StringTokPtr str = new StringTok;
    str->literal = "\"";
    *yylvalp = new StringTok;
}

  /* Matches a single escaped backslash, or as many characters as it can,
   * except backslashes, new-lines, double quotes, and non-printable ASCII characters. */
<QUOTED_STRING>"\\\\"                 |
<QUOTED_STRING>[^\\\n"\x0-\x1f\x7f]+ {
    StringTokPtr str = StringTokPtr::dynamicCast(*yylvalp);
    str->v += yytext;
    str->literal += yytext;
}

  /* Matches a single escaped double-quote, single-quote, or question mark. */
<QUOTED_STRING>"\\\"" |
<QUOTED_STRING>"\\\'" |
<QUOTED_STRING>"\\?"  {
    StringTokPtr str = StringTokPtr::dynamicCast(*yylvalp);
    str->v += yytext[1];
    str->literal += yytext[1];
}

  /* Matches a single ANSI-C escape code pattern. */
<QUOTED_STRING>"\\"[abfnrtv] {
    StringTokPtr str = StringTokPtr::dynamicCast(*yylvalp);
    char ansiCode;
    switch(yytext[1])
    {
        case 'a': ansiCode = '\a'; break;
        case 'b': ansiCode = '\b'; break;
        case 'f': ansiCode = '\f'; break;
        case 'n': ansiCode = '\n'; break;
        case 'r': ansiCode = '\r'; break;
        case 't': ansiCode = '\t'; break;
        case 'v': ansiCode = '\v'; break;
    }
    str->v += ansiCode;
    str->literal += yytext;
}

  /* Matches a single escaped octal value. Octal literals are limited to a max of 3 digits. */
<QUOTED_STRING>"\\"{oct}{1,3} {
    auto value = strtoull((yytext + 1), 0, 8);
    if(value > 255)
    {
        unit->error("octal escape sequence out of range: `\\" + string(yytext + 1) + "'");
    }

    StringTokPtr str = StringTokPtr::dynamicCast(*yylvalp);
    str->v += static_cast<char>(value);
    str->literal += yytext;
}

  /* Matches a single escaped hexadecimal value. Hexadecimal literals are limited to a max of 2 digits. */
<QUOTED_STRING>"\\x"{hex}{1,2} {
    auto value = strtoull((yytext + 2), 0, 16);
    assert(value >= 0 && value <= 255);

    StringTokPtr str = StringTokPtr::dynamicCast(*yylvalp);
    str->v += static_cast<char>(value);
    str->literal += yytext;
}

  /* Matches an empty hexadecimal escape value. */
<QUOTED_STRING>"\\x" {
    unit->error("no hex digit in hex escape sequence");
    StringTokPtr str = StringTokPtr::dynamicCast(*yylvalp);
    str->literal += yytext;
}

  /* Matches a single 4-char or 8-char size universal character code. */
<QUOTED_STRING>"\\u"{hex}{4} |
<QUOTED_STRING>"\\U"{hex}{8} {
    auto codePoint = strtoull((yytext + 2), 0, 16);
    if(codePoint <= 0xdfff && codePoint >= 0xd800)
    {
        unit->error("a universal character name cannot designate a surrogate: `" + string(yytext) + "'");
    }
    StringTokPtr str = StringTokPtr::dynamicCast(*yylvalp);
    str->v += yytext;
    str->literal += yytext;
}

  /* Matches a universal character code that isn't the correct size, or uses incorrect characters. */
<QUOTED_STRING>"\\u"{hex}{0,3}[g-zG-Z]* |
<QUOTED_STRING>"\\U"{hex}{0,7}[g-zG-Z]* {
    unit->error("unknown escape sequence in string literal: `" + string(yytext) + "'");
    StringTokPtr str = StringTokPtr::dynamicCast(*yylvalp);
    str->v += yytext;
    str->literal += yytext;
}

  /* Matches an escaped newline to allow for multi-line strings. The scanner consumes the newline. */
<QUOTED_STRING>"\\"\n {
    unit->nextLine();
}

  /* Matches an unescaped newline, and issues an error. */
<QUOTED_STRING>\n {
    unit->error("encountered un-escaped EOL while scanning a string literal.");
    unit->nextLine();
}

  /* Matches an unknown escape value. This rule has a lower priority than all the other escape rules because
   * it only matches 2 characters (the lowest any match), and it's beneath the others. */
<QUOTED_STRING>"\\". {
    unit->warning(All, "unknown escape sequence in string literal: `" + string(yytext) + "'");

    StringTokPtr str = StringTokPtr::dynamicCast(*yylvalp);
    // Escape the entire sequence.
    str->v += "\\" + string(yytext);
    str->literal += yytext;
}

  /* Matches the end of a double-quoted string literal, but only while scanning a string literal. Flex always prefers
   * to match the longest string it can, so quotes preceeded with a literal '\' will match the rules above this one. */
<QUOTED_STRING>\" {
    yy_pop_state();
    StringTokPtr str = StringTokPtr::dynamicCast(*yylvalp);
    str->literal += yytext;
    return ICE_STRING_LITERAL;
}

  /* Matches EOF, but only while scanning a string literal. */
<QUOTED_STRING><<EOF>> {
    yy_pop_state();
    unit->error("encountered EOF while scanning a string literal");
    return ICE_STRING_LITERAL;
}

{integer_constant} {
    IntegerTokPtr itp = new IntegerTok;
    itp->literal = string(yytext);
    *yylvalp = itp;
    if(!IceUtilInternal::stringToInt64(string(yytext), itp->v))
    {
        assert(itp->v != 0);
        unit->error("integer constant `" + string(yytext) + "' out of range");
    }
    return ICE_INTEGER_LITERAL;
}

{floating_literal} {
    errno = 0;
    FloatingTokPtr ftp = new FloatingTok;
    *yylvalp = ftp;
    string literal(yytext);
    ftp->literal = literal;
    char lastChar = literal[literal.size() - 1];
    if(lastChar == 'f' || lastChar == 'F')
    {
        literal = literal.substr(0, literal.size() - 1);    // Clobber trailing 'f' or 'F' suffix
    }
    ftp->v = strtod(literal.c_str(), 0);
    if((ftp->v == HUGE_VAL || ftp->v == -HUGE_VAL) && errno == ERANGE)
    {
        unit->error("floating-point constant `" + string(yytext) + "' too large (overflow)");
    }
    else if(ftp->v == 0 && errno == ERANGE)
    {
        unit->error("floating-point constant `" + string(yytext) + "' too small (underflow)");
    }
    return ICE_FLOATING_POINT_LITERAL;
}

  /* ========== Whitespace and Others ========== */

  /* Ignore and consume new-line characters. */
\n {
    unit->nextLine();
}

  /* Ignore and consume UTF-8 BOMs, but only when the scanner has just started scanning a new file. */
<INITIAL>{bom} {}

  /* Ignore and consume any whitespace, other than new-lines. This rule is active in any start-condition */
<*>([[:space:]]{-}[\n])+ {}

  /* Matches a single non-printable ASCII character. This rule is active in any start-condition,
   * but is the second lowest priority rule. */
<*>[\x0-\x1f\x7f] {
    stringstream s;
    s << "illegal input character: '\\";
    s.width(3);
    s.fill('0');
    s << oct << static_cast<int>(static_cast<unsigned char>(yytext[0]));
    s << "'";
    unit->error(s.str());
    return BAD_CHAR;
}

  /* Matches any character (except for new-lines) not matched by another rule and forwards it to the parser.
   * This is the lowest priority rule. */
. {
    return yytext[0];
}

%%

namespace Slice {

// This function is always called once, right before scanning begins.
void initScanner()
{
    keywordMap = {
        {"module", ICE_MODULE},
        {"class", ICE_CLASS},
        {"interface", ICE_INTERFACE},
        {"exception", ICE_EXCEPTION},
        {"struct", ICE_STRUCT},
        {"sequence", ICE_SEQUENCE},
        {"dictionary", ICE_DICTIONARY},
        {"enum", ICE_ENUM},
        {"out", ICE_OUT},
        {"extends", ICE_EXTENDS},
        {"implements", ICE_IMPLEMENTS},
        {"throws", ICE_THROWS},
        {"void", ICE_VOID},
        {"byte", ICE_BYTE},
        {"bool", ICE_BOOL},
        {"short", ICE_SHORT},
        {"int", ICE_INT},
        {"long", ICE_LONG},
        {"float", ICE_FLOAT},
        {"double", ICE_DOUBLE},
        {"string", ICE_STRING},
        {"Object", ICE_OBJECT},
        {"const", ICE_CONST},
        {"false", ICE_FALSE},
        {"true", ICE_TRUE},
        {"idempotent", ICE_IDEMPOTENT},
        {"tag", ICE_TAG},
        // 'optional' is kept as an alias for 'tag' for backwards compatability.
        // We need a separate token type since we infer 'optional T' to mean 'tag T?'.
        // But for 'tag' we require an optional (nullable) type. No inferencing is done.
        {"optional", ICE_OPTIONAL},
        {"Value", ICE_VALUE}
    };
}

// This function is always called diectly after a match has been made, but directly before it's action block is run.
void preAction()
{
    // We only use the 'INITIAL' state to consume BOMs, which can only validly be the first match in a file. This
    // function being called means a match has already been made, so we switch states since BOMs are no longer valid.
    if(YY_START == INITIAL)
    {
        BEGIN(SLICE);
    }
}

// Check if an identifier looks like a keyword.
// If the identifier is a keyword, return the
// corresponding keyword token; otherwise, return
// an identifier token.
int checkKeyword(string& id)
{
    const auto pos = keywordMap.find(id);
    if(pos != keywordMap.end())
    {
        if(pos->first != id)
        {
            unit->error("illegal identifier: `" + id + "' differs from keyword `" + pos->first +
                        "' only in capitalization");
            id = pos->first;
        }
        return pos->second;
    }
    return checkIdentifier(id);
}

// Checks an identifier for any illegal syntax and
// determines whether it's scoped. If it is, this
// returns a scoped identifier token; otherwise this
// returns a normal identifier token.
int checkIdentifier(string& id)
{
    // check whether the identifier is scoped
    size_t scopeIndex = id.rfind("::");
    bool isScoped = scopeIndex != string::npos;
    string name;
    if(isScoped)
    {
        name = id.substr(scopeIndex + 2); // Only check the unscoped identifier for syntax
    }
    else
    {
        name = id;
    }

    // check the identifier for reserved suffixes
    static const string suffixBlacklist[] = { "Helper", "Holder", "Prx", "Ptr" };
    for(size_t i = 0; i < sizeof(suffixBlacklist) / sizeof(*suffixBlacklist); ++i)
    {
        if(name.find(suffixBlacklist[i], name.size() - suffixBlacklist[i].size()) != string::npos)
        {
            unit->error("illegal identifier `" + name + "': `" + suffixBlacklist[i] + "' suffix is reserved");
        }
    }

    // check the identifier for illegal underscores
    size_t index = name.find('_');
    if(index == 0)
    {
        unit->error("illegal leading underscore in identifier `" + name + "'");
    }
    else if(name.rfind('_') == (name.size() - 1))
    {
        unit->error("illegal trailing underscore in identifier `" + name + "'");
    }
    else if(name.find("__") != string::npos)
    {
        unit->error("illegal double underscore in identifier `" + name + "'");
    }
    else if(index != string::npos && unit->currentIncludeLevel() == 0 && !unit->allowUnderscore())
    {
        DefinitionContextPtr dc = unit->currentDefinitionContext();
        assert(dc);
        if(dc->findMetaData("underscore") != "underscore") // no 'underscore' global metadata
        {
            unit->error("illegal underscore in identifier `" + name + "'");
        }
    }

    // Check the identifier for illegal ice prefixes
    if(unit->currentIncludeLevel() == 0 && !unit->allowIcePrefix() && name.size() > 2)
    {
        DefinitionContextPtr dc = unit->currentDefinitionContext();
        assert(dc);
        if(dc->findMetaData("ice-prefix") != "ice-prefix") // no 'ice-prefix' metadata
        {
            string prefix3;
            prefix3 += ::tolower(static_cast<unsigned char>(name[0]));
            prefix3 += ::tolower(static_cast<unsigned char>(name[1]));
            prefix3 += ::tolower(static_cast<unsigned char>(name[2]));
            if(prefix3 == "ice")
            {
                unit->error("illegal identifier `" + name + "': `" + name.substr(0, 3) + "' prefix is reserved");
            }
        }
    }

    return isScoped ? ICE_SCOPED_IDENTIFIER : ICE_IDENTIFIER;
}

}
