%top{
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/ScannerConfig.h>

}

%{

#include <Slice/GrammarUtil.h>
#include <Slice/Grammar.h>
#include <IceUtil/InputUtil.h>

#include <iomanip>

#include <stdlib.h>
#include <math.h>

#ifdef _MSC_VER
#   ifdef slice_wrap
#      undef slice_wrap
#      define slice_wrap() 1
#   endif
#endif

using namespace std;
using namespace Slice;

namespace Slice
{

// Definitions for the case-insensitive keyword-token map.
map<string, int> keywordMap;

int checkKeyword(string&);
int checkIsScoped(const string&);

}

// Stores the scanner's current column position. Flex also automatically
// generates 'yylineno', which stores the scanner's current line number.
int yycolno = 0;
// Stores a copy of the filename that the scanner is currently scanning.
shared_ptr<string> yyfilename;

namespace
{

void nextLine(int = 1);
int scanPosition(const char*);
void setLocation(TokenContext*);
void startLocation(TokenContext*);
void endLocation(TokenContext*);

void initScanner();
void preAction();
void yynoreturn fatalError(const char* msg);

}

// Override some of the functions flex auto-generates with our own implementations.
#define YY_USER_INIT initScanner();
#define YY_USER_ACTION preAction();
#define YY_FATAL_ERROR(msg) fatalError(msg);

%}

  /* Changes the default prefix of 'yy' to 'slice_' for functions and variables in the generated code. */
%option prefix="slice_"
  /* Instructs flex to not suppress any warnings when generating the scanner. */
%option warn
  /* Instructs flex to generate a scanner that supports verbose outputting (debug mode). */
%option debug
  /* By default flex will 'default match' any text it encounters that doesn't match any specified rules. This
   * option disables default-matching (it throws 'scanner jammed' instead) to make grammar holes more obvious. */
%option nodefault
  /* Directs flex to generate a scanner tailored for use by bison, and that supports bison's token location mechanism.
   * These options change the signature of the main lexing function, which must match the one declared in Grammar.y */
%option bison-bridge bison-locations

  /* Enables the use of flex's built in start-condition state stack. */
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
%option noinput nounput noyywrap
%option noyy_scan_buffer noyy_scan_bytes noyy_scan_string
%option noyyget_extra noyyset_extra noyyget_leng noyyget_text
%option noyyget_in noyyset_in noyyget_out noyyset_out
%option noyyget_lineno noyyset_lineno noyyget_lloc noyyset_lloc
%option noyyget_lval noyyset_lval noyyget_debug noyyset_debug

  /* List of start-condition states the scanner can be in. This lets the scanning be context dependent. */
%x C_COMMENT
%s PRE_SLICE
%s SLICE
%x PREPROCESS
%x METADATA
%x QUOTED_METADATA
%x UNQUOTED_METADATA
%x STRING_LITERAL
  /* The scanner also has a built in 'INITIAL' start-condition state, which is the state the scanner is initialized in.
   * We use it solely to check for and consume any BOMs at the start of files. See Bug 3140. */

oct                 [0-7]
dec                 [0-9]
hex                 [0-9a-fA-F]
bom                 "\357\273\277"
whitespace          ([[:space:]]{-}[\n])
preprocessor_prefix [[:blank:]]*#[[:blank:]]*
preprocessor_lineno {preprocessor_prefix}(line[[:blank:]]+)?{dec}+
identifier          ((::)?\\?[[:alpha:]_][[:alnum:]_]*)+
integer_constant    (\+|-)?((0{oct}+)|(0x{hex}+)|({dec}+))
fractional_constant (\+|-)?(({dec}*\.{dec}+)|({dec}+\.))
exponent_part       (e|E)(\+|-)?{dec}+
floating_literal    (({fractional_constant}{exponent_part}?)|((\+|-)?{dec}+{exponent_part}))[fF]?

%%

  /* ========== Literals ========== */
  /* Matches the start of a double-quoted string literal. */
"\"" {
    yy_push_state(STRING_LITERAL);
    startLocation(yylloc);

    StringTokPtr str = new StringTok;
    str->literal = "\"";
    *yylval = str;
}

  /* Matches a single escaped backslash, or as many characters as it can,
   * except backslashes, new-lines, double quotes, and non-printable ASCII characters. */

  /* Matches Escaped backslashes and any other valid string characters. Invalid characters are
   * new-lines, non-printable ASCII characters, and double-quotes. */
