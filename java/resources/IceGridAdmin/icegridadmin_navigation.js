/* ---------------- Script © 2005-2007 EC Software -----------------
   This script was created by Help & Manual and is part of the      
   "Browser-based Help" export format. This script is designed for  
   use in combination with the output of Help & Manual and must not 
   be used outside this context.     http://www.helpandmanual.com   
                                                                    
   Do not modify this file! It will be overwritten by Help & Manual.
----------------------------------------------------------------- */

var usecookie = false;
var tocselecting = false;
var currentselection;
var autocollapse = false;
var treestate = "";  

function fullexpand() { switchall(true); }
function fullcollapse() { switchall(false); }

function levelexpand(divID) {
    var div = hmnavigation.document.getElementById(divID).firstChild;
    while (div) {
       switchdiv(div, div.id, true);
       div = div.nextSibling;
    }
}

function switchall(divvisible) {
    var tree = hmnavigation.document.getElementById("tree");
    if (tree) {
       var items = tree.getElementsByTagName("div");
       for(var i = 0; i < items.length; i++) if (divvisible != ((items[i].style.display=="block")?true:false)) switchdiv(items[i], items[i].id, divvisible);
       if ((divvisible) && (currentselection)) intoview(currentselection, tree, false);
    }
}

function loadstate(tree) {
    var divID;
    if ((treestate=="") && (usecookie)) treestate = document.cookie;
    
    while (treestate != "") {
       divID = treestate.substring(0,treestate.indexOf(","));
       treestate = treestate.substring(divID.length+1,treestate.length);
       toggle(divID);
    }
    var topicID = hmcontent.location.href.substring(hmcontent.location.href.lastIndexOf("/")+1,hmcontent.location.href.length);
    if (topicID.lastIndexOf("#") != -1) topicID = topicID.substring(0,topicID.lastIndexOf("#"));
    if (topicID.lastIndexOf("?") != -1) topicID = topicID.substring(0,topicID.lastIndexOf("?"));
    tocselecting = false;
    currentselection = null;
    lazysync(topicID);
}

function savestate(tree) {
    treestate = "";
    var items = tree.getElementsByTagName("div");
    for(var i = 0; i < items.length; i++) if (items[i].style.display=="block") treestate = treestate.concat(items[i].id + ",");
    if (usecookie) document.cookie = treestate;
}

function toggle(divID) {
    var thisdiv = hmnavigation.document.getElementById(divID);
    if (thisdiv) switchdiv(thisdiv, divID, ((thisdiv.style.display=="none")?true:false));
}

function switchdiv(thisdiv, divID, divvisible) {
    var thisicon = hmnavigation.document.getElementById("i"+divID.substring(3,divID.length));
    var icons = "";
    if (thisicon) icons = thisicon.getAttribute("name");
    if (divvisible) {
       thisdiv.style.display="block";
       if (thisicon) thisicon.src = icons.substring(icons.lastIndexOf(":")+1, icons.length);
    }
    else {
       thisdiv.style.display="none";
       if (thisicon) thisicon.src = icons.substring(0, icons.lastIndexOf(":"));
    }
}

function hilightexpand(spanID, divID) {
    hilight(spanID);
    var thisdiv = hmnavigation.document.getElementById(divID);
    if (thisdiv) switchdiv(thisdiv, divID, true);
}

function hilight(spanID) {
    tocselecting = true;
    var thisnode = null;
    var selectionchanged = false;
    thisnode = hmnavigation.document.getElementById(spanID);
    if (thisnode) {
       try {
          if ((currentselection) && (currentselection != thisnode)) currentselection.className = "heading" + currentselection.className.substr(7,1);
       }
       catch(e){}
       thisnode.className = "hilight"+thisnode.className.substr(7,1);
       selectionchanged = (currentselection != thisnode);
       currentselection = thisnode;
    }
    return selectionchanged;
}

function intoview(thisnode, tree, selectionchanged) {
    var thisparent = thisnode;
    while (thisparent != tree) {
       if ((selectionchanged) && (thisparent.nodeName.toLowerCase()=="div")) switchdiv(thisparent,thisparent.id,true);
       thisparent = thisparent.parentNode;
    }
    thisparent = thisnode;
    for (var t=0; thisparent!=null; t+=thisparent.offsetTop, thisparent=thisparent.offsetParent);
    var bt = (hmnavigation.window.pageYOffset)?hmnavigation.window.pageYOffset:hmnavigation.document.body.scrollTop;
    var bh = (hmnavigation.window.innerHeight)?hmnavigation.window.innerHeight:hmnavigation.document.body.offsetHeight;
    if ((t+thisnode.offsetHeight-bt) > bh) hmnavigation.window.scrollTo(0,(t+24-bh))
    else if (t < bt) hmnavigation.window.scrollTo(0,t);              
}

function collapseunfocused(tree, selectedID) {
    if (tree) {
       var nodepath = "div"+selectedID.substring(1,selectedID.length);
       var items = tree.getElementsByTagName("div");
       for (var i = 0; i < items.length; i++) {
          if (nodepath.lastIndexOf(items[i].id)<0) { switchdiv(items[i], items[i].id, false); }
       }
    }
}

function quicksync(aID) {
    if (aID != "") {
       var tree = hmnavigation.document.getElementById("tree");
       if ((tree) && (!tocselecting)) {
          var thisspan = hmnavigation.document.getElementById(aID);
          if (thisspan) {
             var selectionchanged = hilight("s"+aID.substring(1,aID.length));
             intoview(thisspan, tree, selectionchanged);
          }
       }
       if (autocollapse) {
          if (currentselection) collapseunfocused(tree, currentselection.id);
          else collapseunfocused(tree, "");
       }
    }
    tocselecting = false;
}

function lazysync(topicID) {
    if (topicID != "") {
       var tree = hmnavigation.document.getElementById("tree");
       if ((tree) && (!tocselecting)) {
          var array = new Array(0);
          var items = tree.getElementsByTagName("a");
          for(var i = 0; i < items.length; i++) {
             if (items[i].href.substring(items[i].href.lastIndexOf("/")+1,items[i].href.length)==topicID) {
                var selectionchanged = hilight("s"+items[i].id.substring(1,items[i].id.length));
                intoview(items[i], tree, selectionchanged);
                break;
             }
          }
       }
       if (autocollapse) {
          if (currentselection) collapseunfocused(tree, currentselection.id);
          else collapseunfocused(tree, "");
       }
    }
    tocselecting = false;
}

function preloadicons() {
    var icons = new Array();
    for (i=0; i<preloadicons.arguments.length; i++) {
       icons[i] = new Image();
       icons[i].src = preloadicons.arguments[i];
    }
}
