// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var symboltree = new Object();

symboltree.id = 'SymbolTree';
symboltree.cookie = 'SliceSymbolTree';

symboltree.main = function()
{
    //
    // Look for a collapsible list.
    //
    symboltree.tree = document.getElementById(symboltree.id);
    if(!symboltree.tree)
    {
	alert('There is no symbol tree on this page.');
	return;
    }

    if(symboltree.tree.style)
    {
	//
	// Assign class 'CollapsibleList' to all the unordered lists.
	// That way, if the browser does not support DOM, the CCS style
	// that removes the bullets and indent does not apply.
	//
	symboltree.tree.className = 'CollapsibleList';
	var ul = symboltree.tree.getElementsByTagName('ul');
	for(var i = 0; i < ul.length; ++i)
	{
	    ul[i].className = 'CollapsibleList';
	}
    }
    else
    {
        return; // Browser doesn't support DOM, don't do anything.
    }

    //
    // Find all <li> list items.
    //
    var list = symboltree.tree.getElementsByTagName('li');
    for(var i = 0; i < list.length; ++i)
    {
	//
	// If this list contains sub-lists, it needs a collapse/expand widget.
	//
        if(list[i].getElementsByTagName('ul').length != 0)
	{
	    //
	    // Attach an event listener to the widget.
	    //
	    symboltree.addHandler(list[i], 'click', symboltree.handleEvent);

	    //
	    // The list starts out collapsed by default. Set state and class name, and
	    // hide all the entries in this list.
	    //
	    list[i].setAttribute('state', 'collapsed');
	    list[i].className = 'CollapsedList';
	    var children = list[i].childNodes;
	    for(var j = 0; j < children.length; ++j)
	    {
		if(children[j].tagName == 'UL' || children[j].tagName == 'LI')
		{
		    children[j].style.display = 'none';
		}
	    }
	}
	else
	{
	    //
	    // We have a plain <li> list item without <ul> sub-lists. We attach a class name
	    // to these items so we can refer to them separately in the style sheet.
	    //
	    list[i].className = 'ListEntry';
	}
    }

    //
    // Save expanded list indexes in cookie when page unloads.
    //
    symboltree.addHandler(window, 'unload', symboltree.save);

    //
    // Load state of list.
    //
    symboltree.restore();

    //
    // Set handler for the "Expand All" and "Collapse All" buttons.
    //
    var expandAll = document.getElementById('ExpandAllButton');
    symboltree.addHandler(expandAll, 'click', symboltree.expandAll);

    var collapseAll = document.getElementById('CollapseAllButton');
    symboltree.addHandler(collapseAll, 'click', symboltree.collapseAll);
}

//
// Mouse click event handler for collapse/expand widget.
//
symboltree.handleEvent = function(e)
{
    var target;
    if(!e)
    {
        e = window.event;
    }

    if(e.target)
    {
        target = e.target;
    }
    else if(e.currentTarget)
    {
        target = e.currentTarget;
    }
    else if(e.srcObject)
    {
        target = e.srcObject;
    }
    else if(e.srcElement)
    {
        target = e.srcElement;
    }
    if(!target)
    {
        alert('No target for event!');
    }

    if(!target.className || (target.className != 'CollapsedList' && target.className != 'ExpandedList'))
    {
	return; // Ignore event because it bubbled up from child element (namely, from a link in the list).
    }

    //
    // Toggle the list.
    //
    if(target.getAttribute('state') == 'expanded')
    {
        symboltree.collapse(target);
    }
    else
    {
        symboltree.expand(target);
    }

    //
    // Stop event from bubbling up.
    //
    if(e.stopPropagation)
    {
	e.stopPropagation();	// For standards-compliant browsers.
    }
    else
    {
        e.cancelBubble = true;	// For IE
    }
}

//
// Expand sub-list. The state of the list is remembered in the 'state' attribute.
// Expanded lists have a class name of 'ExpandedList', so the style sheet can select
// the correct icon. To expand a list, we change the display of all immediate
// child <ul> and <li> nodes to 'block'.
//
symboltree.expand = function(list)
{
    var state = list.getAttribute('state');
    if(state && state != 'expanded')
    {
	list.setAttribute('state', 'expanded');
	list.className = 'ExpandedList';
	var children = list.childNodes;
	for(var i = 0; i < children.length; ++i)
	{
	    if(children[i].tagName == 'UL' || children[i].tagName == 'LI')
	    {
		children[i].style.display = 'block';
	    }
	}
    }
}

symboltree.expandAll = function()
{
    var list = symboltree.tree.getElementsByTagName('li');
    for(var i = 0; i < list.length; ++i)
    {
	if(list[i].getElementsByTagName('ul').length != 0)
	{
	    symboltree.expand(list[i]);
	}
    }
}

//
// Collapse a sub-list. The state of the list is remembered in the 'state' attribute.
// Collapsed lists have a class name of 'CollapsedList', so the style sheet can select
// the correct icon. To collapse a list, we change the display of all immediate
// child <ul> and <li> nodes to 'none'.
//
symboltree.collapse = function(list)
{
    var state = list.getAttribute('state');
    if(state && state != 'collapsed')
    {
	list.setAttribute('state', 'collapsed');
	list.className = 'CollapsedList';
	var children = list.childNodes;
	for(var i = 0; i < children.length; ++i)
	{
	    if(children[i].tagName == 'UL' || children[i].tagName == 'LI')
	    {
		children[i].style.display = 'none';
	    }
	}
    }
}

symboltree.collapseAll = function()
{
    var list = symboltree.tree.getElementsByTagName('li');
    for(var i = 0; i < list.length; ++i)
    {
	if(list[i].getElementsByTagName('ul').length != 0)
	{
	    symboltree.collapse(list[i]);
	}
    }
}

//
// Save state of the list in a cookie. We save the index of each
// expanded <li> element.
//
symboltree.save = function()
{
    var list = symboltree.tree.getElementsByTagName('li');
    var saved = new Array();
    for(var i = 0; i < list.length; ++i)
    {
        if(list[i].getElementsByTagName('ul').length != 0)
	{
	    var state = list[i].getAttribute('state');
	    if(state && state == 'expanded')
	    {
	        saved[saved.length] = i;
	    }
	}
    }
    if(saved.length == 0)
    {
        saved[0] = 'none';
    }
    document.cookie = symboltree.cookie + '=' + saved;
}

//
// Restore the state of the list from a cookie.
//
symboltree.restore = function()
{
    var regex = new RegExp(symboltree.cookie + '=[^;]+', 'i');
    if (document.cookie.match(regex))
    {
	var value = document.cookie.match(regex)[0].split('=')[1];
	if(value != 'none')
	{
	    var indexes = value.split(',');
	    var list = symboltree.tree.getElementsByTagName('li');
	    for(var i = 0; i < indexes.length; ++i)
	    {
	        symboltree.expand(list[indexes[i]]);
	    }
	}
    }
}

//
// Add an event handler.
//
symboltree.addHandler = function(target, event, handler)
{
    if(target.addEventListener)
    {
        target.addEventListener(event, handler, false);
    }
    else
    {
        target.attachEvent('on' + event, handler);
    }
}

symboltree.main(); // Start running.