<STRING_LITERAL>"\\\\"+               |
<STRING_LITERAL>[^\\\n"\x0-\x1f\x7f]+ {
    StringTokPtr str = StringTokPtr::dynamicCast(*yylval);
    str->literal += yytext;
    str->v += yytext;
}

  /* Matches an escaped double-quote, single-quote, or question mark. */
<STRING_LITERAL>"\\\"" |
<STRING_LITERAL>"\\\'" |
<STRING_LITERAL>"\\?"  {
    StringTokPtr str = StringTokPtr::dynamicCast(*yylval);
    str->literal += yytext;
    str->v += yytext[1];
}

  /* Matches an ANSI-C escape code pattern. */
<STRING_LITERAL>"\\"[abfnrtv] {
    StringTokPtr str = StringTokPtr::dynamicCast(*yylval);
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
        default: ansiCode = '\0'; assert(false);
    }
    str->literal += yytext;
    str->v += ansiCode;
}

  /* Matches an escaped octal value. Octal literals are limited to a max of 3 digits. */
<STRING_LITERAL>"\\"{oct}{1,3} {
    auto value = strtoull((yytext + 1), 0, 8);
    if(value > 255)
    {
        unit->error("octal escape sequence out of range: `\\" + string(yytext + 1) + "'");
    }

    StringTokPtr str = StringTokPtr::dynamicCast(*yylval);
    str->literal += yytext;
    str->v += static_cast<char>(value);
}

  /* Matches an escaped hexadecimal value. Hexadecimal literals are limited to a max of 2 digits. */
<STRING_LITERAL>"\\x"{hex}{1,2} {
    auto value = strtoull((yytext + 2), 0, 16);
    assert(value <= 255);

    StringTokPtr str = StringTokPtr::dynamicCast(*yylval);
    str->literal += yytext;
    str->v += static_cast<char>(value);
}

  /* Matches an empty hexadecimal escape value. */
<STRING_LITERAL>"\\x" {
    unit->error("no hex digit in hex escape sequence");
    StringTokPtr str = StringTokPtr::dynamicCast(*yylval);
    str->literal += yytext;
}

  /* Matches a 4-char or 8-char size universal character code. */
<STRING_LITERAL>"\\u"{hex}{4} |
<STRING_LITERAL>"\\U"{hex}{8} {
    auto codePoint = strtoull((yytext + 2), 0, 16);
    if(codePoint <= 0xdfff && codePoint >= 0xd800)
    {
        unit->error("a universal character name cannot designate a surrogate: `" + string(yytext) + "'");
    }
    StringTokPtr str = StringTokPtr::dynamicCast(*yylval);
    str->literal += yytext;
    str->v += yytext;
}

  /* Matches a universal character code that isn't the correct size, or uses incorrect characters. */
<STRING_LITERAL>"\\u"{hex}{0,3}[g-zG-Z]* |
<STRING_LITERAL>"\\U"{hex}{0,7}[g-zG-Z]* {
    unit->error("unknown escape sequence in string literal: `" + string(yytext) + "'");
    StringTokPtr str = StringTokPtr::dynamicCast(*yylval);
    str->literal += yytext;
    str->v += yytext;
}

  /* Matches an unescaped newline in a string literal, and issues an error. */
<STRING_LITERAL>\n {
    yy_pop_state();
    endLocation(yylloc);
    nextLine();

    unit->error("encountered un-escaped EOL while scanning a string literal.");
    return ICE_STRING_LITERAL;
}

  /* Matches an unknown escape value. This rule has a lower priority than all the other escape rules because
   * it only matches 2 characters (the lowest any match), and it's beneath the others. */
<STRING_LITERAL>"\\". {
    unit->warning(All, "unknown escape sequence in string literal: `" + string(yytext) + "'");

    StringTokPtr str = StringTokPtr::dynamicCast(*yylval);
    // Escape the entire sequence.
    str->literal += yytext;
    str->v += "\\" + string(yytext);
}

  /* Matches a dangling backslash, with nothing to escape. This rule is mostly included for grammar completeness. */
<STRING_LITERAL>\\ {
    unit->warning(All, "dangling backslash in string literal");
    StringTokPtr str = StringTokPtr::dynamicCast(*yylval);
    str->literal += yytext;
}

  /* Matches the end of a double-quoted string literal, but only while scanning a string literal. Flex always prefers
   * to match the longest string it can, so quotes preceeded with a literal '\' will match the rules above this one. */
