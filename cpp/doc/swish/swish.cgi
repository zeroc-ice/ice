#!/usr/bin/perl -w
package SwishSearch;
use strict;

# This is set to where Swish-e's "make install" installed the helper modules.
use lib ( '/usr/local/lib/swish-e/perl' );


my $DEFAULT_CONFIG_FILE = '.swishcgi.conf';

###################################################################################
#
#    If this text is displayed on your browser then your web server
#    is not configured to run .cgi programs.  Contact your web server administrator.
#
#    To display documentation for this program type "perldoc swish.cgi"
#
#    swish.cgi $Revision$ Copyright (C) 2001 Bill Moseley swishscript@hank.org
#    Example CGI program for searching with SWISH-E
#
#    This example program will only run under an OS that supports fork().
#    Under windows it uses a piped open which MAY NOT BE SECURE.
#
#
#    This program is free software; you can redistribute it and/or
#    modify it under the terms of the GNU General Public License
#    as published by the Free Software Foundation; either version
#    2 of the License, or (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    The above lines must remain at the top of this program
#
#    $Id$
#
####################################################################################

# This is written this way so the script can be used as a CGI script or a mod_perl
# module without any code changes.

# use CGI ();  # might not be needed if using Apache::Request


#=================================================================================
#   CGI entry point
#
#=================================================================================


use vars '$speedy_config';  # Global for caching in persistent environment such as SpeedyCGI

# Run the script -- entry point if running as a CGI script

    unless ( $ENV{MOD_PERL} ) {
        if ( !$speedy_config ) {
            $speedy_config = default_config();

            # Merge with disk config file.
            $speedy_config = merge_read_config( $speedy_config );
        }

        process_request( $speedy_config );
    }




#==================================================================================
#   This sets the default configuration parameters
#
#   Any configuration read from disk is merged with these settings.
#
#   Only a few settings are actually required.  Some reasonable defaults are used
#   for most.  If fact, you can probably create a complete config as:
#
#    return = {
#        swish_binary    => '/usr/local/bin/swish-e',
#        swish_index     => '/usr/local/share/swish/index.swish-e',
#        title_property  => 'swishtitle',  # Not required, but recommended
#    };
#
#   But, that doesn't really show all the options.
#
#   You can modify the options below, or you can use a config file.  The config file
#   is .swishcgi.conf by default (read from the current directory) that must return
#   a hash reference.  For example, to create a config file that changes the default
#   title and index file name, plus uses Template::Toolkit to generate output
#   create a config file as:
#
#       # Example config file -- returns a hash reference
#       return {
#           title           => 'Search Our Site',
#           swish_index     => 'index.web',
#
#           template => {
#               package         => 'SWISH::TemplateToolkit',
#               file            => 'swish.tt',
#               options         => {
#                   INCLUDE_PATH    => '/home/user/swish-e/example',
#               },
#           },
#        };
#
#
#-----------------------------------------------------------------------------------

sub default_config {



    ##### Configuration Parameters #########

    #---- This lists all the options, with many commented out ---
    # By default, this config is used -- see the process_request() call below.

    # You should adjust for your site, and how your swish index was created.

    ##>>
    ##>>  Please don't post this entire section on the swish-e list if looking for help!
    ##>>
    ##>>  Send a small example, without all the comments.

    #======================================================================
    #                   *** NOTES ****
    #   Items beginning with an "x" or "#" are commented out
    #   the "x" form simply renames (hides) that setting.  It's used
    #   to make it easy to disable a mult-line configuation setting.
    #
    #   If you do not understand a setting then best to leave the default.
    #
    #   Please follow the documentation (perldoc swish.cgi) and set up
    #   a test using the defaults before making changes.  It's much easier
    #   to modify a working example than to try to get a modified example to work...
    #
    #   Again, this is a Perl hash structure.  Commas are important.
    #======================================================================


    return {
        title           => 'Search our site',  # Title of your choice.  Displays on the search page
        swish_binary    => '/usr/local/bin/swish-e',  # Location of swish-e binary


        # By default, this script tries to read a config file.  You should probably
        # comment this out if not used save a disk stat
        config_file     => $DEFAULT_CONFIG_FILE,    # Default config file


        # The location of your index file.  Typically, this would not be in
        # your web tree.
        # If you have more than one index to search then specify an array
        # reference.  e.g. swish_index =>[ qw( index1 index2 index3 )],

        swish_index     => 'index.swish-e',    # Location of your index file
                                               # See "select_indexes" below for how to
                                               # select more than one index.

        page_size       => 15,                 # Number of results per page  - default 15




        # prepend this path to the filename (swishdocpath) returned by swish.  This is used to
        # make the href link back to the original document.  Comment out to disable.

        #prepend_path    => 'http://localhost/mydocs',



        # This is the property that is used for the href link back to the original
        # document.  It's "swishdocpath" by default

        #link_property   => 'swishdocpath',


        ## Display properties ##

        # Everything swish records about a file is called a "property".  These
        # next three settings tell the swish.cgi script which properties should be passed
        # to the templating coded for output generation.


        # First is the property name to use as the main link text to the indexed document.
        # Typically, this will be 'swishtitle' if have indexed html documents,
        # but you can specify any PropertyName defined in your document.
        # By default, swish will display the pathname for documents that do not
        # have a title.
        # In other words, this is used for the text of the links of the search results.
        #  <a href="prepend_path/swishdocpath">title_property</a>

        title_property => 'swishtitle',



        # Swish has a configuration directive "StoreDescription" that will save part or
        # all of a document's contents in the index file.  This can then be displayed
        # along with results.  If you are indexing a lot of files this can use a lot of disk
        # space, so test carefully before indexing your entire site.
        # Building swish with zlib can greatly reduce the space used by StoreDescription.
        #
        # This settings tells this script to display this property as the description.
        # Normally, this should be 'swishdescription', but you can specify another property name.
        # There is no default.

        description_prop    => 'swishdescription',



        # Property names listed here will be displayed in a table below each result
        # You may wish to modify this list if you are using document properties (PropertyNames)
        # in your swish-e index configuration
        # There is no default.

        display_props   => [qw/swishlastmodified swishdocsize swishdocpath/],





        # Results can be be sorted by any of the properties listed here
        # They will be displayed in a drop-down list on the form.
        # You may modify this list if you are using document properties of your own creation
        # Swish uses the rank as the default sort

        sorts           => [qw/swishrank swishlastmodified swishtitle swishdocpath/],


        # Secondary_sort is used to sort within a sort
        # You may enter a property name followed by a direction (asc|desc)

        secondary_sort  => [qw/swishlastmodified desc/],




        # You can limit by MetaNames here.  Names listed here will be displayed in
        # a line of radio buttons.
        # The default is to not allow any metaname selection.
        # To use this feature you must define MetaNames while indexing.

        # The special "swishdefault" says to search any text that was not indexed
        # as a specific metaname (e.g. typically the body of a HTML document and its title).

        # To see how this might work, add to your *swish-e* config file:
        #   MetaNames swishtitle swishdocpath
        # reindex and try:

        metanames       => [qw/ swishdefault swishtitle swishdocpath /],

        # Add "all" to this list to test the meta_groups feature described below



        # Another example: if you indexed an email archive
        # that defined the metanames subject name email (as in the swish-e discussion archive)
        # you might use:
        #metanames       => [qw/body subject name email/],


        # Searching multiple meta names:

        # You can also group metanames into "meta-metanames".
        # Example: Say you defined metanames "author", "comment" and "keywords"
        # You want to allow searching "author", "comment" and the document body ("swishdefault")
        # But you would also like an "all" search that searches all metanames, including "keywords":
        #
        #   metanames  => [qw/swishdefault author comment all/],
        #
        # Now, the "all" metaname is not a real metaname.  It must be expanded into its
        # individual metanames using meta_groups:
        #
        #  "meta_groups" maps a fake metaname to a list of real metanames
        #
        #   meta_groups => {
        #       all => [qw/swishdefault author comment keywords / ],
        #   },
        #
        #  swish.cgi will then take a query like
        #
        #       all=(query words)
        #
        #  and create the query
        #
        #       swishdefault=(query words) OR author=(query words) OR comment=(query words) OR keywords=(query words)
        #
        #  This is not ideal, but should work for most cases
        #  (might fail under windows since the query is passed through the shell).

        # To enable this group add "all" to the list of metanames above

        meta_groups => {
            all =>  [qw/swishdefault swishtitle swishdocpath/],
        },

        # Note that you can use other words than "all".  The script just checks if a given metaname is
        # listed in "meta_groups" and expands as needed.


        # "name_labels" is used to map MetaNames and PropertyNames to user-friendly names
        # on the CGI form.

        name_labels => {
            swishdefault        => 'Title & Body',
            swishtitle          => 'Title',
            swishrank           => 'Rank',
            swishlastmodified   => 'Last Modified Date',
            swishdocpath        => 'Document Path',
            swishdocsize        => 'Document Size',
            all                 => 'All',              # group of metanames
            subject             => 'Message Subject',  # other examples
            name                => "Poster's Name",
            email               => "Poster's Email",
            sent                => 'Message Date',
        },


        timeout         => 10,    # limit time used by swish when fetching results - DoS protection.
                                  # does not work under Windows

        max_query_length => 100,  # limit length of query string.  Swish also has a limit (default is 40)
                                  # You might want to set swish-e's limit higher, and use this to get a
                                  # somewhat more friendly message.





        max_chars       => 500,   # Limits the size of the description_prop if it is not highlighted

        # This structure defines term highlighting, and what type of highlighting to use
        # If you are using metanames in your searches and they map to properties that you
        # will display, you may need to adjust the "meta_to_prop_map".

        highlight       => {

            # Pick highlighting module -- you must make sure the module can be found
            # The highlighting modules are in the example/modules directory by default

            # Ok speed, but doesn't handle phrases or stopwords
            # Deals with stemming, and shows words in context
            # Takes into consideration WordCharacters, IgnoreFirstChars and IgnoreLastChars.
            #package         => 'SWISH::DefaultHighlight',

            # Somewhat slow, but deals with phases, stopwords, and stemming.
            # Takes into consideration WordCharacters, IgnoreFirstChars and IgnoreLastChars.
            package         => 'SWISH::PhraseHighlight',

            # Faster: phrases without regard to wordcharacter settings
            # doesn't do context display, so must match in first X words, so may not even highlight
            # doesn't handle stemming or stopwords.
            #package         => 'SWISH::SimpleHighlight',

            show_words      => 10,    # Number of "swish words" words to show around highlighted word
            max_words       => 100,   # If no words are found to highlighted then show this many words
            occurrences     => 6,     # Limit number of occurrences of highlighted words
            highlight_on   => '<b>', # HTML highlighting codes
            highlight_off  => '</b>',
            #highlight_on    => '<font style="background:#FFFF99">',
            #highlight_off   => '</font>',

            # This maps (real) search metatags to display properties.
            # e.g. if searching in "swishdefault" then highlight in the
            # swishtitle and swishdescription properties
            # Do not include "fake" metanames defined with meta_groups, just
            # list the real metanames used in your index, and the properties they
            # relate to.

            meta_to_prop_map => {
                swishdefault    => [ qw/swishtitle swishdescription/ ],
                swishtitle      => [ qw/swishtitle/ ],
                swishdocpath    => [ qw/swishdocpath/ ],
            },
        },



        # If you specify more than one index file (as an array reference) you
        # can set this allow selection of which indexes to search.
        # The default is to search all indexes specified if this is not used.
        # When used, the first index is the default index.

        # You need to specify your indexes as an array reference:
        #swish_index     => [ qw/ index.swish-e index.other index2.other index3.other index4.other / ],

        Xselect_indexes  => {
            # pick radio_group, popup_menu, or checkbox_group
            method  => 'checkbox_group',
            #method => 'radio_group',
            #method => 'popup_menu',

            columns => 3,
            # labels must match up one-to-one with elements in "swish_index"
            labels  => [ 'Main Index', 'Other Index', qw/ two three four/ ],
            description => 'Select Site: ',

            # Optional - Set the default index if none is selected
            # This needs to be an index file name listed in swish_index
            # above, not a label

            default_index => '',
        },


        # Similar to select_indexes, this adds a metaname search
        # based on a metaname.  You can use any metaname, and this will
        # add an "AND" search to limit results to a subset of your records.
        # i.e. it adds something like  'site=(foo or bar or baz)' if foo, bar, and baz were selected.

        # This really just allows you to limit existing searches by a metaname, instead of
        # selecting a metaname (with metanames option above).

        # Swish-e's ExtractPath would work well with this.  For example,
        # to allow limiting searches to specific sections of the apache docs use this
        # in your swish-e config file:
        #   ExtractPath site regex !^/usr/local/apache/htdocs/manual/([^/]+)/.+$!$1!
        #   ExtractPathDefault site other
        # which extracts the segment of the path after /manual/ and indexes that name
        # under the metaname "site".  Then searches can be limited to files with that
        # path (e.g. query would be swishdefault=foo AND site=vhosts to limit searches
        # to the virtual host section.


        Xselect_by_meta  => {
            #method      => 'radio_group',  # pick: radio_group, popup_menu, or checkbox_group
            method      => 'checkbox_group',
            #method      => 'popup_menu',
            columns     => 3,
            metaname    => 'site',     # Can't be a metaname used elsewhere!
            values      => [qw/misc mod vhosts other/],
            labels  => {
                misc    => 'General Apache docs',
                mod     => 'Apache Modules',
                vhosts  => 'Virtual hosts',
            },
            description => 'Limit search to these areas: ',
        },



        # The 'template' setting defines what generates the output
        # The default is "TemplateDefault" which is reasonably ugly,
        # but does not require installation of a separate templating system.

        # Note that some of the above options may not be available
        # for templating, as it's up to you to layout the form
        # and swish-e results in your template.

        # TemplateDefault is the default

        xtemplate => {
            package     => 'SWISH::TemplateDefault',
        },

        xtemplate => {
            package     => 'SWISH::TemplateDumper',
        },

        xtemplate => {
            package         => 'SWISH::TemplateToolkit',
            file            => 'swish.tt',
            options         => {
                INCLUDE_PATH    => '/usr/local/share/swish-e',
                #PRE_PROCESS     => 'config',
            },
        },

        xtemplate => {
            package         => 'SWISH::TemplateHTMLTemplate',
            options         => {
                filename            => 'swish.tmpl',
                path                => '/usr/local/share/swish-e',
                die_on_bad_params   => 0,
                loop_context_vars   => 1,
                cache               => 1,
            },
        },



        # The "on_intranet" setting is just a flag that can be used to say you do
        # not have an external internet connection.  It's here because the default
        # page generation includes links to images on swish-e.or and on www.w3.org.
        # If this is set to one then those images will not be shown.
        # (This only effects the default ouput module SWISH::TemplateDefault)

        on_intranet => 0,



        # Here you can hard-code debugging options.  The will help you find
        # where you made your mistake ;)
        # Using all at once will generate a lot of messages to STDERR
        # Please see the documentation before using these.
        # Typically, you will set these from the command line instead of in the configuration.

        # debug_options => 'basic, command, headers, output, summary, dump',



        # This defines the package object for reading CGI parameters
        # Defaults to CGI.  Might be useful with mod_perl.
        # request_package     => 'CGI',
        # request_package     => 'Apache::Request',


        # use_library => 1,  # set true and will use the SWISH::API module
                           # will cache based on index files when running under mod_perl


        # Minor adjustment to page display.  The page navigation normally looks like:
        #                          Page: 1 5 6 7 8 9 24
        # where the first page and last page are always displayed.  These can be disabled by
        # by setting to true values ( 1 )

        no_first_page_navigation   => 0,
        no_last_page_navigation    => 0,
        num_pages_to_show          => 12,  # number of pages to offer




        # Limit to date ranges



        # This adds in the date_range limiting options
        # You will need the DateRanges.pm module from the author to use that feature

        # Noramlly, you will want to limit by the last modified date, so specify
        # "swishlastmodified" as the property_name.  If indexing a mail archive, and, for
        # example, you store the date (a unix timestamp) as "date" then specify
        # "date" as the property_name.

        date_ranges     => {
            property_name   => 'swishlastmodified',      # property name to limit by

            # what you specify here depends on the DateRanges.pm module.
            time_periods    => [
                'All',
                'Today',
                'Yesterday',
                #'Yesterday onward',
                'This Week',
                'Last Week',
                'Last 90 Days',
                'This Month',
                'Last Month',
                #'Past',
                #'Future',
                #'Next 30 Days',
            ],

            line_break      => 0,
            default         => 'All',
            date_range      => 1,
        },


        # This is suppose to reduce the load on systems if hit with a large number
        # of requests.  Although this will limit the number of swish-e processes run
        # it will not limit the number of CGI requests.  I feel like a better solution
        # is to use mod_perl (with the SWISH::API module).
        # I also think that running /bin/ps for every is not ideal.

        # This only works on unix-based systems when running the swish-e binary.
        # It greps /swish-e/ from the output of ps and aborts if the count is < limit_procs

        # Set max number of swish-e binaries and ps command to run
        limit_procs     => 0,  # max number of swish process to run (zero to not limit)
        ps_prog         => '/bin/ps -Unobody -ocommand',  # command to list number of swish binaries

    };

}

