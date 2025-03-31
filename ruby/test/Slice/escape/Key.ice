// Copyright (c) ZeroC, Inc.

// TODO replace this with 'ruby:identifier'.
module EscapedBEGIN
{
    ["ruby:identifier:END_"]
    enum END
    {
        ["ruby:identifier:_alias"]
        alias
    }

    ["ruby:identifier:_and"]
    struct and
    {
        int begin;
    }

    ["ruby:identifier:_next"]
    exception next
    {
        ["ruby:identifier:_new"]
        int new;
    }

    ["ruby:identifier:_nil"]
    exception nil extends next
    {
        ["ruby:identifier:_not"] int not;
        ["ruby:identifier:_or"] and or;
    }

    ["ruby:identifier:_break"]
    interface break
    {
        ["ruby:identifier:_case"]
        void case(["ruby:identifier:_clone"] int clone, out ["ruby:identifier:_def"] int def);

        ["ruby:identifier:my_operation"]
        END instance_variables();
    }

    ["ruby:identifier:_elsif"]
    interface elsif extends break
    {
        ["ruby:identifier:_for"]
        END for(
            ["ruby:identifier:_freeze"] display freeze,
            ["ruby:identifier:_if"] break* if,
            ["ruby:identifier:_methods"] int methods
        ) throws nil;
    }

    ["ruby:identifier:_display"]
    class display
    {
        ["ruby:identifier:_when"] and when;
        ["ruby:identifier:_dup"] break* dup;
        ["ruby:identifier:_else"] int else;
    }

    ["ruby:identifier:_rescue"]
    sequence<END> rescue;
    ["ruby:identifier:_ensure"]
    dictionary<string, END> ensure;

    ["ruby:identifier:_redo"]
    const int redo = 1;
}