<STRING_LITERAL>"\"" {
    yy_pop_state();
    endLocation(yylloc);

    StringTokPtr str = StringTokPtr::dynamicCast(*yylval);
    str->literal += yytext;
    return ICE_STRING_LITERAL;
}

  /* Matches EOF, but only while scanning a string literal. */
<STRING_LITERAL><<EOF>> {
    yy_pop_state();
    endLocation(yylloc);

    unit->error("encountered EOF while scanning a string literal");
    return ICE_STRING_LITERAL;
}

{integer_constant} {
    setLocation(yylloc);

    IntegerTokPtr itp = new IntegerTok;
    itp->literal = string(yytext);
    *yylval = itp;
    if(!IceUtilInternal::stringToInt64(string(yytext), itp->v))
    {
        assert(itp->v != 0);
        unit->error("integer constant `" + string(yytext) + "' out of range");
    }
    return ICE_INTEGER_LITERAL;
}

{floating_literal} {
    setLocation(yylloc);

    errno = 0;
    FloatingTokPtr ftp = new FloatingTok;
    *yylval = ftp;
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

  /* ========== Comments ========== */

  /* Matches and records a triple-slash style doc comment. */
<*>"///".* {
    unit->addToComment(yytext + 3);
}

  /* Matches and consumes a C++ style comment. */
<*>"//".* {}

  /* Matches the start of a C style comment, and switches the scanner to the C_COMMENT state. */
<*>"/*" {
    yy_push_state(C_COMMENT);
}

  /* Matches any character except for newlines and adds them to the comments. '*' are matched one at a time to ensure
   * Flex scans '* /' correctly. Flex prioritizes longer matches over shorter ones, so '* /' will match before '*'. */
<C_COMMENT>"*"     |
<C_COMMENT>[^\n*]+ {
    yymore();
}

  /* Matches as many newlines as are available and adds them to the comment, after incrementing the line count. */
<C_COMMENT>\n+ {
    nextLine(yyleng);
    yymore();
}

  /* Matches the end of a C style comment, and reverts the scanner state to what it previously was. */
<C_COMMENT>"*/" {
    yy_pop_state();

    string comment(yytext);
    // The last 2 characters are the '*/' matched by this rule.
    unit->setComment(comment.substr(0, yyleng - 2));
}

  /* Handles reaching EOF while scanning a C style comment by issuing a warning but continuing normally. */
<C_COMMENT><<EOF>> {
    yy_pop_state();

    unit->error("encountered EOF while scanning a comment");
    unit->setComment(yytext);
}

  /* ========== Preprocessor Statements ========== */

  /* Matches the empty preprocessor directive. */
<*>^{preprocessor_prefix} {
    yy_push_state(PREPROCESS);
}

  /* Matches a line preprocessor directive, but missing a line number. */
<*>^{preprocessor_prefix}line {
    yy_push_state(PREPROCESS);
    unit->error("missing line number in line preprocessor directive");
}

  /* Matches a line preprocessor directive (optionally with a file specified afterwards). */
<*>^{preprocessor_lineno}                         |
<*>^{preprocessor_lineno}[[:blank:]]+\"[^\"\n]*\" {
    int includeAction = scanPosition(yytext);
    if(yylineno == 0 || includeAction == 1) // Push: Indicated the scanner has started scanning a new file.
    {
        yy_push_state(INITIAL);
    }
    else if(includeAction == 2) // Pop: Indicates the scanner has completed scanning a new file.
    {
        yy_pop_state();
    }
    yy_push_state(PREPROCESS);
}

  /* Matches any non white-space character. This is a catch-all to report any invalid characters
   * found while scanning a preprocessor directive. */
<PREPROCESS>[^[:space:]]+ {
    unit->error("encountered unexpected token while scanning preprocessor directive: `" + string(yytext) + "'");
}

  /* Matches a new-line character or EOF. This signals the end of the preprocessor statement. */
<PREPROCESS>\n      |
<PREPROCESS><<EOF>> {
    yy_pop_state();
    nextLine();
}

  /* ========== Metadata ========== */

"[" {
    yy_push_state(METADATA);
    return ICE_LOCAL_METADATA_OPEN;
}