#^^^^^^^^^^^^^^^^^^^^^^^^^ end of user config ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#========================================================================================



#=================================================================================
#   mod_perl entry point
#
#   As an example, you might use a PerlSetVar to point to paths to different
#   config files, and then cache the different configurations by path.
#
#=================================================================================

my %cached_configs;

sub handler {
    my $r = shift;

    if ( my $config_path = $r->dir_config( 'Swish_Conf_File' ) ) {

        # Already cached?
        # Note that this is cached for the life of the server -- must restart if want to change config

        if ( $cached_configs{ $config_path } ) {
            process_request( $cached_configs{ $config_path } );
            return Apache::Constants::OK();
        }


        # Else, load config
        my $config = default_config();
        $config->{config_file} = $config_path;

        # Merge with disk config file.
        $cached_configs{ $config_path } = merge_read_config( $config );

        process_request( $cached_configs{ $config_path } );
        return Apache::Constants::OK();
    }


    # Otherwise, use hard-coded config
    my $config = default_config();

    # Merge with disk config file.
    $config = merge_read_config( $config );

    process_request( default_config() );

    return Apache::Constants::OK();

}


#============================================================================
#   Read config settings from disk, and merge
#   Note, all errors are ignored since by default this script looks for a
#   config file.
#
#============================================================================
sub merge_read_config {
    my $config = shift;


    set_default_debug_flags();

    set_debug($config);  # get from config or from %ENV


    return $config unless $config->{config_file};

    my $return = do $config->{config_file};  # load the config file

    unless ( ref $return eq 'HASH' ) {

        # First, let's check for file not found for the default config, which we can ignore

        my $error = $@ || $!;

        if ( $config->{config_file} eq $DEFAULT_CONFIG_FILE && !-e $config->{config_file} ) {
            warn "Config file '$config->{config_file}': $!" if $config->{debug};
            return $config;
        }

        die "Config file '$config->{config_file}': $error";
    }



    if ( $config->{debug} || $return->{debug} ) {
        require Data::Dumper;
        print STDERR "\n---------- Read config parameters from '$config->{config_file}' ------\n",
              Data::Dumper::Dumper($return),
              "-------------------------\n";
    }

    set_debug( $return );


    # Merge settings
    return { %$config, %$return };
}

#--------------------------------------------------------------------------------------------------
sub set_default_debug_flags {
    # Debug flags defined

    $SwishSearch::DEBUG_BASIC       = 1;  # Show command used to run swish
    $SwishSearch::DEBUG_COMMAND     = 2;  # Show command used to run swish
    $SwishSearch::DEBUG_HEADERS     = 4;  # Swish output headers
    $SwishSearch::DEBUG_OUTPUT      = 8;  # Swish output besides headers
    $SwishSearch::DEBUG_SUMMARY     = 16;  # Summary of results parsed
    $SwishSearch::DEBUG_RESULTS     = 32;  # Detail of results parsed
    $SwishSearch::DEBUG_DUMP_DATA   = 64;  # dump data that is sent to templating modules
}




#---------------------------------------------------------------------------------------------------
sub set_debug {
    my $conf = shift;

    $conf->{debug} = 0;

    my $debug_string =  $ENV{SWISH_DEBUG} ||$conf->{debug_options};
    return unless $debug_string;


    my %debug = (
        basic       => [$SwishSearch::DEBUG_BASIC,   'Basic debugging'],
        command     => [$SwishSearch::DEBUG_COMMAND, 'Show command used to run swish'],
        headers     => [$SwishSearch::DEBUG_HEADERS, 'Show headers returned from swish'],
        output      => [$SwishSearch::DEBUG_OUTPUT,  'Show output from swish'],
        summary     => [$SwishSearch::DEBUG_SUMMARY, 'Show summary of results'],
        results     => [$SwishSearch::DEBUG_RESULTS, 'Show detail of results'],
        dump        => [$SwishSearch::DEBUG_DUMP_DATA, 'Show all data available to templates'],
    );


    $conf->{debug} = 1;

    my @debug_str;

    for ( split /\s*,\s*/, $debug_string ) {
        if ( exists $debug{ lc $_ } ) {
            push @debug_str, lc $_;
            $conf->{debug} |= $debug{ lc $_ }->[0];
            next;
        }

        print STDERR "Unknown debug option '$_'.  Must be one of:\n",
             join( "\n", map { sprintf('  %10s: %10s', $_, $debug{$_}->[1]) } sort { $debug{$a}->[0] <=> $debug{$b}->[0] }keys %debug),
             "\n\n";
        exit;
    }

    print STDERR "Debug level set to: $conf->{debug} [", join( ', ', @debug_str), "]\n";
}


#============================================================================
#
#   This is the main controller (entry point), where a config hash is passed in.
#
#   Loads the request module (e.g. CGI.pm), and the output module
#   Also sets up debugging
#
#============================================================================

sub process_request {
    my $conf = shift;  # configuration parameters



    # Limit number of requests - questionable value
    limit_swish( $conf->{limit_procs}, $conf->{ps_prog} )
        if !$conf->{use_library}
          && $conf->{limit_procs} && $conf->{limit_procs} =~ /^\d+$/
          && $conf->{ps_prog};



    # Set default property used or the href link to the document
    $conf->{link_property} ||= 'swishdocpath';

    # Use CGI.pm by default
    my $request_package = $conf->{request_package} || 'CGI';

    load_module( $request_package );
    my $request_object = $request_package->new;


    # load the templating module
    my $template = $conf->{template} || { package => 'SWISH::TemplateDefault' };
    load_module( $template->{package} );


    # Allow fixup within the config file
    if ( $conf->{request_fixup} && ref $conf->{request_fixup} eq 'CODE' ) {
        &{$conf->{request_fixup}}( $request_object, $conf );
    }


    set_debug_input( $conf, $request_object )
        if $conf->{debug} && !$ENV{GATEWAY_INTERFACE};


    # Create search object and build a query based on CGI parameters
    my $search = SwishQuery->new(
        config    => $conf,
        request   => $request_object,
    );




    # run the query (run if there's a query)
    $search->run_query;  # currently, results is the just the $search object

    if ( $search->hits ) {
        $search->set_navigation;  # sets links
    }



    show_debug_output( $conf, $search )
        if $conf->{debug};


    $template->{package}->show_template( $template, $search );
}


