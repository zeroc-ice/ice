// ----------------------------------------------------------------------------
// Zoom Search Engine 4.3 (27/6/2006)
// Highlight & auto-scroll script
//
// email: zoom@wrensoft.com
// www: http://www.wrensoft.com
//
// Copyright (C) Wrensoft 2006
// ----------------------------------------------------------------------------
// Use this script to allow your search matches to highlight and scroll to
// the matched word on the actual web page where it was found.
//
// You will need to link to this JS file from each page of your site
// which requires the "highlight/jump to matched word" feature.
//
// For example, you could paste the following HTML in your site's header or 
// footer:
//
//   <style>.highlight { background: #FFFF40; }</style>
//   <script type="text/javascript" src="highlight.js"></script>
//
// Note: You will need to specify the correct path to "highlight.js" depending
// on where the file is located.
//
// You will then need to modify the BODY tag on your page to include an "onLoad" 
// attribute, such as:
//
//   <body onload="highlight();">
//
// If for some reason you can not modify the body tag of your page, an alternative
// would be to put the following line after the </body> tag of your page:
//
//   <script type="text/javascript">highlight();</script>
//
// For more information, consult the Users Guide and our support website at:
// http://www.wrensoft.com/zoom/support
//
//
// This script is licensed for use with the Zoom Search Engine. Original
// development by Brett Alcock, VBasys Limited. To licence other applications 
// email highlight@vbasys.com with the subject "license".

var CatchJSErrors = true;

function catcherror() { return true; }
if (CatchJSErrors)
{
    window.onerror = catcherror;
}

function QueryString(key)
{
    var value = null;
    for (var i=0;i<QueryString.keys.length;i++)
    {
        if (QueryString.keys[i]==key)
        {
            value = QueryString.values[i];
            break;
        }
    }
    return value;
}

function QueryString_Parse()
{
    var query = window.location.search.substring(1);
    var pairs = query.split("&");

    for (var i=0;i<pairs.length;i++)
    {
        var pos = pairs[i].indexOf('=');
        if (pos >= 0)
        {
            var argname = pairs[i].substring(0,pos);
            var value = pairs[i].substring(pos+1);
            QueryString.keys[QueryString.keys.length] = argname;
            QueryString.values[QueryString.values.length] = value;
        }
    }
}

QueryString.keys = new Array();
QueryString.values = new Array();

QueryString_Parse();

function getElement(id)
{
    if (document.getElementById)
        return(document.getElementById(id));
    else if (document.all)
        return(document.all[id]);
}

function findPosY(obj)
{
    var curtop = 0;
    if (obj.offsetParent)
    {
        while (obj.offsetParent)
        {
            curtop += obj.offsetTop
            obj = obj.offsetParent;
        }
    }
    else if (obj.y)
        curtop += obj.y;
    return curtop;
}


// regular expression version
function SearchHiLite(text)
{
    var SearchAsSubstring = 0;
    var hl;

    hl = QueryString("zoom_highlight");
    if (hl == "" || hl == null)
    {
        hl = QueryString("zoom_highlightsub");
        if (hl == "" || hl == null)
            return false;
        else
            SearchAsSubstring = 1;
    }
    if ((document.charset && document.charset == "utf-8") ||
    	(document.characterSet && document.characterSet == "UTF-8"))
    	hl = decodeURIComponent(hl);
    else
    	hl = unescape(hl);
    hl = hl.toLowerCase();
                
    // create array of terms        
    //var term = hl.split("+"); 
    var re = /\"(.*?)\"|[^\\+\"]+/g;
    var term = hl.match(re);    
   
    // convert terms in regexp patterns
    for (var i=0;i<term.length;i++) // take each term in turn
    {       
        if(term[i] != "")
        {                   
            if (term[i].indexOf("\"") != -1)
            {
                // contains double quotes               
                term[i]=term[i].replace(/\"/g,"");
                term[i]=term[i].replace(/\+/g," "); 
            }
            else
            {
                term[i]=term[i].replace(/\+/g,"");  
            }                           

            if (term[i].indexOf("*") != -1 || term[i].indexOf("?") != -1)
            {
                // convert wildcard pattern to regexp
                term[i] = term[i].replace(/\\/g, " ");
                term[i] = term[i].replace(/\^/g, " ");

                //term[i] = term[i].replace(/\+/g, " "); // split on this so no point in looking

                term[i] = term[i].replace(/\#/g, " ");
                term[i] = term[i].replace(/\$/g, " ");
                term[i] = term[i].replace(/\./g, " ");
                
                // check if search term only contains only wildcards
                // if so, we will not attempt to highlight this term
                var wildcards = /\w/;
                if (wildcards.test(term[i]))
                {
                	term[i] = term[i].replace(/\*/g, "[^\\s]*");
                	term[i] = term[i].replace(/\?/g, "[^\\s]"); // insist upon one non whitespace
                }                
                else                
                	term[i] = "";                
            }
			
			if (term[i] != "")
			{
	            if (SearchAsSubstring == 0)
	            {	                
	                term[i] = "(>[\\s]*|>[^<]+[\\b\\W])("+term[i]+")(<|[\\b\\W][^>]*<)";
	            }
	            else
	            {
	                // if term leads with wildcard then allow it to match preceeding text in word
	                var strWB="";
	                if(term[i].substr(0,7)=="[^\\s]*") strWB="\\b";
	                term[i] = "(>|>[^<]+)"+strWB+"("+term[i]+")([^>]*<)";
	            }
	        }	        
        }
    }

    text=text.replace(/&amp;/ig, '&');
    text=text.replace(/&nbsp;/ig, '');

    for (var i=0;i<term.length;i++) // take each term in turn
    {
        if(term[i] != "")
        {        	        	
            // we need a loop for the main search to catch all between ><
            // and we add  before each found to ignore those done etc
            // todo: develop reliable single pass regexp and dispose of loop
            var l = 0;
            re = new RegExp(term[i], "gi");
            var count = 0; // just incase
            text = ">" + text + "<"; // temporary tag marks
            do 
            {
                l=text.length;
                text=text.replace(re, '$1<span style="background:#FFFF40;" class="highlight" id="highlight" name="highlight">$2</span id="highlight">$3');
                count++;
            }
            //while(re.lastIndex>0 && count<100); lastIndex not set properly under netscape
            while(l!=text.length && count<100);
            text = text.substring(1, text.length-1); // remove temporary tags
        }
    }        
    text = text.replace(eval("//g"), '');        
    text = text.replace(eval("//g"), '&nbsp;');    
       
    return(text);
}

function jumpHL()
{
    var d=getElement("highlight");
    if(d)
    {
        var y=findPosY(d);
        // if element near top of page
        if(y < 100)
            window.scrollTo(0,0); // go to top of page
        else
            window.scrollTo(0,y-50); // show space of 50 above
    }
}

function highlight()
{	
    var x = document.body;
    if (x)
    {
        var strHTML=SearchHiLite(x.innerHTML);
        if (strHTML!=false) x.innerHTML = strHTML;
        jumpHL();
    }
}