"[[" {
    yy_push_state(METADATA);

    // We use a different token to indicate metadata that should be ignored (if it came after a slice definition).
    if(yy_top_state() != PRE_SLICE)
    {
        unit->error("file metadata must appear before any definitions");
    }
    return ICE_FILE_METADATA_OPEN;
}

  /* Matches the start of a metadata string, ensures the scanner is in QUOTED_METADATA mode,
   * then starts scanning a string literal. */
<METADATA,QUOTED_METADATA>"\"" {
    BEGIN(QUOTED_METADATA);
    yy_push_state(STRING_LITERAL);
    startLocation(yylloc);

    StringTokPtr str = new StringTok;
    str->literal = "\"";
    *yylval = str;
}

  /* Matches commas between string literals in quoted metadata and forwards them to the parser. */
<QUOTED_METADATA>"," {
    return yytext[0];
}

<METADATA,QUOTED_METADATA>"]" {
    yy_pop_state();
    return ICE_LOCAL_METADATA_CLOSE;
}

<METADATA,QUOTED_METADATA>"]]" {
    yy_pop_state();
    return ICE_FILE_METADATA_CLOSE;
}

  /* Matches the start of unquoted metadata and switches the scanner into UNQUOTED_METADATA mode. */
<METADATA>[^[:space:]\[\]\"] {
    BEGIN(UNQUOTED_METADATA);
    startLocation(yylloc);

    StringTokPtr str = new StringTok;
    str->literal = yytext;
    str->v = yytext;
    *yylval = str;
}

  /* Matches unquoted text in UNQUOTED_METADATA mode. */
<UNQUOTED_METADATA>({whitespace}*[^[:space:]\[\]]+)+ {
    StringTokPtr str = StringTokPtr::dynamicCast(*yylval);
    str->literal += yytext;
    str->v += yytext;
}

  /* Matches the end of unquoted metadata, and places the ']' back into the stream. */
<UNQUOTED_METADATA>"]" {
    BEGIN(METADATA);
    yyless(0);
    return ICE_STRING_LITERAL;
}

  /* Matches and consumes newlines in between metadata after incrementing the line count. */
<METADATA,QUOTED_METADATA,UNQUOTED_METADATA>\n+ {
    nextLine(yyleng);
}

  /* Matches any characters not matched by another metadata rule (except whitespace), and reports an error. */
<METADATA,QUOTED_METADATA,UNQUOTED_METADATA>[^[:space:]] {
    unit->error("invalid character between metadata");
}

  /* ========== Identifiers and Keywords ========== */

{identifier} {
    StringTokPtr ident = new StringTok;
    ident->v = *yytext == '\\' ? yytext + 1 : yytext;
    *yylval = ident;
    return *yytext == '\\' ? checkIsScoped(ident->v) : checkKeyword(ident->v);
}

  /* ========== Whitespace ========== */

  /* Matches and consumes any whitespace, except for newlines. */
<*>{whitespace}+ {}

  /* Matches and consumes newlines, but only when the scanner isn't in a sub-scanner. */
\n+ {
    nextLine(yyleng);
}

  /* ========== Others ========== */

  /* Matches and consumes a BOM, but only when the scanner has just started scanning a new file. */
<INITIAL>{bom} {}

  /* Matches invalid characters, one at a time to make this the 2nd lowest priority rule. All printable ASCII
   * characters are valid (those between 32 and 127 inclusively), anything outside this range is invalid. */
[^\x20-\x7f] {
    stringstream s;
    s << "illegal input character: '\\";
    s.width(3);
    s.fill('0');
    s << oct << static_cast<int>(static_cast<unsigned char>(yytext[0]));
    s << "'";

    unit->error(s.str());
    return BAD_CHAR;
}

  /* Matches any valid character (except newlines) not matched by another rule and fowards it to the parser.
   * This is the lowest priority rule in the scanner, and is only active while not in a sub-scanner. */
. {
    setLocation(yylloc);
    return yytext[0];
}

%%

namespace Slice
{

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
    return checkIsScoped(id);
}

// Checks if an identifier is scoped or not, and returns the corresponding token.
int checkIsScoped(const string& id)
{
    return id.find("::") == string::npos ? ICE_IDENTIFIER : ICE_SCOPED_IDENTIFIER;
}

}

