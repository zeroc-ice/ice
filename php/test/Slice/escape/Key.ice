// Copyright (c) ZeroC, Inc.

["php:identifier:escapedAnd"]
module and
{
    ["php:identifier:another_name"]
    const int require_once = 1;

    ["php:identifier:_array"]
    enum array
    {
        ["php:identifier:_clone"]
        clone = require_once
    }

    ["php:identifier:_endfor"]
    sequence<array> endfor;
    ["php:identifier:_endforeach"]
    dictionary<string, array> endforeach;

    ["php:identifier:_xor"]
    struct xor
    {
        ["php:identifier:_abstract"] int abstract;
        ["php:identifier:_var"] int var = require_once;
    }

    ["php:identifier:_endif"]
    exception endif
    {
        ["php:identifier:_switch"] int switch;
    }

    ["php:identifier:_endwhile"]
    exception endwhile extends endif
    {
        ["php:identifier:_eval"] int eval;
        ["php:identifier:_exit"] int exit = require_once;
    }

    ["php:identifier:_function"]
    interface function
    {
        ["php:identifier:_continue"]
        void continue(["php:identifier:_declare"] int declare, ["php:identifier:_default"] int default);
    }

    ["php:identifier:_die"]
    interface die
    {
        ["php:identifier:_do"]
        void do();
    }

    ["php:identifier:_echo"]
    class echo
    {
        ["php:identifier:_if"] int if;
        ["php:identifier:_empty"] die* empty;
    }

    ["php:identifier:_enddeclare"]
    interface enddeclare extends die, function
    {
        ["php:identifier:foreach"]
        array foreach(
            ["php:identifier:_global"] echo global,
            ["php:identifier:_include"] optional(require_once) function* include,
            out ["php:identifier:_new"] endfor new,
            out ["php:identifier:_static"] optional(2) endforeach static
        ) throws endif, endwhile;
    }
}
