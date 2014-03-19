// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(){

$(document).foundation();

$("#timeout").noUiSlider({range: [0, 2500], start: 0, handles: 1});
$("#delay").noUiSlider({range: [0, 2500], start: 0, handles: 1});
$("#progress .icon").spin("small");

//
// Show demo/test README modal dialog.
//
$("#viewReadme").click(
    function()
    {
        $("#readme-modal").foundation("reveal", "open");
        return false;
    });

//
// Load the source code and highlight it.
//
$(".source").each(
    function(i, e)
    {
        $.ajax(
            {
                url: $(e).attr("data-code"), 
                //
                // Use text data type to avoid problems interpreting the data.
                //
                dataType: "text"
            }
        ).done(
            function(data)
            {
                $(e).text(data);
                hljs.highlightBlock(e);
            });
    });

//
// Show source code modal dialog.
//
$("#viewSource").click(
    function()
    {
        $("#source-modal").foundation("reveal", "open");
        return false;
    });

//
// If the demo page was not load from a web server display
// the setup-modal dialog.
//
if(document.location.protocol === "file:")
{
    var setupDialog = "<div id=\"setup-modal\" class=\"reveal-modal\" data-reveal>" +
        "<p>The Ice for JavaScript demos require a web server. Please refer to the Sample Programs page from the " + 
        "Ice for JavaScript <a href=\"http://doc.zeroc.com/display/Rel/Ice+for+JavaScript+0.1.0+Release+Notes\">" +
        " release notes</a> for instructions on how to run the web server included with your distribution.</p></div>";
    
    $("body").append(setupDialog);
    $("#setup-modal").foundation({
        reveal:
        {
            close_on_background_click: false,
            close_on_esc: false
        }
    });
    $("#setup-modal").foundation("reveal", "open");
}


}());

//
// Check if the corresponding generated files can be access, if they
// cannot be access display the build-required-modal otherwhise do 
// nothing.
//
function checkGenerated(files)
{
    var dialog = "<div id=\"build-required-modal\" class=\"reveal-modal\" data-reveal>" +
        "<p>Couldn't find generated file `%FILENAME%'. This is expected if you didn't build the JavaScript demos. " +
        "Please refer to the Sample Programs page from the Ice for JavaScript " +
        "<a href=\"http://doc.zeroc.com/display/Rel/Ice+for+JavaScript+0.1.0+Release+Notes\">release notes</a> " + 
        "for instructions on how to build the demos.</p>" +
        "</div>";
    
    var basePath = document.location.pathname;
    basePath = basePath.substr(0, basePath.lastIndexOf("/"));
    
    var error = false;
    files.forEach(
        function(f)
        {
            $.ajax(
                {
                    headers: {method: "HEAD"}, 
                    url: basePath + "/" + f,
                    //
                    // Use text data type to avoid problems interpreting the data.
                    //
                    dataType: "text"
                }
            ).fail(
                function(err)
                {
                    if(!error)
                    {
                        error = true;
                        $("body").append(dialog.replace("%FILENAME%", f));
                        $("#build-required-modal").foundation({
                            reveal:
                            {
                                close_on_background_click: false,
                                close_on_esc: false
                            }
                        });
                        $("#build-required-modal").foundation("reveal", "open");
                    }
                });
        });
}