# For limiting number of swish-e binaries

sub limit_swish {
    my ( $limit_procs, $ps_prog ) = @_;


    my $num_procs = scalar grep { /swish-e/ } `$ps_prog`;
    return if $num_procs <= $limit_procs;

    warn "swish.cgi - limited due to too many currently running swish-e binaries: $num_procs running is more than $limit_procs\n";

    ## Abort
    print <<EOF;
Status: 503 Too many requests

<html>
<head><title>Too Many Requests</title></head>
<body>
Too Many Requests -- Try again later
</body>
</html>
EOF

    exit;
}




#============================================================================
#
#   Loads a perl module -- and shows a pretty web page to say the obvious
#
#
#============================================================================
sub load_module {
    my $package = shift;
    $package =~ s[::][/]g;
    eval { require "$package.pm" };
    if ( $@ ) {
        print <<EOF;
Content-Type: text/html

<html>
<head><title>Software Error</title></head>
<body><h2>Software Error</h2><p>Please check error log</p></body>
</html>
EOF

        die "$0 $@\n";
    }
}



#==================================================================
# set debugging input
#
#==================================================================

sub set_debug_input {
    my ( $conf, $request_object ) = @_;

    print STDERR 'Enter a query [all]: ';
    my $query = <STDIN>;
    $query =~ tr/\r//d;
    chomp $query;
    unless ( $query ) {
        print STDERR "Using 'not asdfghjklzxcv' to match all records\n";
        $query = 'not asdfghjklzxcv';
    }

    $request_object->param('query', $query );

    print STDERR 'Enter max results to display [1]: ';
    my $max = <STDIN>;
    chomp $max;
    $max = 1 unless $max && $max =~/^\d+$/;

    $conf->{page_size} = $max;
}

#==================================================================
# show debugging output
#
#==================================================================
sub show_debug_output {
    my ( $conf, $results ) = @_;

    require Data::Dumper;


    if ( $results->hits ) {
        print STDERR "swish.cgi: returned a page of $results->{navigation}{showing} results of $results->{navigation}{hits} total hits\n";
    } else {
        print STDERR "swish.cgi: No results\n";
    }

    if ($conf->{debug} & $SwishSearch::DEBUG_HEADERS ) {
        print STDERR "\n------------- Index Headers ------------\n";
        if ( $results->{_headers} ) {
            print STDERR Data::Dumper::Dumper( $results->{_headers} );
        } else {
            print STDERR "No headers\n";
        }

        print STDERR "--------------------------\n";
    }



    if ( $conf->{debug} & $SwishSearch::DEBUG_DUMP_DATA ) {
        print STDERR "\n------------- Results structure passed to template ------------\n",
              Data::Dumper::Dumper( $results ),
              "--------------------------\n";

    } elsif ( $conf->{debug} & $SwishSearch::DEBUG_SUMMARY ) {
        print STDERR "\n------------- Results summary ------------\n";
        if ( $results->{hits} ) {
            print STDERR "$_->{swishrank} $_->{swishdocpath}\n" for @{ $results->{_results}};

        } else {
            print STDERR "** NO RESULTS **\n";
        }

    } elsif ( $conf->{debug} & $SwishSearch::DEBUG_RESULTS ) {
        print STDERR "\n------------- Results detail ------------\n";
        if ( $results->{hits} ) {
            print STDERR Data::Dumper::Dumper( $results->{_results} );
        } else {
            print STDERR "** NO RESULTS **\n";
        }

        print STDERR "--------------------------\n";
    }
}







#==================================================================================================
package SwishQuery;
#==================================================================================================

use Carp;
# Or use this instead -- PLEASE see perldoc CGI::Carp for details
# <opinion>CGI::Carp doesn't help that much</opinion>
#use CGI::Carp; # qw(fatalsToBrowser);

use SWISH::ParseQuery;




#--------------------------------------------------------------------------------
# new() doesn't do much, just create the object
#--------------------------------------------------------------------------------
sub new {
    my $class = shift;
    my %options = @_;

    my $conf = $options{config};

    croak "Failed to set the swish index files in config setting 'swish_index'" unless $conf->{swish_index};
    croak "Failed to specify 'swish_binary' in configuration" unless $conf->{swish_binary};

    # initialize the request search hash
    my $sh = {
       prog         => $conf->{swish_binary},
       config       => $conf,
       q            => $options{request},
       hits         => 0,
       MOD_PERL     => $ENV{MOD_PERL},
    };

    my $self = bless $sh, $class;


    # load highlight module, if requsted

    if ( my $highlight = $self->config('highlight') ) {
        $highlight->{package} ||= 'SWISH::DefaultHighlight';
        SwishSearch::load_module( $highlight->{package} );
    }


    # Fetch the swish-e query from the CGI parameters
    $self->set_query;

    return $self;
}


sub hits { shift->{hits} }

sub config {
    my ($self, $setting, $value ) = @_;

    confess "Failed to pass 'config' a setting" unless $setting;

    my $cur = $self->{config}{$setting} if exists $self->{config}{$setting};

    $self->{config}{$setting} = $value if $value;

    return $cur;
}

# Returns false if all of @values are not valid options - for checking
# $config is what $self->config returns

sub is_valid_config_option {
    my ( $self, $config, $err_msg, @values ) = @_;

    unless ( $config ) {
        $self->errstr( "No config option set: $err_msg" );
        return;
    }

    # Allow multiple values.
    my @options = ref $config eq 'ARRAY' ? @$config : ( $config );

    my %lookup = map { $_ => 1 } @options;

    for ( @values ) {
        unless ( exists $lookup{ $_ } ) {
            $self->errstr( $err_msg );
            return;
        }
    }

    return 1;
}


sub header {
    my $self = shift;
    return unless ref $self->{_headers} eq 'HASH';

    return $self->{_headers}{$_[0]} || '';
}


# return a ref to an array
sub results {
    my $self = shift;
    return $self->{_results} || undef;
}

sub navigation {
    my $self = shift;
    return unless ref $self->{navigation} eq 'HASH';

    return exists $self->{navigation}{$_[0]} ? $self->{navigation}{$_[0]} : '';
}

sub CGI { $_[0]->{q} };




sub swish_command {

    my ($self, $param_name, $value ) = @_;

    return $self->{swish_command} || {} unless $param_name;
    return $self->{swish_command}{$param_name} || '' unless $value;

    $self->{swish_command}{$param_name} = $value;
}

# For use when forking

sub swish_command_array {

    my ($self ) = @_;

    my @params;
    my $swish_command = $self->swish_command;

    for ( keys %$swish_command ) {

        my $value = $swish_command->{$_};

        if ( /^-/ ) {
            push @params, $_;
            push @params, ref $value eq 'ARRAY' ? @$value : $value;
            next;
        }

        # special cases
        if ( $_ eq 'limits' ) {
            push @params, '-L', $value->{prop}, $value->{low}, $value->{high};
            next;
        }

        die "Unknown swish_command '$_' = '$value'";
    }

    return @params;

}



sub errstr {
    my ($self, $value ) = @_;


    $self->{_errstr} = $value if $value;

    return $self->{_errstr} || '';
}


#==============================================================================
# Set query from the CGI parameters
#------------------------------------------------------------------------------

sub set_query {
    my $self = shift;
    my $q = $self->{q};

    # Sets the query string, and any -L limits.
    return unless $self->build_query;

    # Set the starting position (which is offset by one)

    my $start = $q->param('start') || 0;
    $start = 0 unless $start =~ /^\d+$/ && $start >= 0;

    $self->swish_command( '-b', $start+1 );



    # Set the max hits

    my $page_size = $self->config('page_size') || 15;
    $self->swish_command( '-m', $page_size );


    return unless $self->set_index_file;


    # Set the sort option, if any
    return unless $self->set_sort_order;


    return 1;

}





#============================================
# This returns "$self" just in case we want to seperate out into two objects later


sub run_query {

    my $self = shift;

    my $q = $self->{q};
    my $conf = $self->{config};

    return $self unless $self->swish_command('-w');

    my $time_out_str = 'Timed out';


    my $timeout = $self->config('timeout') || 0;

    eval {
        local $SIG{ALRM} = sub {
            kill 'KILL', $self->{pid} if $self->{pid};
            die $time_out_str . "\n";
        };

        alarm $timeout if $timeout && $^O !~ /Win32/i;
        $self->run_swish;
        alarm 0  unless $^O =~ /Win32/i;

        # catch zombies
        waitpid $self->{pid}, 0 if $self->{pid};  # for IPC::Open2
    };

    if ( $@ ) {
        warn "$0 aborted: $@"; # if $conf->{debug};

        $self->errstr(
            $@ =~ /$time_out_str/
            ? "Search timed out after $timeout seconds."
            : "Service currently unavailable"
        );
        return $self;
    }
}


# Build href for repeated search via GET (forward, backward links)

sub set_navigation {
    my $self = shift;
    my $q = $self->{q};


    # Single string

    # default fields
    my @std_fields = qw/query metaname sort reverse/;

    # Extra fields could be added in the config file
    if ( my $extra = $self->config('extra_fields') ) {
        push @std_fields, @$extra;
    }

    my @query_string =
         map { "$_=" . $q->escape( $q->param($_) ) }
            grep { $q->param($_) }  @std_fields;



    # Perhaps arrays

    for my $p ( qw/si sbm/ ) {
        my @settings = $q->param($p);
        next unless @settings;
        push @query_string,  "$p=" . $q->escape( $_ ) for @settings;
    }




    if ( $self->config('date_ranges' ) ) {
        my $dr = SWISH::DateRanges::GetDateRangeArgs( $q );
        push @query_string, $dr, if $dr;
    }


    $self->{query_href} = $q->script_name . '?' . join '&amp;', @query_string;
    $self->{my_url} = $q->script_name;


    my $hits = $self->hits;

    my $start = $self->swish_command('-b') || 1;
    $start--;

    $self->{navigation}  = {
            showing     => $hits,
            from        => $start + 1,
            to          => $start + $hits,
            hits        => $self->header('number of hits') ||  0,
            run_time    => $self->header('run time') ||  'unknown',
            search_time => $self->header('search time') ||  'unknown',
    };



    $self->set_page ( $self->swish_command( '-m' ) );

    return $self;

}


#============================================================
# Build a query string from swish
# Just builds the -w string
#------------------------------------------------------------

sub build_query {
    my $self = shift;

    my $q = $self->{q};


    # set up the query string to pass to swish.
    my $query = $q->param('query') || '';

    for ( $query ) {  # trim the query string
        s/\s+$//;
        s/^\s+//;
    }

    $self->{query_simple} = $query;    # without metaname
    $q->param('query', $query );  # clean up the query, if needed.


    # Read in the date limits, if any.  This can create a new query, which is why it is here
    return unless $self->get_date_limits( \$query );


    unless ( $query ) {
        $self->errstr('Please enter a query string') if $q->param('submit');
        return;
    }


    if ( length( $query ) > $self->{config}{max_query_length} ) {
        $self->errstr('Please enter a shorter query');
        return;
    }



    # Adjust the query string for metaname search
    # *Everything* is a metaname search
    # Might also like to allow searching more than one metaname at the same time

    my $metaname = $q->param('metaname') || 'swishdefault';

    return unless $self->is_valid_config_option( $self->config('metanames') || 'swishdefault', 'Bad MetaName provided', $metaname );

    # save the metaname so we know what field to highlight
    # Note that this might be a fake metaname
    $self->{metaname} = $metaname;


    # prepend metaname to query

    # expand query when using meta_groups

    my $meta_groups = $self->config('meta_groups');

    if ( $meta_groups && $meta_groups->{$metaname} ) {
        $query = join ' OR ', map { "$_=($query)" } @{$meta_groups->{$metaname}};

        # This is used to create a fake entry in the parsed query so highlighting
        # can find the query words
        $self->{real_metaname} = $meta_groups->{$metaname}[0];
    } else {
        $query = $metaname . "=($query)";
    }




    ## Look for a "limit" metaname -- perhaps used with ExtractPath
    # Here we don't worry about user supplied data

    my $limits = $self->config('select_by_meta');
    my @limits = $q->param('sbm');  # Select By Metaname


    # Note that this could be messed up by ending the query in a NOT or OR
    # Should look into doing:
    # $query = "( $query ) AND " . $limits->{metaname} . '=(' . join( ' OR ', @limits ) . ')';

    if ( @limits && ref $limits eq 'HASH' && $limits->{metaname} ) {
        $query .= ' and ' . $limits->{metaname} . '=(' . join( ' or ', @limits ) . ')';
    }


    $self->swish_command('-w', $query );

    return 1;
}

