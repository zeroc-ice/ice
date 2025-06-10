%top{
// Copyright (c) ZeroC, Inc.

#include "../Ice/ScannerConfig.h"
#include <cstdint>

// Clang detects unreachable code in the generated scanner, so we disable the warning.
#if defined(__clang__)
#    pragma clang diagnostic ignored "-Wunreachable-code"
#endif

// NOLINTBEGIN

}

%{

#include "GrammarUtil.h"
#include "Grammar.h"

#include <iostream>
#include <cstdlib>
#include <cmath>

using namespace std;
using namespace Slice;

namespace Slice
{
    // Definitions for the case-insensitive keyword-token map.
    using StringTokenMap = map<string, int>;
    StringTokenMap keywordMap;

    int checkKeyword(string& identifier);
    int checkIsScoped(const string& identifier);
}

// Stores the scanner's current column position. Flex also automatically
// generates 'yylineno', which stores the scanner's current line number.
int yycolno = 0;
// Stores a copy of the filename that the scanner is currently scanning.
string yyfilename;

namespace
{
    void nextLine(int count = 1);
    int scanPosition(const char* s);
    void setLocation(TokenContext* location);
    void startLocation(TokenContext* location);
    void endLocation(TokenContext* location);

    void initScanner();
    void preAction();
    void pushState(int newState);
    void popState();

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
%x C_DOC_COMMENT
%s SLICE
%x PREPROCESS
%x METADATA
%x STRING_LITERAL
  /* The scanner also has a built in 'INITIAL' start-condition state, which is the state the scanner is initialized in.
   * We use it solely to check for and consume any BOMs at the start of files. See Bug 3140. */

oct                 [0-7]
dec                 [0-9]
hex                 [0-9a-fA-F]
bom                 "\357\273\277"
whitespace          ([[:space:]]{-}[\n])+
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
    pushState(STRING_LITERAL);
    startLocation(yylloc);

    StringTokPtr str = make_shared<StringTok>();
    str->literal = "\"";
    *yylval = str;
}

  /* Matches a single escaped backslash, or as many characters as it can,
   * except backslashes, new-lines, double quotes, and non-printable ASCII characters. */

  /* Matches Escaped backslashes and any other valid string characters. Invalid characters are
   * new-lines, non-printable ASCII characters, and double-quotes. */
<STRING_LITERAL>"\\\\"+               |
<STRING_LITERAL>[^\\\n"\x0-\x1f\x7f]+ {
    StringTokPtr str = dynamic_pointer_cast<StringTok>(*yylval);
    str->literal += yytext;
    str->v += yytext;
}

  /* Matches an escaped double-quote, single-quote, or question mark. */
<STRING_LITERAL>"\\\"" |
<STRING_LITERAL>"\\\'" |
<STRING_LITERAL>"\\?"  {
    StringTokPtr str = dynamic_pointer_cast<StringTok>(*yylval);
    str->literal += yytext;
    str->v += yytext[1];
}

  /* Matches an ANSI-C escape code pattern. */
<STRING_LITERAL>"\\"[abfnrtv] {
    StringTokPtr str = dynamic_pointer_cast<StringTok>(*yylval);
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
    int64_t value = std::stoll((yytext + 1), nullptr, 8);
    if (value > 255)
    {
        currentUnit->error("octal escape sequence out of range: '\\" + string(yytext + 1) + "'");
    }

    StringTokPtr str = dynamic_pointer_cast<StringTok>(*yylval);
    str->literal += yytext;
    str->v += static_cast<char>(value);
}

  /* Matches an escaped hexadecimal value. Hexadecimal literals are limited to a max of 2 digits. */
<STRING_LITERAL>"\\x"{hex}{1,2} {
    int64_t value = std::stoll((yytext + 2), nullptr, 16);
    assert(value <= 255);

    StringTokPtr str = dynamic_pointer_cast<StringTok>(*yylval);
    str->literal += yytext;
    str->v += static_cast<char>(value);
}

  /* Matches an empty hexadecimal escape value. */
<STRING_LITERAL>"\\x" {
    currentUnit->error("no hex digit in hex escape sequence");
    StringTokPtr str = dynamic_pointer_cast<StringTok>(*yylval);
    str->literal += yytext;
}

  /* Matches a 4-char or 8-char size universal character code. */
<STRING_LITERAL>"\\u"{hex}{4} |
<STRING_LITERAL>"\\U"{hex}{8} {
    int64_t codePoint = std::stoll((yytext + 2), nullptr, 16);
    if (codePoint <= 0xdfff && codePoint >= 0xd800)
    {
        currentUnit->error("a universal character name cannot designate a surrogate: '" + string(yytext) + "'");
    }
    StringTokPtr str = dynamic_pointer_cast<StringTok>(*yylval);
    str->literal += yytext;
    str->v += yytext;
}

  /* Matches a universal character code that isn't the correct size, or uses incorrect characters. */
<STRING_LITERAL>"\\u"{hex}{0,3}[g-zG-Z]* |
<STRING_LITERAL>"\\U"{hex}{0,7}[g-zG-Z]* {
    currentUnit->error("unknown escape sequence in string literal: '" + string(yytext) + "'");
    StringTokPtr str = dynamic_pointer_cast<StringTok>(*yylval);
    str->literal += yytext;
    str->v += yytext;
}

  /* Matches an unescaped newline in a string literal, and issues an error. */
<STRING_LITERAL>\n {
    popState();
    endLocation(yylloc);
    nextLine();

    currentUnit->error("encountered un-escaped EOL while scanning a string literal.");
    return ICE_STRING_LITERAL;
}

  /* Matches an unknown escape value. This rule has a lower priority than all the other escape rules because
   * it only matches 2 characters (the lowest any match), and it's beneath the others. */
<STRING_LITERAL>"\\". {
    currentUnit->warning(All, "unknown escape sequence in string literal: '" + string{yytext} + "'");

    StringTokPtr str = dynamic_pointer_cast<StringTok>(*yylval);
    // Escape the entire sequence.
    str->literal += yytext;
    str->v += "\\" + string(yytext);
}

  /* Matches a dangling backslash, with nothing to escape. This rule is mostly included for grammar completeness. */
<STRING_LITERAL>\\ {
    currentUnit->warning(All, "dangling backslash in string literal");
    StringTokPtr str = dynamic_pointer_cast<StringTok>(*yylval);
    str->literal += yytext;
}

  /* Matches the end of a double-quoted string literal, but only while scanning a string literal. Flex always prefers
   * to match the longest string it can, so quotes preceeded with a literal '\' will match the rules above this one. */
<STRING_LITERAL>"\"" {
    popState();
    endLocation(yylloc);

    StringTokPtr str = dynamic_pointer_cast<StringTok>(*yylval);
    str->literal += yytext;
    return ICE_STRING_LITERAL;
}

  /* Matches EOF, but only while scanning a string literal. */
<STRING_LITERAL><<EOF>> {
    popState();
    endLocation(yylloc);

    currentUnit->error("encountered EOF while scanning a string literal");
    return ICE_STRING_LITERAL;
}

{integer_constant} {
    setLocation(yylloc);

    IntegerTokPtr itp = make_shared<IntegerTok>();
    itp->literal = string(yytext);
    *yylval = itp;
    try
    {
        itp->v = std::stoll(string(yytext), nullptr, 0);
    }
    catch (const std::out_of_range&)
    {
        currentUnit->error("integer constant '" + string(yytext) + "' out of range");
        itp->v = INT64_MAX;
    }
    catch (const std::invalid_argument&)
    {
        currentUnit->error("invalid integer constant '" + string(yytext) + "'");
        itp->v = INT64_MAX;
    }
    return ICE_INTEGER_LITERAL;
}

{floating_literal} {
    setLocation(yylloc);

    errno = 0;
    FloatingTokPtr ftp = make_shared<FloatingTok>();
    *yylval = ftp;
    string literal(yytext);
    ftp->literal = literal;
    char lastChar = literal[literal.size() - 1];
    if (lastChar == 'f' || lastChar == 'F')
    {
        literal = literal.substr(0, literal.size() - 1);    // Clobber trailing 'f' or 'F' suffix
    }
    ftp->v = strtod(literal.c_str(), 0);
    if ((ftp->v == HUGE_VAL || ftp->v == -HUGE_VAL) && errno == ERANGE)
    {
        currentUnit->error("floating-point constant '" + string{yytext} + "' too large (overflow)");
    }
    else if (ftp->v == 0 && errno == ERANGE)
    {
        currentUnit->error("floating-point constant '" + string{yytext} + "' too small (underflow)");
    }
    return ICE_FLOATING_POINT_LITERAL;
}

  /* ========== Comments ========== */

  /* Matches and records a triple-slash style doc-comment. */
<*>"///".* {
    currentUnit->appendToDocComment(yytext + 3);
}

  /* Matches and consumes a C++ style comment. */
<*>"//".* {}

  /* Matches the start of a C style doc-comment, and switches the scanner to the C_DOC_COMMENT state.
   * Specifically we match a forward slash and 2 stars when they are _not_ followed by either a '/' or '*' character. */
<*>"/**"/[^/*] {
    pushState(C_DOC_COMMENT);
}

  /* Matches the start of a C style comment, and switches the scanner to the C_COMMENT state. */
<*>"/*" {
    pushState(C_COMMENT);
}

  /* Matches any character except for newlines and adds them to the comments. '*' are matched one at a time to ensure
   * Flex scans '* /' correctly. Flex prioritizes longer matches over shorter ones, so '* /' will match before '*'. */
<C_COMMENT,C_DOC_COMMENT>"*"     |
<C_COMMENT,C_DOC_COMMENT>[^\n*]+ {
    yymore();
}

  /* Matches as many newlines as are available and adds them to the comment, after incrementing the line count. */
<C_COMMENT,C_DOC_COMMENT>\n+ {
    nextLine(yyleng);
    yymore();
}

  /* Matches the end of a C style comment, and reverts the scanner state to what it previously was. */
<C_COMMENT,C_DOC_COMMENT>"*/" {
    string comment(yytext);
    // The last 2 characters are the '*/' matched by this rule.
    if (YY_START == C_DOC_COMMENT)
    {
        currentUnit->setDocComment(comment.substr(0, yyleng - 2));
    }

    popState();
}

  /* Handles reaching EOF while scanning a C style comment by issuing a warning but continuing normally. */
<C_COMMENT,C_DOC_COMMENT><<EOF>> {
    currentUnit->error("encountered EOF while scanning a comment");
    if (YY_START == C_DOC_COMMENT)
    {
        currentUnit->setDocComment(yytext);
    }

    popState();
}

  /* ========== Preprocessor Statements ========== */

  /* Matches the empty preprocessor directive. */
<*>^{preprocessor_prefix} {
    pushState(PREPROCESS);
}

  /* Matches a line preprocessor directive, but missing a line number. */
<*>^{preprocessor_prefix}line {
    pushState(PREPROCESS);
    currentUnit->error("missing line number in line preprocessor directive");
}

  /* Matches a line preprocessor directive (optionally with a file specified afterwards). */
<*>^{preprocessor_lineno}                         |
<*>^{preprocessor_lineno}[[:blank:]]+\"[^\"\n]*\" {
    int includeAction = scanPosition(yytext);
    if (yylineno == 0 || includeAction == 1) // Push: Indicated the scanner has started scanning a new file.
    {
        pushState(INITIAL);
    }
    else if (includeAction == 2) // Pop: Indicates the scanner has completed scanning a file.
    {
        popState();
    }
    pushState(PREPROCESS);
}

  /* Matches any non white-space character. This is a catch-all to report any invalid characters
   * found while scanning a preprocessor directive. */
<PREPROCESS>[^[:space:]]+ {
    currentUnit->error("encountered unexpected token while scanning preprocessor directive: '" + string(yytext) + "'");
}

  /* Matches a new-line character or EOF. This signals the end of the preprocessor statement. */
<PREPROCESS>\n      |
<PREPROCESS><<EOF>> {
    popState();
    nextLine();
}

  /* ========== Metadata ========== */

"[" {
    pushState(METADATA);
    return ICE_METADATA_OPEN;
}

"[[" {
    pushState(METADATA);
    return ICE_FILE_METADATA_OPEN;
}

  /* Matches the start of a metadata string, then switches the scanner into STRING_LITERAL mode. */
<METADATA>"\"" {
    pushState(STRING_LITERAL);
    startLocation(yylloc);

    StringTokPtr str = make_shared<StringTok>();
    str->literal = "\"";
    *yylval = str;
}

  /* Matches commas between string literals in quoted metadata and forwards them to the parser. */
<METADATA>"," {
    return yytext[0];
}

  /* Matches and consumes newlines in between metadata after incrementing the line count. */
<METADATA>\n+ {
    nextLine(yyleng);
}

<METADATA>"]" {
    popState();
    return ICE_METADATA_CLOSE;
}

<METADATA>"]]" {
    popState();
    return ICE_FILE_METADATA_CLOSE;
}

  /* Matches any characters not matched by another metadata rule (except whitespace), and reports an error. */
<METADATA>[^[:space:]\"\],\n]+ {
    ostringstream msg;
    msg << "invalid tokens in metadata: '" << yytext << "' (maybe you forgot to wrap it in quotes?)";
    currentUnit->error(msg.str());

    StringTokPtr str = make_shared<StringTok>();
    str->literal = yytext;
    *yylval = str;
    return BAD_TOKEN;
}

  /* ========== Identifiers and Keywords ========== */

{identifier}[[:space:]]*"(" {
    StringTokPtr ident = make_shared<StringTok>();
    ident->v = *yytext == '\\' ? yytext + 1 : yytext;
    ident->v.erase(ident->v.find_first_of(" \t\v\n\r\f("));
    *yylval = ident;
    if (*yytext == '\\')
    {
        if (checkIsScoped(ident->v) == ICE_SCOPED_IDENTIFIER)
        {
            currentUnit->error("Operation identifiers cannot be scoped: '" + (ident->v) + "'");
        }
        return ICE_IDENT_OPEN;
    }
    int st = checkKeyword(ident->v);
    if (st == ICE_IDENTIFIER)
    {
        return ICE_IDENT_OPEN;
    }
    else if (st == ICE_SCOPED_IDENTIFIER)
    {
        currentUnit->error("Operation identifiers cannot be scoped: '" + (ident->v) + "'");
        return ICE_IDENT_OPEN;
    }
    else if (st == ICE_OPTIONAL)
    {
        return ICE_OPTIONAL_OPEN;
    }
    else
    {
        return ICE_KEYWORD_OPEN;
    }
}

{identifier} {
    StringTokPtr ident = make_shared<StringTok>();
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

  /* Handles UTF-8 BOMs.
   * The scanner has a special 'INITIAL' state, which it enters whenever it hits the start of a file.
   * This occurs when parsing initially begins, but also when we see the start of an '#include'd file.
   * As soon as the scanner sees a token, it will transition into a more specific state: SLICE, METADATA, etc.
   * depending on what token it sees.
   *
   * Since BOMs are only allowed at the beginning of files, we only allow them in this 'INITIAL' state.
   * If we see a BOM in any other state, we emit an error and throw it away. */
<INITIAL>{bom} { /* do nothing */ }
<SLICE,PREPROCESS,METADATA>{bom} {
    currentUnit->error("encountered unexpected UTF-8 BOM in input; BOMs can only appear at the beginning of files");
}

  /* Matches invalid characters, one at a time to make this the 2nd lowest priority rule. All printable ASCII
   * characters are valid (those between 32 and 127 inclusively), anything outside this range is invalid. */
[^\x20-\x7f] {
    stringstream s;
    s << "illegal input character: '\\";
    s.width(3);
    s.fill('0');
    s << oct << static_cast<int>(static_cast<unsigned char>(yytext[0]));
    s << "'";

    currentUnit->error(s.str());
    return BAD_TOKEN;
}

  /* Matches any valid character (except newlines) not matched by another rule and fowards it to the parser.
   * This is the lowest priority rule in the scanner, and is only active while not in a sub-scanner. */
. {
    setLocation(yylloc);
    return yytext[0];
}

%%

// NOLINTEND

namespace Slice
{
    // Check if an identifier looks like a keyword.
    // If the identifier is a keyword, return the
    // corresponding keyword token; otherwise, return
    // an identifier token.
    int checkKeyword(string& identifier)
    {
        const auto pos = keywordMap.find(identifier);
        if (pos != keywordMap.end())
        {
            if (pos->first != identifier)
            {
                currentUnit->error(
                    "illegal identifier: '" + identifier + "' differs from keyword '" + pos->first +
                    "' only in capitalization");
                identifier = pos->first;
            }
            return pos->second;
        }
        return checkIsScoped(identifier);
    }

    // Checks if an identifier is scoped or not, and returns the corresponding token.
    int checkIsScoped(const string& identifier)
    {
        return identifier.find("::") == string::npos ? ICE_IDENTIFIER : ICE_SCOPED_IDENTIFIER;
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
        if (line.find("line", idx) == idx)
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
        if (!line.empty())
        {
            if (line[0] == '"')
            {
                string::size_type edx = line.rfind('"');
                if (edx != string::npos)
                {
                    line = line.substr(1, edx - 1);
                }
                else
                {
                    currentUnit->error("mismatched quotations in line directive");
                    line = line.substr(1);
                }
            }
            lineTypeCode = currentUnit->setCurrentFile(line, yylineno);
            yyfilename = string(line);
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

        keywordMap["module"] = ICE_MODULE;
        keywordMap["class"] = ICE_CLASS;
        keywordMap["interface"] = ICE_INTERFACE;
        keywordMap["exception"] = ICE_EXCEPTION;
        keywordMap["struct"] = ICE_STRUCT;
        keywordMap["sequence"] = ICE_SEQUENCE;
        keywordMap["dictionary"] = ICE_DICTIONARY;
        keywordMap["enum"] = ICE_ENUM;
        keywordMap["out"] = ICE_OUT;
        keywordMap["extends"] = ICE_EXTENDS;
        keywordMap["throws"] = ICE_THROWS;
        keywordMap["void"] = ICE_VOID;
        keywordMap["byte"] = ICE_BYTE;
        keywordMap["bool"] = ICE_BOOL;
        keywordMap["short"] = ICE_SHORT;
        keywordMap["int"] = ICE_INT;
        keywordMap["long"] = ICE_LONG;
        keywordMap["float"] = ICE_FLOAT;
        keywordMap["double"] = ICE_DOUBLE;
        keywordMap["string"] = ICE_STRING;
        keywordMap["Object"] = ICE_OBJECT;
        keywordMap["const"] = ICE_CONST;
        keywordMap["false"] = ICE_FALSE;
        keywordMap["true"] = ICE_TRUE;
        keywordMap["idempotent"] = ICE_IDEMPOTENT;
        keywordMap["optional"] = ICE_OPTIONAL;
        keywordMap["Value"] = ICE_VALUE;
    }

    // This function is always called directly after a match has been made, but directly before it's action block is run.
    void preAction()
    {
        yycolno += yyleng;

        // We only use the 'INITIAL' state to consume BOMs, which can only validly be the first match in a file. This
        // function being called means a match has already been made, so we switch states since BOMs are no longer valid.
        if (YY_START == INITIAL)
        {
            BEGIN(SLICE);
        }
    }

    void printStateStack()
    {
        (void)fprintf(stderr, "current state stack [ " );
        for (int i = 0; i < yy_start_stack_ptr; i++)
        {
            (void)fprintf(stderr, "%d ", yy_start_stack[i]);
        }
        (void)fprintf(stderr, "(%d) ]\n", YY_START);
    }

    void pushState(int newState)
    {
        yy_push_state(newState);
        if (yy_flex_debug)
        {
            (void)fprintf(stderr, "--pushed: ");
            printStateStack();
        }
    }

    void popState()
    {
        yy_pop_state();
        if (yy_flex_debug)
        {
            (void)fprintf(stderr, "--popped: ");
            printStateStack();
        }
    }

    // This function is called whenever the scanner encounters an unrecoverable error.
    void yynoreturn fatalError(const char* msg)
    {
        cerr << yyfilename << ":" << yylineno << ":" << yycolno << ": fatal error: " << msg << endl
             << "\tlast matched text: '" << yytext << "'" << endl
             << "\tlast scanner state: '" << YY_START << "'" << endl;
        exit(YY_EXIT_FAILURE);
    }
}
