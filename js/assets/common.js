//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

(function(){

$(document).foundation();

$("#progress .icon").spin("small");

//
// Show test README modal dialog.
//
$("#viewReadme").click(
    function()
    {
        $("#readme-modal").foundation("reveal", "open");
        return false;
    });

//
// If the page was not loaded from a web server display
// the setup-modal dialog.
//
if(document.location.protocol === "file:")
{
    var setupDialog = "<div id=\"setup-modal\" class=\"reveal-modal\" data-reveal>" +
        "<p>The Ice for JavaScript Test Suite requires a web server. Please refer to the usage instructions " +
        "in our <a href=\"https://github.com/zeroc-ice/ice\">GitHub repository</a> for information " +
        "on how to run the web server included with your distribution.</p></div>";

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