#========================================================================
#  Get the index files from the form, or from the config settings
#  Uses index numbers to hide path names
#------------------------------------------------------------------------

sub set_index_file {
    my $self = shift;

    my $q = $self->CGI;

    # Set the index file - first check for options

    my $si =  $self->config('select_indexes');
    if ( $si && ref $self->config('swish_index') eq 'ARRAY'  ) {

        my @choices = $q->param('si');

        if ( !@choices ) {

           if ( $si->{default_index} ) {
               $self->swish_command('-f', $si->{'default_index'});
               return 1;

            } else {
                $self->errstr('Please select a source to search');
                return;
            }
        }

        my @indexes = @{$self->config('swish_index')};


        my @selected_indexes = grep {/^\d+$/ && $_ >= 0 && $_ < @indexes } @choices;

        if ( !@selected_indexes ) {
            $self->errstr('Invalid source selected');
            return $self;
        }
        my %dups;
        my @idx = grep { !$dups{$_}++ } map { ref($_) ? @$_ : $_ } @indexes[ @selected_indexes ];
        $self->swish_command( '-f', \@idx );


    } else {
        $self->swish_command( '-f', $self->config('swish_index') );
    }

    return 1;
}

#================================================================================
#   Parse out the date limits from the form or from GET request
#
#---------------------------------------------------------------------------------

sub get_date_limits {

    my ( $self, $query_ref ) = @_;  # reference to query since may be modified

    my $conf = $self->{config};

    # Are date ranges enabled?
    return 1 unless $conf->{date_ranges};


    eval { require SWISH::DateRanges };
    if ( $@ ) {
        print STDERR "\n------ Can't use DateRanges feature ------------\n",
                     "\nScript will run, but you can't use the date range feature\n",
                     $@,
                     "\n--------------\n" if $conf->{debug};

        delete $conf->{date_ranges};
        return 1;
    }

    my $q = $self->{q};

    my %limits;

    unless ( SWISH::DateRanges::DateRangeParse( $q, \%limits ) ) {
        $self->errstr( $limits{dr_error} || 'Bad date range selection' );
        return;
    }

    # Store the values for later (for display on templates)

    $self->{DateRanges_time_low} = $limits{dr_time_low};
    $self->{DateRanges_time_high} = $limits{dr_time_high};


    # Allow searchs just be date if not "All dates" search
    # $$$ should place some limits here, and provide a switch to disable
    # as it can bring up a lot of results.

    $$query_ref ||= 'not skaiqwdsikdeekk'
        if $limits{dr_time_high};


    # Now specify limits, if a range was specified

    my $limit_prop = $conf->{date_ranges}{property_name} || 'swishlastmodified';


    if ( $limits{dr_time_low} && $limits{dr_time_high} ) {

        my %limits = (
            prop    => $limit_prop,
            low     => $limits{dr_time_low},
            high    => $limits{dr_time_high},
        );

        $self->swish_command( 'limits', \%limits );
    }

    return 1;
}



#================================================================
#  Set the sort order
#  Just builds the -s string
#----------------------------------------------------------------

sub set_sort_order {
    my $self = shift;

    my $q = $self->{q};

    my $sorts_array = $self->config('sorts');
    my $sortby =  $q->param('sort') || '';

    return 1 unless $sorts_array && $sortby;
    return unless $self->is_valid_config_option( $sorts_array, 'Invalid Sort Option Selected', $sortby );


    my $conf = $self->{config};


    # Now set sort option - if a valid option submitted (or you could let swish-e return the error).
    my $direction = $sortby eq 'swishrank'
        ? $q->param('reverse') ? 'asc' : 'desc'
        : $q->param('reverse') ? 'desc' : 'asc';

    my @sort_params = ( $sortby, $direction );

    if ( $conf->{secondary_sort} ) {
        my @secondary = ref $conf->{secondary_sort} ? @{ $conf->{secondary_sort} } : $conf->{secondary_sort};

        push @sort_params, @secondary
            if $sortby ne $secondary[0];
    }


    $self->swish_command( '-s', \@sort_params );


    return 1;
}



#========================================================
# Sets prev and next page links.
# Feel free to clean this code up!
#
#   Pass:
#       $results - reference to a hash (for access to the headers returned by swish)
#       $q       - CGI object
#
#   Returns:
#       Sets entries in the $results hash
#

sub set_page {

    my ( $self, $Page_Size ) = @_;

    my $q = $self->{q};
    my $config = $self->{config};

    my $navigation = $self->{navigation};


    my $start = $navigation->{from} - 1;   # Current starting record index


    # Set start number for "prev page" and the number of hits on the prev page

    my $prev = $start - $Page_Size;
    $prev = 0 if $prev < 0;

    if ( $prev < $start ) {
        $navigation->{prev} = $prev;
        $navigation->{prev_count} = $start - $prev;
    }


    my $last = $navigation->{hits} - 1;


    # Set start number for "next page" and number of hits on the next page

    my $next = $start + $Page_Size;
    $next = $last if $next > $last;
    my $cur_end   = $start + $self->{hits} - 1;
    if ( $next > $cur_end ) {
        $navigation->{next} = $next;
        $navigation->{next_count} = $next + $Page_Size > $last
                                ? $last - $next + 1
                                : $Page_Size;
    }


    # Calculate pages  ( is this -1 correct here? )
    # Build an array of a range of page numbers.

    my $total_pages = int (($navigation->{hits} -1) / $Page_Size);  # total pages for all results.

    if ( $total_pages ) {

        my @pages = 0..$total_pages;

        my $show_pages = $config->{num_pages_to_show} || 12;

        # To make the number always work
        $show_pages-- unless $config->{no_first_page_navigation};
        $show_pages-- unless $config->{no_last_page_navigation};


        # If too many pages then limit

        if ( @pages > $show_pages ) {

            my $start_page = int ( $start / $Page_Size - $show_pages/2) ;
            $start_page = 0 if $start_page < 0;

            # if close to the end then move of center
            $start_page = $total_pages - $show_pages
                if $start_page + $show_pages - 1 > $total_pages;

            @pages = $start_page..$start_page + $show_pages - 1;


            # Add first and last pages, unless config says otherwise
            unshift @pages, 0
                unless $start_page == 0 || $config->{no_first_page_navigation};

            push @pages, $total_pages
                unless $start_page + $show_pages - 1 == $total_pages || $config->{no_last_page_navigation}
        }


        # Build "canned" pages HTML

        $navigation->{pages} =
            join ' ', map {
                my $page_start = $_ * $Page_Size;
                my $page = $_ + 1;
                $page_start == $start
                ? $page
                : qq[<a href="$self->{query_href}&amp;start=$page_start">$page</a>];
                        } @pages;


        # Build just the raw data - an array of hashes
        # for custom page display with templates

        $navigation->{page_array} = [
            map {
                    {
                        page_number     => $_ + 1,  # page number to display
                        page_start      => $_ * $Page_Size,
                        cur_page        => $_ * $Page_Size == $start,  # flag
                    }
                } @pages
        ];


    }

}

#==================================================
# Format and return the date range options in HTML
#
#--------------------------------------------------
sub get_date_ranges {

    my $self = shift;

    my $q = $self->{q};
    my $conf = $self->{config};

    return '' unless $conf->{date_ranges};

    # pass parametes, and a hash to store the returned values.

    my %fields;

    SWISH::DateRanges::DateRangeForm( $q, $conf->{date_ranges}, \%fields );


    # Set the layout:

    my $string = '<br>Limit to: '
                 . ( $fields{buttons} ? "$fields{buttons}<br>" : '' )
                 . ( $fields{date_range_button} || '' )
                 . ( $fields{date_range_low}
                     ? " $fields{date_range_low} through $fields{date_range_high}"
                     : '' );

    return $string;
}



#============================================
# Run swish-e and gathers headers and results
# Currently requires fork() to run.
#
#   Pass:
#       $sh - an array with search parameters
#
#   Returns:
#       a reference to a hash that contains the headers and results
#       or possibly a scalar with an error message.
#


sub run_swish {


    my $self = shift;

    my $results = $self->{results};
    my $conf    = $self->{config};
    my $q       = $self->{q};


    my @properties;
    my %seen;

    # Gather up the properties we need in results

    for ( qw/ title_property description_prop display_props link_property/ ) {
        push @properties, ref $conf->{$_} ? @{$conf->{$_}} : $conf->{$_}
            if $conf->{$_} && !$seen{$_}++;
    }

    # Add in the default props that should be seen.
    for ( qw/swishrank/ ) {
        push @properties, $_ unless $seen{$_};
    }


    # add in the default prop - a number must be first (this might be a duplicate in -x, oh well)
    unshift @properties, 'swishreccount';


    $self->swish_command( -x => join( '\t', map { "<$_>" } @properties ) . '\n' );
    $self->swish_command( -H => 9 );


    if ( $conf->{debug} & $SwishSearch::DEBUG_COMMAND ) {
        require Data::Dumper;
        print STDERR "---- Swish parameters ----\n";
        print STDERR Data::Dumper::Dumper($self->swish_command);
        print STDERR "\n-----------------------------------------------\n";
    }






    # Use the swish-e library?

    return $self->run_library( @properties )
        if $self->config('use_library');


    my $fh = $^O =~ /Win32/i
             ? windows_fork( $conf, $self )
             : real_fork( $conf, $self );


    # read in from child

    my %stops_removed;

    my $unknown_output = '';


    while (<$fh>) {

        chomp;

        print STDERR "$_\n" if $conf->{debug} & $SwishSearch::DEBUG_OUTPUT;


        tr/\r//d;

        # This will not work correctly with multiple indexes when different values are used.
        if ( /^# ([^:]+):\s+(.+)$/ ) {

            my $h = lc $1;
            my $value = $2;
            $self->{_headers}{$h} = $value;

            push @{$self->{_headers}{'removed stopwords'}}, $value if $h eq 'removed stopword' && !$stops_removed{$value}++;

            next;
        }


        # return swish errors as a message to the script
        $self->errstr($1), return if /^err:\s*(.+)/;

        # Or, if you want to log the errors and just say "Service Unavailable" use this:
        #die "$1\n" if /^err:\s*(.+)/;


        # Found a result
        if ( /^\d/ ) {

            my %h;
            @h{@properties} = split /\t/;
            $self->add_result_to_list( \%h );
            next;

        } elsif ( /^\.$/ ) {
            last;

        } else {
            next if /^#/;
        }

        $unknown_output .= "'$_'\n";

    }

    die "Swish returned unknown output: $unknown_output\n" if $unknown_output;

    $self->{hits} = $self->{_results} ? @{$self->{_results}} : 0;

}