namespace
{

void nextLine(int count)
{
    yylineno += count;
    yycolno = 0;
}

int scanPosition(const char* s)
{
    string line(s);
    // Skip the leading '#', optional 'line', and any whitespace before the line number.
    string::size_type idx = line.find_first_not_of(" \t\r", (line.find('#') + 1));
    if(line.find("line", idx) == idx)
    {
        idx = line.find_first_not_of(" \t\r", (idx + 4));
    }
    line.erase(0, idx);

    // Read the line number
    yylineno = stoi(line.c_str(), &idx) - 1;

    // Scan the remainder of the line for a filename.
    idx = line.find_first_not_of(" \t\r", idx);
    line.erase(0, idx);

    int lineTypeCode = 0;
    if(!line.empty())
    {
        if(line[0] == '"')
        {
            string::size_type edx = line.rfind('"');
            if(edx != string::npos)
            {
                line = line.substr(1, edx - 1);
            }
            else
            {
                unit->error("mismatched quotations in line directive");
                line = line.substr(1);
            }
        }
        lineTypeCode = unit->setCurrentFile(line, yylineno);
        yyfilename = make_shared<string>(move(line));
    }
    return lineTypeCode;
}

void setLocation(TokenContext* location)
{
    startLocation(location);
    endLocation(location);
}

void startLocation(TokenContext* location)
{
    location->firstLine = yylineno;
    // The string has already been scanned, so the scanner is positioned at the end of it.
    location->firstColumn = yycolno - yyleng;
    location->filename = yyfilename;
}

void endLocation(TokenContext* location)
{
    location->lastLine = yylineno;
    location->lastColumn = yycolno;
}

// This function is always called once, right before scanning begins.
void initScanner()
{
    // Ensure the scanner starts at line number 1, column position 0.
    yylineno = 1;

    keywordMap = {
        {"module", ICE_MODULE},
        {"class", ICE_CLASS},
        {"interface", ICE_INTERFACE},
        {"exception", ICE_EXCEPTION},
        {"struct", ICE_STRUCT},
        {"using", ICE_USING},
        {"sequence", ICE_SEQUENCE},
        {"dictionary", ICE_DICTIONARY},
        {"enum", ICE_ENUM},
        {"out", ICE_OUT},
        {"extends", ICE_EXTENDS},
        {"implements", ICE_IMPLEMENTS},
        {"throws", ICE_THROWS},
        {"void", ICE_VOID},
        {"bool", ICE_BOOL},
        {"byte", ICE_BYTE},
        {"short", ICE_SHORT},
        {"ushort", ICE_USHORT},
        {"int", ICE_INT},
        {"uint", ICE_UINT},
        {"varint", ICE_VARINT},
        {"varuint", ICE_VARUINT},
        {"long", ICE_LONG},
        {"ulong", ICE_ULONG},
        {"varlong", ICE_VARLONG},
        {"varulong", ICE_VARULONG},
        {"float", ICE_FLOAT},
        {"double", ICE_DOUBLE},
        {"string", ICE_STRING},
        {"Object", ICE_OBJECT},
        {"const", ICE_CONST},
        {"false", ICE_FALSE},
        {"true", ICE_TRUE},
        {"idempotent", ICE_IDEMPOTENT},
        {"tag", ICE_TAG},
        // 'optional' is kept as an alias for 'tag' for backwards compatibility.
        // We need a separate token type since we infer 'optional T' to mean 'tag T?'.
        // For 'tag' we require an optional type. No auto-conversion is done.
        {"optional", ICE_OPTIONAL},
        {"AnyClass", ICE_ANYCLASS},
        // 'Value' is kept as an alias for 'AnyClass' for backwards compatibility.
        {"Value", ICE_VALUE},
        {"unchecked", ICE_UNCHECKED},
        {"stream", ICE_STREAM}
    };
}

// This function is always called directly after a match has been made, but directly before it's action block is run.
void preAction()
{
    yycolno += yyleng;

    // We only use the 'INITIAL' state to consume BOMs, which can only validly be the first match in a file. This
    // function being called means a match has already been made, so we switch states since BOMs are no longer valid.
    if(YY_START == INITIAL)
    {
        BEGIN(PRE_SLICE);
    }
}

// This function is called whenever the scanner encounters an unrecoverable error.
void yynoreturn fatalError(const char* msg)
{
    cerr << *yyfilename << ":" << yylineno << ":" << yycolno << ": fatal  error: " << msg << endl
         << "\tlast matched text: `" << yytext << "'" << endl
         << "\tlast scanner state: `" << YY_START << "'" << endl;
    exit(YY_EXIT_FAILURE);
}

}
