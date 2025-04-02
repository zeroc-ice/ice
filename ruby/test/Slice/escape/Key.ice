// Copyright (c) ZeroC, Inc.

["ruby:identifier:EscapedBEGIN"]
module BEGIN
{
    ["ruby:identifier:END_"]
    enum END
    {
        alias
    }

    ["ruby:identifier:AndStruct"]
    struct and
    {
        ["ruby:identifier:_begin"]
        int begin;
    }

    ["ruby:identifier:NextException"]
    exception next
    {
        ["ruby:identifier:_new"]
        int new;
    }

    ["ruby:identifier:NIL"]
    exception nil extends next
    {
        ["ruby:identifier:_not"] int not;
        ["ruby:identifier:_or"] and or;
    }

    ["ruby:identifier:MyBreak"]
    interface break
    {
        ["ruby:identifier:_case"]
        void case(["ruby:identifier:_clone"] int clone, out ["ruby:identifier:_def"] int def);

        ["ruby:identifier:PascalOperation"]
        END instance_variables();
    }

    ["ruby:identifier:Display"]
    class display
    {
        ["ruby:identifier:_when"] and when;
        ["ruby:identifier:_dup"] break* dup;
        ["ruby:identifier:_else"] int else;
    }

    ["ruby:identifier:Els_if_"]
    interface elsif extends break
    {
        ["ruby:identifier:_for"]
        END for(
            ["ruby:identifier:_freeze"] display freeze,
            ["ruby:identifier:_if"] break* if,
            ["ruby:identifier:_methods"] int methods
        ) throws nil;
    }

    ["ruby:identifier:_rescue"]
    sequence<END> rescue;
    ["ruby:identifier:_ensure"]
    dictionary<string, END> ensure;

    ["ruby:identifier:REDO"]
    const int redo = 1;
}