# Filters in place
sub html_escape {
    $_[0] = '' unless defined $_[0];
    for ($_[0]) {
        s/&/&amp;/g;
        s/</&lt;/g;
        s/>/&gt;/g;
        s/"/&quot;/g;
    }
}


#============================================================================
# Adds a result to the result list and highlight the search words

# This is a common source of bugs!  The problem is that highlighting is done in this code.
# This is good, especially for the description because it is trimmed down as processing each
# result.  Otherwise, would use a lot of memory. It's bad because the highlighting is
# creating html which really should be done in the template output code.
# What that means is the properties that are "searched" are run through the highlighting
# code (and thus HTML escaped) but other properties are not.
# If highlighting (and trimming) is to be kept here then either we need to
# html escape all display properties, or flag which ones are escaped.
# Since we know the ultimate output is HTML, the current method will be to escape here.


sub add_result_to_list {
    my ( $self, $props ) = @_;


    # Push the result onto the list

    push @{$self->{_results}}, $props;


    # We need to save the text of the link prop (almost always swishdocpath)
    # because all properties are escaped.

    my $link_property = $self->config('link_property') || 'swishdocpath';
    my $link_href = ( $self->config('prepend_path') || '' )
                                  . $props->{$link_property};

    # Replace spaces ***argh this is the wrong place to do this! ***
    # This doesn't really work -- file names could still have chars that need to be escaped.
    $link_href =~ s/\s/%20/g;


    # Returns hash of the properties that were highlighted
    my $highlighted = $self->highlight_props( $props ) || {};

    my $trim_prop = $self->config('description_prop') || '';
    $props->{$trim_prop} ||= ''
        if $trim_prop;

    # HTML escape all properties that were not highlighted
    for my $prop (keys %$props) {
        next if  $highlighted->{$prop};

        # not highlighted, so escape
        html_escape( $props->{$prop} );

        if ( $prop eq $trim_prop ) {
            my $max = $self->config('max_chars') || 500;

            $props->{$trim_prop} = substr( $props->{$trim_prop}, 0, $max) . ' <b>...</b>'
                if length $props->{$trim_prop} > $max;
        }
    }

    $props->{swishdocpath_href} = $link_href;  # backwards compatible
    $props->{link_property} = $link_href;  # backwards compatible


}


#=======================================================================================


# This will call the highlighting module as needed.
# The highlighting module MUST html escape the property.
# returns a hash of properties highlighted


sub highlight_props {
    my ( $self, $props ) = @_;

    # make sure we have the config we need.
    my $highlight_settings = $self->config('highlight') || return;
    my $meta_to_prop = $highlight_settings->{meta_to_prop_map} || return;



    # Initialize highlight module ( could probably do this once per instance )
    # pass in the config highlight settings, and the swish-e headers as a hash.

    $self->{_highlight_object} ||= $highlight_settings->{package}->new( $highlight_settings, $self->{_headers} );
    my $highlight_object = $self->{_highlight_object} || return;




    # parse the query on first result

    my $parsed_words =  $self->header( 'parsed words' ) || die "Failed to find 'Parsed Words' in swish headers";

    $self->{parsed_query} ||= ( parse_query( $parsed_words ) || return );


    my %highlighted;  # track which were highlighted to detect if need to trim the description


    # this is probably backwards -- might be better to loop through the %$props

    while ( my( $meta, $phrases ) = each %{$self->{parsed_query}} ) {
        next unless $meta_to_prop->{$meta};  # is it a prop defined to highlight?

        # loop through the properties for the metaname

        for ( @{ $meta_to_prop->{$meta} } ) {
            if ( $props->{$_} ) {
                $highlighted{$_}++ if $highlight_object->highlight( \$props->{$_}, $phrases, $_ );
            }
        }
    }

    return \%highlighted;
}






#==================================================================
# Run swish-e by using the SWISH::API module
#

my %cached_handles;

sub run_library {
    my ( $self, @props ) = @_;

    SwishSearch::load_module( 'SWISH::API' );

    my $indexes = $self->swish_command('-f');


    print STDERR "swish.cgi: running library thus no 'output' available -- try 'summary'\n"
        if ($self->{config}{debug} || 0) & $SwishSearch::DEBUG_OUTPUT;

    eval { require Time::HiRes };
    my $start_time = [Time::HiRes::gettimeofday()] unless $@;



    unless ( $cached_handles{$indexes} ) {

        my $swish = SWISH::API->new( ref $indexes ? join(' ', @$indexes) : $indexes );
        if ( $swish->Error ) {
            $self->errstr( join ': ', $swish->ErrorString, $swish->LastErrorMsg );
            delete $cached_handles{$indexes} if $swish->CriticalError;
            return;
        }

        # read headers (currently only reads one set)
        my %headers;
        my $index = ($swish->IndexNames)[0];

        for ( $swish->HeaderNames ) {
            my @value = $swish->HeaderValue( $index, $_ );
            my $x = @value;
            next unless @value;
            $headers{ lc($_) } = join ' ', @value;
        }


        $cached_handles{$indexes} = {
            swish => $swish,
            headers => \%headers,
        };
    }

    my $swish = $cached_handles{$indexes}{swish};

    my $headers = $cached_handles{$indexes}{headers};

    $self->{_headers} = $headers;


    my $search = $swish->New_Search_Object;  # probably could cache this, too

    if ( my $limits = $self->swish_command( 'limits' ) ) {
        $search->SetSearchLimit( @{$limits}{ qw/prop low high/ } );
    }

    if ( $swish->Error ) {
        $self->errstr( join ': ', $swish->ErrorString, $swish->LastErrorMsg );
        delete $cached_handles{$indexes} if $swish->CriticalError;
        return;
    }


    if ( my $sort = $self->swish_command('-s') ) {
        $search->SetSort( ref $sort ? join( ' ', @$sort) : $sort );
    }

    my $search_time = [Time::HiRes::gettimeofday()] if $start_time;

    my $results = $search->Execute( $self->swish_command('-w') );


    $headers->{'search time'} = sprintf('%0.3f seconds', Time::HiRes::tv_interval( $search_time, [Time::HiRes::gettimeofday()] ))
        if $start_time;


    if ( $swish->Error ) {
        $self->errstr( join ': ', $swish->ErrorString, $swish->LastErrorMsg );
        delete $cached_handles{$indexes} if $swish->CriticalError;
        return;
    }

    # Add in results-related headers
    $headers->{'parsed words'} = join ' ', $results->ParsedWords( ($swish->IndexNames)[0] );

    if ( ! $results->Hits ) {
        $self->errstr('No results');
        return;
    }
    $headers->{'number of hits'} = $results->Hits;

    # Get stopwords removed from each index (really need to track headers per index to be correct)

    for my $index ( $swish->IndexNames ) {
        my @stopwords = $results->RemovedStopwords( $index );

        push @{$headers->{'removed stopwords'}}, @stopwords
            if @stopwords;
    }



    # Now fetch properties

    $results->SeekResult( $self->swish_command( '-b' ) - 1 );

    my $page_size = $self->swish_command( '-m' );

    if ( $swish->Error ) {
        $self->errstr( join ': ', $swish->ErrorString, $swish->LastErrorMsg );
        delete $cached_handles{$indexes} if $swish->CriticalError;
        return;
    }

    my $hit_count;

    while ( my $result = $results->NextResult ) {
        my %props;


        for my $prop ( @props ) {
            # Note, we use ResultPropertyStr instead since this is a general purpose
            # script (it converts dates to a string, for example).
            # $result->Property is a faster method and does not convert dates and numbers to strings.
            #my $value = $result->Property( $prop );
            my $value = $result->ResultPropertyStr( $prop );
            next unless $value;  # ??

            $props{$prop} = $value;
        }

        $hit_count++;

        $self->add_result_to_list( \%props );

        last unless --$page_size;
    }


    $headers->{'run time'} = sprintf('%0.3f seconds', Time::HiRes::tv_interval( $start_time, [Time::HiRes::gettimeofday()] ))
        if $start_time;


    $self->{hits} = $hit_count;


}



#==================================================================
# Run swish-e by forking
#

use Symbol;

sub real_fork {
    my ( $conf, $self ) = @_;


    # Run swish
    my $fh = gensym;
    my $pid = open( $fh, '-|' );

    die "Failed to fork: $!\n" unless defined $pid;


    if ( !$pid ) {  # in child
        unless ( exec $self->{prog},  $self->swish_command_array ) {
            warn "Child process Failed to exec '$self->{prog}' Error: $!";
            print "Failed to exec Swish";  # send this message to parent.
            exit;
        }
    } else {
        $self->{pid} = $pid;
    }

    return $fh;
}


#=====================================================================================
#   Windows work around
#   from perldoc perlfok -- na, that doesn't work.  Try IPC::Open2
#
sub windows_fork {
    my ( $conf, $self ) = @_;


    require IPC::Open2;
    my ( $rdrfh, $wtrfh );

    # Ok, I'll say it.  Windows sucks.
    my @command = map { s/"/\\"/g; qq["$_"] }  $self->{prog}, $self->swish_command_array;
    my $pid = IPC::Open2::open2($rdrfh, $wtrfh, @command );


    $self->{pid} = $pid;

    return $rdrfh;
}



1;


__END__

=head1 NAME

swish.cgi -- Example Perl script for searching with the SWISH-E search engine.

=head1 DESCRIPTION

C<swish.cgi> is a CGI script for searching with the SWISH-E search
engine version 2.1-dev and above.  It returns results a page at a
time, with matching words from the source document highlighted,
showing a few words of content on either side of the highlighted word.

The script is highly configurable.  Features include searching
multiple (or selectable) indexes, limiting searches to a subset of documents,
sorting by a number of different properties, and limiting results to
a date range.

On unix type systems the swish.cgi script is installed in the directory
$prefix/lib/swish-e, which is typically /usr/local/lib/swish-e.  This
can be overridden by the configure options --prefix or --libexecdir.

The standard configuration (i.e. not using a config file) should work
with most swish index files.  Customization of the parameters will be
needed if you are indexing special meta data and want to search and/or
display the meta data.  The configuration can be modified by editing
this script directly, or by using a configuration file (.swishcgi.conf
by default).  The script's configuration file is described below.

You are strongly encouraged to get the default configuration working
before making changes.  Most problems using this script are the result
of configuration modifications.

The script is modular in design.  Both the highlighting code and
output generation is handled by modules, which are included in the
F<example/modules> distribution directory and installed in the
$libexecdir/perl directory.
This allows for easy customization of
the output without changing the main CGI script.

Included with the Swish-e distribution is a module to generate standard HTML
output.  There's also modules and template examples to use with the popular
Perl templating systems HTML::Template and Template-Toolkit.  This is very
useful if your site already uses one of these templating systems The
HTML::Template and Template-Toolkit packages are not distributed with
Swish-e.  They are available from the CPAN (http://search.cpan.org).

This scipt can also run basically unmodified as a mod_perl handler,
providing much better performance than running as a CGI script.  Usage
under mod_perl is described below.

Please read the rest of the documentation.  There's a C<DEBUGGING>
section, and a C<FAQ> section.

This script should work on Windows, but security may be an issue.

=head1 REQUIREMENTS

A reasonably current version of Perl.  5.00503
or above is recommended (anything older will not be supported).

The Date::Calc module is required to use the date range feature of the script.
The Date::Calc module is also available from CPAN.


=head1 INSTALLATION

Here's an example installation session under Linux.  It should be
similar for other operating systems.

For the sake of simplicity in this installation example all files are
placed in web server space, including files such as swish-e index and
configuration files that would normally not be made available via the
web server.  Access to these files should be limited once the script
is running.  Either move the files to other locations (and adjust the
script's configuration) or use features of the web server to limit
access (such as with F<.htaccess>).

Please get a simple installation working before modifying the
configuration file.  Most problems reported for using this script have
been due to improper configuration.

The script's default settings are setup for initial testing.  By default the settings expect
to find most files and the swish-e binary in the same directory as the script.

For I<security> reasons, once you have tested the script you will want to change settings to
limit access to some of these files by the web server (either by moving them out of web
space, or using access control such as F<.htaccess>). An example of using F<.htaccess> on
Apache is given below.

It's expected that swish-e has already been unpacked and the swish-e binary has be compiled
from source and "make install" has been run. If swish-e was installed from a vendor package
(such as from a RPM or Debian package) see that pakage's documentation for where files are
installed.

Example Installation:

=over 4

=item 1 Symlink or copy the swish.cgi.

Symlink (or copy if your platform or webserver does not allow symlinks)
the swish.cgi script from the installation directory to a local directory.
Typically, this would be the cgi-bin directory or a location where CGI script
are located.  In this example a new directory is created and the script is
symlinked.

    ~$ mkdir swishdir
    ~$ cd swishdir
    ~/swishdir$ ln -s /usr/local/lib/swish-e/swish.cgi

The installation directory is set at configure time with the --prefix or
--libexecdir options, but by default is in /usr/local/lib/swish-e.

=item 2 Create an index

Use an editor and create a simple configuration file for indexing your
files.  In this example the Apache documentation is indexed.  Last we
run a simple query to test that the index works correctly.

    ~/swishdir$ cat swish.conf
    IndexDir /usr/local/apache/htdocs
    IndexOnly .html .htm
    DefaultContents HTML*
    StoreDescription HTML* <body> 200000
    MetaNames swishdocpath swishtitle
    ReplaceRules remove /usr/local/apache/

If you do not have the Apache docs installed then pick another directory to index
such as /usr/share/doc.

Create the index.

    ~/swishdir$ swish-e -c swish.conf
    Indexing Data Source: "File-System"
    Indexing "/usr/local/apache/htdocs"
    Removing very common words...
    no words removed.
    Writing main index...
    Sorting words ...
    Sorting 7005 words alphabetically
    Writing header ...
    Writing index entries ...
      Writing word text: Complete
      Writing word hash: Complete
      Writing word data: Complete
    7005 unique words indexed.
    5 properties sorted.
    124 files indexed.  1485844 total bytes.  171704 total words.
    Elapsed time: 00:00:02 CPU time: 00:00:02
    Indexing done!

Now, verify that the index can be searched:

    ~/swishdir$ swish-e -w install -m 1
    # SWISH format: 2.1-dev-25
    # Search words: install
    # Number of hits: 14
    # Search time: 0.001 seconds
    # Run time: 0.040 seconds
    1000 htdocs/manual/dso.html "Apache 1.3 Dynamic Shared Object (DSO) support" 17341
    .

Let's see what files we have in our directory now:

    ~/swishdir$ ls -1
    index.swish-e
    index.swish-e.prop
    swish.cgi
    swish.conf

=item 3 Test the CGI script

This is a simple step, but often overlooked.  You should test from the command line instead of jumping
ahead and testing with the web server.  See the C<DEBUGGING> section below for more information.

    ~/swishdir$ ./swish.cgi | head
    Content-Type: text/html; charset=ISO-8859-1

    <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
    <html>
        <head>
           <title>
              Search our site
           </title>
        </head>
        <body>

The above shows that the script can be run directly, and generates a correct HTTP header and HTML.

If you run the above and see something like this:

    ~/swishdir >./swish.cgi
    bash: ./swish.cgi: No such file or directory

then you probably need to edit the script to point to the correct location of your perl program.
Here's one way to find out where perl is located (again, on unix):

    ~/swishdir$ which perl
    /usr/local/bin/perl

    ~/swishdir$ /usr/local/bin/perl -v
    This is perl, v5.6.0 built for i586-linux
    ...

Good! We are using a reasonably current version of perl.

Now that we know perl is at F</usr/local/bin/perl> we can adjust the "shebang" line
in the perl script (e.g. the first line of the script):

    ~/swishdir$ pico swish.cgi
    (edit the #! line)
    ~/swishdir$ head -1 swish.cgi
    #!/usr/local/bin/perl -w

=item 4 Test with the web server

How you do this is completely dependent on your web server, and you may need to talk to your web
server admin to get this working.  Often files with the .cgi extension are automatically set up to
run as CGI scripts, but not always.  In other words, this step is really up to you to figure out!

This example shows creating a I<symlink> from the web server space to the directory used above.
This will only work if the web server is configured to follow symbolic links (the default for Apache).

This operation requires root access:

    ~/swishdir$ su -c "ln -s $HOME/swishdir /usr/local/apache/htdocs/swishdir"
    Password: *********

If your account is on an ISP and your web directory is F<~/public_html> the you might just move the entire
directory:

    mv ~/swishdir ~/public_html

Now, let's make a real HTTP request:

    ~/swishdir$ GET http://localhost/swishdir/swish.cgi | head -3
    #!/usr/local/bin/perl -w
    package SwishSearch;
    use strict;

Oh, darn. It looks like Apache is not running the script and instead returning it as a
static page.  Apache needs to be told that swish.cgi is a CGI script.

F<.htaccess> comes to the rescue:

    ~/swishdir$ cat .htaccess

    # Deny everything by default
    Deny From All

    # But allow just CGI script
    <files swish.cgi>
        Options ExecCGI
        Allow From All
        SetHandler cgi-script
    </files>

That "Deny From All" prevents access to all files (such as config and index files), and only access is allowed to the
F<swish.cgi> script.

Let's try the request one more time:

    ~/swishdir >GET http://localhost/swishdir/swish.cgi | head
    <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
    <html>
        <head>
           <title>
              Search our site
           </title>
        </head>
        <body>
            <h2>
            <a href="http://swish-e.org">

That looks better!  Now use your web browser to test.

Now, you may note that the links are not valid on the search results page.  The swish config file
contained the line:

     ReplaceRules remove /usr/local/apache/

To make those links works (and assuming your web server will follow symbolic links):

    ~/swishtest$ ln -s /usr/local/apache/htdocs


BTW - "GET" used above is a program included with Perl's LWP library.  If you do no have this you might
try something like:

    wget -O - http://localhost/swishdir/swish.cgi | head

and if nothing else, you can always telnet to the web server and make a basic request.

    ~/swishtest$ telnet localhost 80
    Trying 127.0.0.1...
    Connected to localhost.
    Escape character is '^]'.
    GET /swishtest/swish.cgi http/1.0

    HTTP/1.1 200 OK
    Date: Wed, 13 Feb 2002 20:14:31 GMT
    Server: Apache/1.3.20 (Unix) mod_perl/1.25_01
    Connection: close
    Content-Type: text/html; charset=ISO-8859-1

    <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
    <html>
        <head>
           <title>
              Search our site
           </title>
        </head>
        <body>

This may seem like a lot of work compared to using a browser, but browsers
are a poor tool for basic CGI debugging.


=back

If you have problems check the C<DEBUGGING> section below.

=head1 CONFIGURATION

If you want to change the location of the swish-e binary or the index
file, use multiple indexes, add additional metanames and properties,
change the default highlighting behavior, etc., you will need to
adjust the script's configuration settings.

Again, please get a test setup working with the default parameters before
making changes to any configuration settings.  Better to debug one
thing at a time...

In general, you will need to adjust the script's settings to match the
index file you are searching.  For example, if you are indexing a
hypermail list archive you may want to make the script use
metanames/properties of Subject, Author, and, Email address.  Or you
may wish to provide a way to limit searches to subsets of documents
(e.g. parts of your directory tree).

To make things somewhat "simple", the configuration parameters are
included near the top of the swish.cgi program.  That is the only
place that the individual parameters are defined and explained, so you
will need to open up the swish.cgi script in an editor to view the
options.  Further questions about individual settings should be
referred to the swish-e discussion list.

The parameters are all part of a perl C<hash> structure, and the
comments at the top of the program should get you going.  The perl
hash structure may seem a bit confusing, but it makes it easy to
create nested and complex parameters.  Syntax is important, so
cut-n-paste should be your best defense if you are not a perl
programmer.

By the way, Perl has a number of quote operators.  For example, to
quote a string you might write:

    title => 'Search My Site',

Some options take more than one parameter, where each parameter must
be quoted.  For example:

    metanames => [ 'swishdefault', 'swishtitle',  'swishdocpath' ],

which assigns an array ( [...] ) of three strings to the "metanames"
variable.  Lists of quoted strings are so common in perl that there's
a special operator called "qw" (quote word) to save typing all those quotes:

    metanames => [ qw/ swishdefault swishtitle swishdocpath / ],

or to use the parenthesis as the quote character (you can pick any):

    metanames => [ qw( swishdefault swishtitle swishdocpath ) ],


There are two options for changing the configuration settings from
their default values: One way is to edit the script directly, or the
other was is to use a separate configuration file.  In either case,
the configuration settings are a basic perl hash reference.

Using a configuration file is described below, but contains the same hash structure.

There are many configuration settings, and some of them are commented out either by using
a "#" symbol, or by simply renaming the configuration directive (e.g. by adding an "x" to the parameter
name).

A very basic configuration setup might look like:

    return {
        title           => 'Search the Swish-e list',   # Title of your choice.
        swish_binary    => 'swish-e',                   # Location of swish-e binary
        swish_index     => 'index.swish-e',             # Location of your index file
    };

Or if searching more than one index:

    return {
        title           => 'Search the Swish-e list',
        swish_binary    => 'swish-e',
        swish_index     => ['index.swish-e', 'index2'],
    };

Both of these examples return a reference to a perl hash ( C<return
{...}> ).  In the second example, the multiple index files are set as
an array reference.

Note that in the example above the swish-e binary file is relative to
the current directory.  If running under mod_perl you will need to use
absolute paths.

The script can also use the SWISH::API perl module (included with the
swish-e distribution in the F<perl> directory) to access the swish-e
index.  The C<use_library> option is used to enable the use of the
SWISH::API module:

    return {
        title           => 'Search the Swish-e list',
        swish_index     => ['index.swish-e', 'index2'],
        use_library     => 1, # enable use of the SWISH::API module
    };

The module must be available via the @INC array, like all Perl modules.

Using the SWISH::API module avoids the need to fork and execute a
the swish-e program.  Under mod_perl you will may see a significant
performance improvement when using the SWISH::API module.  Under
normal CGI usage you will probably not see any speed improvements.


B<Using A Configuration File>

As mentioned above, configuration settings can be either set in the
F<swish.cgi> script, or set in a separate configuration file.
Settings in a configuration file will override the settings in the
script.

By default, the F<swish.cgi> script will attempt to read settings from the file F<.swishcgi.conf>.
For example, you might only wish to change the title used
in the script.  Simply create a file called F<.swishcgi.conf> in the same directory as the CGI script:

    > cat .swishcgi.conf
    # Example swish.cgi configuration script.
    return {
       title => 'Search Our Mailing List Archive',
    };

The settings you use will depend on the index you create with swish:

   return {
        title           => 'Search the Apache documentation',
        swish_binary    => 'swish-e',
        swish_index     => 'index.swish-e',
        metanames       => [qw/swishdefault swishdocpath swishtitle/],
        display_props   => [qw/swishtitle swishlastmodified swishdocsize swishdocpath/],
        title_property  => 'swishdocpath',
        prepend_path    => 'http://myhost/apachedocs',

        name_labels => {
            swishdefault        => 'Search All',
            swishtitle          => 'Title',
            swishrank           => 'Rank',
            swishlastmodified   => 'Last Modified Date',
            swishdocpath        => 'Document Path',
            swishdocsize        => 'Document Size',
        },

    };

The above configuration defines metanames to use on the form.
Searches can be limited to these metanames.

"display_props" tells the script to display the property "swishlastmodified" (the last modified
date of the file), the document size, and path with the search results.

The parameter "name_labels" is a hash (reference)
that is used to give friendly names to the metanames.

Here's another example.  Say you want to search either (or both) the Apache 1.3 documentation and the
Apache 2.0 documentation indexed seperately.

    return {
       title       => 'Search the Apache Documentation',
       date_ranges => 0,
       swish_index => [ qw/ index.apache index.apache2 / ],
       select_indexes  => {
            method  => 'checkbox_group',
            labels  => [ '1.3.23 docs', '2.0 docs' ],  # Must match up one-to-one to swish_index
            description => 'Select: ',
        },

    };

Now you can select either or both sets of documentation while searching.

All the possible settings are included in the default configuration located near the top of the F<swish.cgi>
script.  Open the F<swish.cgi> script with an editor to look at the various settings.  Contact the Swish-e Discussion list
for help in configuring the script.


=head1 DEBUGGING

Most problems with using this script have been a result of improper configuration.  Please
get the script working with default settings before adjusting the configuration settings.

The key to debugging CGI scripts is to run them from the command line, not with a browser.

First, make sure the program compiles correctly:

    $ perl -c swish.cgi
    swish.cgi syntax OK

Next, simply try running the program:

    $ ./swish.cgi | head
    Content-Type: text/html; charset=ISO-8859-1

    <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
    <html>
        <head>
           <title>
              Search our site
           </title>
        </head>
        <body>

Under Windows you will need to run the script as:

   C:\wwwroot\swishtest> perl swish.cgi


Now, you know that the program compiles and will run from the command line.
Next, try accessing the script from a web browser.

If you see the contents of the CGI script instead of its output then
your web server is not configured to run the script.  With Apache look
at settings like ScriptAlias, SetHandler, and Options.

If an error is reported (such as Internal Server Error or Forbidden)
you need to locate your web server's error_log file and carefully read
what the problem is.  Contact your web administrator for help locating
the web server's error log.

If you don't have access to the web server's error_log file, you can
modify the script to report errors to the browser screen.  Open the
script and search for "CGI::Carp".  (Author's suggestion is to debug
from the command line -- adding the browser and web server into the
equation only complicates debugging.)

The script does offer some basic debugging options that allow
debugging from the command line.  The debugging options are enabled by
setting an environment variable "SWISH_DEBUG".  How that is set
depends on your operating system and the shell you are using.  These
examples are using the "bash" shell syntax.

Note: You can also use the "debug_options" configuration setting, but
the recommended method is to set the environment variable.

You can list the available debugging options like this:

    $ SWISH_DEBUG=help ./swish.cgi >outfile
    Unknown debug option 'help'.  Must be one of:
           basic: Basic debugging
         command: Show command used to run swish
         headers: Show headers returned from swish
          output: Show output from swish
         summary: Show summary of results
            dump: Show all data available to templates

Debugging options may be combined:

    $ SWISH_DEBUG=command,headers,summary ./swish.cgi >outfile

You will be asked for an input query and the max number of results to return.  You can use the defaults
in most cases.  It's a good idea to redirect output to a file.  Any error messages are sent to stderr, so
those will still be displayed (unless you redirect stderr, too).

Here are some examples:

    ~/swishtest$ SWISH_DEBUG=basic ./swish.cgi >outfile
    Debug level set to: 1
    Enter a query [all]:
    Using 'not asdfghjklzxcv' to match all records
    Enter max results to display [1]:

    ------ Can't use DateRanges feature ------------

    Script will run, but you can't use the date range feature
    Can't locate Date/Calc.pm in @INC (@INC contains: modules /usr/local/lib/perl5/5.6.0/i586-linux /usr/local/lib/perl5/5.6.0 /usr/local/lib/perl5/site_perl/5.6.0/i586-linux /usr/local/lib/perl5/site_perl/5.6.0 /usr/local/lib/perl5/site_perl/5.005/i586-linux /usr/local/lib/perl5/site_perl/5.005 /usr/local/lib/perl5/site_perl .) at modules/DateRanges.pm line 107, <STDIN> line 2.
    BEGIN failed--compilation aborted at modules/DateRanges.pm line 107, <STDIN> line 2.
    Compilation failed in require at ./swish.cgi line 971, <STDIN> line 2.

    --------------
    Can't exec "./swish-e": No such file or directory at ./swish.cgi line 1245, <STDIN> line 2.
    Child process Failed to exec './swish-e' Error: No such file or directory at ./swish.cgi line 1246, <STDIN> line 2.
    Failed to find any results

The above indicates two problems.  First problem is that the Date::Calc module is not installed.
The Date::Calc module is needed to use the date limiting feature of the script.

The second problem is a bit more serious.  It's saying that the script can't find the
swish-e binary file. In this example it's specified as being in the current directory.
Either correct the path to the swish-e binary, or make a local copy or symlink to the
swish-e binary.

    ~/swishtest$ cat .swishcgi.conf
        return {
           title       => 'Search the Apache Documentation',
           swish_binary => '/usr/local/bin/swish-e',
           date_ranges => 0,
        };

Now, let's try again:

    ~/swishtest$ SWISH_DEBUG=basic ./swish.cgi >outfile
    Debug level set to: 1

    ---------- Read config parameters from '.swishcgi.conf' ------
    $VAR1 = {
              'date_ranges' => 0,
              'title' => 'Search the Apache Documentation'
            };
    -------------------------
    Enter a query [all]:
    Using 'not asdfghjklzxcv' to match all records
    Enter max results to display [1]:
    Found 1 results

    Can't locate SWISH::TemplateDefault.pm in @INC (@INC contains: modules /usr/local/lib/perl5/5.6.0/i586-linux /usr/local/lib/perl5/5.6.0 /usr/local/lib/perl5/site_perl/5.6.0/i586-linux /usr/local/lib/perl5/site_perl/5.6.0 /usr/local/lib/perl5/site_perl/5.005/i586-linux /usr/local/lib/perl5/site_perl/5.005 /usr/local/lib/perl5/site_perl .) at ./swish.cgi line 608.

This means that the swish.cgi script could not locate a required module.  To correct this
locate where the SWISH::Template module is installed and add a "use lib" line to
your configuration file (or to the swish.cgi script):


    ~/swishtest$ cat .swishcgi.conf
    use lib '/home/bill/local/lib/perl';

    return {
       title       => 'Search the Apache Documentation',
       date_ranges => 0,
    };

    ~/swishtest$ SWISH_DEBUG=basic ./swish.cgi >outfile
    Debug level set to: 1

    ---------- Read config parameters from '.swishcgi.conf' ------
    $VAR1 = {
              'date_ranges' => 0,
              'title' => 'Search the Apache Documentation'
            };
    -------------------------
    Enter a query [all]:
    Using 'not asdfghjklzxcv' to match all records
    Enter max results to display [1]:
    Found 1 results

That is much better!

The "use lib" statement tells Perl where to look for modules by adding the path supplied
to an array called @INC.

Note that most modules are in the SWISH namespace.  For example, the default output
module is called SWISH::TemplateDefault.  When Perl is looking for that module it is looking for
the file F<SWISH/TemplateDefault.pm>.  If the "use lib" statement is set as:

    use lib '/home/bill/local/lib/perl';

then Perl will look (among other places) for the file

    /home/bill/local/lib/perl/SWISH/TemplateDefault.pm

when attempting to load the SWISH::TemplateDefault module.  Relative paths may also be used.

    use lib 'modules';

will cause Perl to look for the file:

    ./modules/SWISH/TemplateDefault.pm

relative to where the swish.cgi script is running.  (This is not true when running under mod_perl).

Here's another common problem.  Everything checks out, but when you run the script you see
the message:

    Swish returned unknown output

Ok, let's find out what output it is returning:

    ~/swishtest$ SWISH_DEBUG=headers,output ./swish.cgi >outfile
    Debug level set to: 13

    ---------- Read config parameters from '.swishcgi.conf' ------
    $VAR1 = {
              'swish_binary' => '/usr/local/bin/swish-e',
              'date_ranges' => 0,
              'title' => 'Search the Apache Documentation'
            };
    -------------------------
    Enter a query [all]:
    Using 'not asdfghjklzxcv' to match all records
    Enter max results to display [1]:
      usage: swish [-i dir file ... ] [-S system] [-c file] [-f file] [-l] [-v (num)]
      ...
    version: 2.0
       docs: http://sunsite.berkeley.edu/SWISH-E/

    *** 9872 Failed to run swish: 'Swish returned unknown output' ***
    Failed to find any results

Oh, looks like /usr/local/bin/swish-e is version 2.0 of swish.  We need 2.1-dev and above!

=head1 Frequently Asked Questions

Here's some common questions and answers.

=head2 How do I change the way the output looks?

The script uses a module to generate output.  By default it uses the
SWISH::TemplateDefault.pm module. The module used is selected in the
swish.cgi configuration file.  Modules are located in the
example/modules/SWISH directory in the distribution, but are installed in
the $prefix/lib/swish-e/perl/SWISH/ directory.

To make simple changes you can edit the installed SWISH::TemplatDefault
module directly, otherwise make a copy of the module and modify its package
name.  For example, change directories to the location of the installed
module and copy the module to a new name:

    $ cp TemplateDefault.pm MyTemplateDefault.pm

Then at the top of the module adjust the "package" line to:

    package SWISH::MyTemplateDefault;

To use this modules you need to adjust the configuration settings (either at
the top of F<swish.cgi> or in a configuration file:


        template => {
            package     => 'SWISH::MyTemplateDefault',
        },

The module does not need to be in the SWISH namespace, and can be stored in
any location as long as the module can be found via the @INC array (i.e.
modify the "use lib" statement in swish.cgi if needed).


=head2 How do I use a templating system with swish.cgi?

In addition to the TemplateDefault.pm module, the swish-e distribution includes two other Perl modules for
generating output using the templating systems HTML::Template and Template-Toolkit.

Templating systems use template files to generate the HTML, and make maintaining the look of a large (or small) site
much easier. HTML::Template and Template-Toolkit are separate packages and can be downloaded from the CPAN.
See http://search.cpan.org.

Two basic templates are provided as examples for generating output using these templating systems.
The example templates are located in the F<example> directory.
The module F<SWISH::TemplateHTMLTemplate> uses the file F<swish.tmpl> to generate its output, while the
module F<SWISH::TemplateToolkit> uses the F<swish.tt> file.
(Note: swish.tt was renamed from search.tt Jun 03, 2004.)

To use either of these modules you will need to adjust the "template" configuration setting.  Examples for
both templating systems are provided in the configuration settings near the top of the F<swish.cgi> program.

Use of these modules is an advanced usage of F<swish.cgi> and are provided as examples only.

All of the output generation modules are passed a hash with the results from the search, plus other data use to create the
output page.  You can see this hash by using the debugging option "dump" or by using the included SWISH::TemplateDumper
module:

    ~/swishtest >cat .swishcgi.conf
        return {
           title       => 'Search the Apache Documentation',
           template => {
                package     => 'SWISH::TemplateDumper',
            },
        };

And run a query.  For example:

    http://localhost/swishtest/swish.cgi?query=install

=head2 Why are there three different highlighting modules?

Three are three highlighting modules included with the swish-e distribution.
Each is a trade-off of speed vs. accuracy:

    SWISH::DefaultHighlight - reasonably fast, but does not highlight phrases
    SWISH::PhraseHighlight  - reasonably slow, but is reasonably accurate
    SWISH::SimpleHighlight  - fast, some phrases, but least accurate

Eh, the default is actually "PhraseHighlight".  Oh well.

All of the highlighting modules slow down the script.  Optimizations to these modules are welcome!

=head2 My ISP doesn't provide access to the web server logs

There are a number of options.  One way it to use the CGI::Carp module.  Search in the
swish.cgi script for:

    use Carp;
    # Or use this instead -- PLEASE see perldoc CGI::Carp for details
    # use CGI::Carp qw(fatalsToBrowser warningsToBrowser);

And change it to look like:

    #use Carp;
    # Or use this instead -- PLEASE see perldoc CGI::Carp for details
    use CGI::Carp qw(fatalsToBrowser warningsToBrowser);

This should be only for debugging purposes, as if used in production you may end up sending
quite ugly and confusing messages to your browsers.

=head2 Why does the output show (NULL)?

Swish-e displays (NULL) when attempting to display a property that does not exist in the index.

The most common reason for this message is that you did not use StoreDescription in your config file while indexing.

    StoreDescription HTML* <body> 200000

That tells swish to store the first 200,000 characters of text extracted from the body of each document parsed
by the HTML parser.  The text is stored as property "swishdescription".

The index must be recreated after changing the swish-e configuration.

Running:

    ~/swishtest > ./swish-e -T index_metanames

will display the properties defined in your index file.

This can happen with other properties, too.
For example, this will happen when you are asking for a property to display that is not defined in swish.

    ~/swishtest > ./swish-e -w install -m 1 -p foo
    # SWISH format: 2.1-dev-25
    # Search words: install
    err: Unknown Display property name "foo"
    .

    ~/swishtest > ./swish-e -w install -m 1 -x 'Property foo=<foo>\n'
    # SWISH format: 2.1-dev-25
    # Search words: install
    # Number of hits: 14
    # Search time: 0.000 seconds
    # Run time: 0.038 seconds
    Property foo=(NULL)
    .

To check that a property exists in your index you can run:

    ~/swishtest > ./swish-e -w not dkdk -T index_metanames | grep foo
            foo : id=10 type=70  META_PROP:STRING(case:ignore) *presorted*

Ok, in this case we see that "foo" is really defined as a property.  Now let's make sure F<swish.cgi>
is asking for "foo" (sorry for the long lines):

    ~/swishtest > SWISH_DEBUG=command ./swish.cgi > /dev/null
    Debug level set to: 3
    Enter a query [all]:
    Using 'not asdfghjklzxcv' to match all records
    Enter max results to display [1]:
    ---- Running swish with the following command and parameters ----
    ./swish-e  \
    -w  \
    'swishdefault=(not asdfghjklzxcv)'  \
    -b  \
    1  \
    -m  \
    1  \
    -f  \
    index.swish-e  \
    -s  \
    swishrank  \
    desc  \
    swishlastmodified  \
    desc  \
    -x  \
    '<swishreccount>\t<swishtitle>\t<swishdescription>\t<swishlastmodified>\t<swishdocsize>\t<swishdocpath>\t<fos>\t<swishrank>\t<swishdocpath>\n'  \
    -H  \
    9

If you look carefully you will see that the -x parameter has "fos" instead of "foo", so there's our problem.

=head2 How do I use the SWISH::API perl module with swish.cgi?

Use the C<use_library> configuration directive:

    use_library => 1,

This will only provide improved performance when running under mod_perl or other persistent
environments.

=head2 Why does the "Run time" differ when using the SWISH::API module

When using the SWISH::API module the run (and search) times are calculated
within the script.  When using the swish-e binary the swish-e program reports the
times.  The "Run time" may include the time required to load and compile the SWISH::API
module.

=head1 MOD_PERL

This script can be run under mod_perl (see http://perl.apache.org).
This will improve the response time of the script compared to running under CGI by loading the
swish.cgi script into the Apache web server.

You must have a mod_perl enabled Apache server to run this script under mod_perl.

Configuration is simple.  In your httpd.conf or your startup.pl file you need to
load the script.  For example, in httpd.conf you can use a perl section:

    <perl>
        use lib '/usr/local/apache/cgi-bin';  # location of the swish.cgi file
        use lib '/home/yourname/swish-e/example/modules';  # modules required by swish.cgi
        require "swish.cgi";
    </perl>

Again, note that the paths used will depend on where you installed the script and the modules.
When running under mod_perl the swish.cgi script becomes a perl module, and therefore the script
does not need to be installed in the cgi-bin directory.  (But, you can actually use the same script as
both a CGI script and a mod_perl module at the same time, read from the same location.)

The above loads the script into mod_perl.  Then to configure the script to run add this to your httpd.conf
configuration file:

    <location /search>
        PerlSetVar Swish_Conf_File /home/yourname/swish-e/myconfig.pl
        allow from all
        SetHandler perl-script
        PerlHandler SwishSearch
    </location>

Note that you use the "Swish_Conf_File" setting in httpd.conf to tell the script
which config file to use.  This means you can use the same script (and loaded modules)
for different search sites (running on the same Apache server).  You can just specify
differnt config files for each Location and they can search different indexes and
have a completely different look for each site, but all share the same code.

B<Note> that the config files are cached in the swish.cgi script.  Changes to the config file
will require restarting the Apache server before they will be reloaded into the swish.cgi
script.  This avoids calling stat() for every request.

Unlike CGI, mod_perl does not change the current directory to the location of the script, so
your settings for the swish binary and the path to your index files must be absolute
paths (or relative to the server root).

Using the SWISH::API module with mod_perl will provide the most performance improvements.
Use of the SWISH::API module can be enabled by the configuration setting C<use_library>:

    use_library     => 1,

Without highlighting code enabled, using the SWISH::API module resulted in about 20 requests
per second, where running the swish-e binary slowed the script down to about 8 requests per second.

Note that the highlighting code is slow.  For the best search performance turn off highlighting.
In your config file you can add:

    highlighting    => 0,  # disable highlighting

and the script will show the first 500 chars of the description (or whatever you set for "max_chars").
Without highlight one test was processing about 20 request per second.
With The "PhraseHighlight" module that dropped to a little better than two requests per second,
"DefaultHighlight" was about 2.3 request per second, and "SimpleHighlight" was about 6 request per second.

Experiement with different highlighting options when testing performance.

Please post to the swish-e discussion list if you have any questions about running this
script under mod_perl.

Here's some general request/second on an Athlon XP 1800+ with 1/2GB RAM, Linux 2.4.20.

                              Highlighting Mode

                      None     Phrase    Default     Simple
   Using SWISH::API   45        1.5        2          12
   ----------------------------------------------------------------------------
   Using swish-e      12        1.3       1.8         7.5
     binary

As you can see the highlighting code is a limiting factor.

=head1 SpeedyCGI

SpeedyCGI (also called PersistentPerl) is another way to run Perl scripts persistently.
SpeedyCGI is good if you do not have mod_perl available or do not have root access.
SpeedyCGI works on Unix systems by loading the script into a "back end" process and keeping
it in memory between requests.  New requests are passed to the back end processes which avoids
the startup time required by a Perl CGI script.

Install SpeedyCGI from http://daemoninc.com/ (your OS may provide a packaged version of
SpeedyCGI) and then change the first line of swish.cgi.  For example, if the speedy binary is
installed in /usr/bin/speedy, use the line:

    #! /usr/bin/speedy -w -- -t60

The -w option is passed to Perl, and all options following the double-dash are SpeedyCGI options.

Note that when using SpeedyCGI configuration data is cached in memory.  If you change the swish.cgi
configuration file (.swishcgi.conf) then touch the main swish.cgi script to force reloading of
configuration data.

=head1 Spidering

There are two ways to spider with swish-e.  One uses the "http" input method that uses code that's
part of swish.  The other way is to use the new "prog" method along with a perl helper program called
C<spider.pl>.

Here's an example of a configuration file for spidering with the "http" input method.
You can see that the configuration is not much different than the file system input method.
(But, don't use the http input method -- use the -S prog method shown below.)

    # Define what to index
    IndexDir http://www.myserver.name/index.html
    IndexOnly .html .htm

    IndexContents HTML* .html .htm
    DefaultContents HTML*
    StoreDescription HTML* <body> 200000
    MetaNames swishdocpath swishtitle

    # Define http method specific settings -- see swish-e documentation
    SpiderDirectory ../swish-e/src/
    Delay 0

You index with the command:

    swish-e -S http -c spider.conf

Note that this does take longer.  For example, spidering the Apache documentation on
a local web server with this method took over a minute, where indexing with the
file system took less than two seconds.  Using the "prog" method can speed this up.

Here's an example configuration file for using the "prog" input method:

    # Define the location of the spider helper program
    IndexDir ../swish-e/prog-bin/spider.pl

    # Tell the spider what to index.
    SwishProgParameters default http://www.myserver.name/index.html

    IndexContents HTML* .html .htm
    DefaultContents HTML*
    StoreDescription HTML* <body> 200000
    MetaNames swishdocpath swishtitle

Then to index you use the command:

    swish-e -c prog.conf -S prog -v 0

Spidering with this method took nine seconds.


=head1 Stemmed Indexes

Many people enable a feature of swish called word stemming to provide
"fuzzy" search options to their users.  The stemming code does not
actually find the "stem" of word, rather removes and/or replaces
common endings on words.  Stemming is far from perfect, and many words
do not stem as you might expect.  Plus, currently only English is
supported.  But, it can be a helpful tool for searching your site.
You may wish to create both a stemmed and non-stemmed index, and
provide a checkbox for selecting the index file.

To enable a stemmed index you simply add to your configuration file:

    UseStemming yes

If you want to use a stemmed index with this program and continue to highlight search terms you will need
to install a perl module that will stem words.  This section explains how to do this.

The perl module is included with the swish-e distribution.  It can be found in the examples directory (where
you found this file) and called something like:

    SWISH-Stemmer-0.05.tar.gz

The module should also be available on CPAN (http://search.cpan.org/).

Here's an example session for installing the module.  (There will be quite a bit of output
when running make.)


    % gzip -dc SWISH-Stemmer-0.05.tar.gz |tar xof -
    % cd SWISH-Stemmer-0.05
    % perl Makefile.PL
    or
    % perl Makefile.PL PREFIX=$HOME/perl_lib
    % make
    % make test

    (perhaps su root at this point if you did not use a PREFIX)
    % make install
    % cd ..

Use the B<PREFIX> if you do not have root access or you want to install the modules
in a local library.  If you do use a PREFIX setting, add a C<use lib> statement to the top of this
swish.cgi program.

For example:

    use lib qw(
        /home/bmoseley/perl_lib/lib/site_perl/5.6.0
        /home/bmoseley/perl_lib/lib/site_perl/5.6.0/i386-linux/
    );

Once the stemmer module is installed, and you are using a stemmed index, the C<swish.cgi> script will automatically
detect this and use the stemmer module.

=head1 DISCLAIMER

Please use this CGI script at your own risk.

This script has been tested and used without problem, but you should still be aware that
any code running on your server represents a risk.  If you have any concerns please carefully
review the code.

See http://www.w3.org/Security/Faq/www-security-faq.html

Security on Windows questionable.

=head1 SUPPORT

The SWISH-E discussion list is the place to ask for any help regarding SWISH-E or this example
script. See http://swish-e.org.

Before posting please review:

    http://swish-e.org/2.2/docs/INSTALL.html#When_posting_please_provide_the_

Please do not contact the author or any of the swish-e developers directly.

=head1 LICENSE

swish.cgi $Revision$ Copyright (C) 2001 Bill Moseley search@hank.org
Example CGI program for searching with SWISH-E


This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version
2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


=head1 AUTHOR

Bill Moseley

=cut


