// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import javax.microedition.lcdui.*;
import java.util.Vector;

public class Console extends CustomItem
{
    public
    Console(String label, String text, Font f, int w, int minLines)
    {
	super(label);

	if(text != null && text.length() > 0)
	{
	    _text = new StringBuffer(text);
	}
	else
	{
	    _text = new StringBuffer();
	}

	_font = f;
	if(_font == null)
	{
	    _font = Font.getFont(Font.FACE_PROPORTIONAL, Font.STYLE_PLAIN, Font.SIZE_SMALL);
	}

	_maxWidth = w;
	_minLines = minLines;

	_topLine = -1;
    }

    public synchronized void
    setText(String text)
    {
	_lines.removeAllElements();
	if(text != null && text.length() > 0)
	{
	    _text = new StringBuffer(text);
	}
	else
	{
	    _text = new StringBuffer();
	}
	formatText(0);
	_topLine = -1;
	repaint();
    }

    public synchronized void
    addText(String text)
    {
	if(text != null && text.length() > 0)
	{
	    int start = _text.length();
	    _text.append(text);
	    formatText(start);
	    _topLine = -1;
	    repaint();
	}
    }

    protected int
    getMinContentHeight()
    {
	return _font.getHeight() + 2;
    }

    protected int
    getMinContentWidth()
    {
	return _maxWidth;
    }

    protected int
    getPrefContentHeight(int w)
    {
	return _font.getHeight() * _minLines + 2;
    }

    protected int
    getPrefContentWidth(int h)
    {
	return _maxWidth;
    }

    protected synchronized void
    paint(Graphics g, int w, int h)
    {
	//
	// Clear the screen.
	//
	g.setColor(255, 255, 255);
	g.fillRect(0, 0, _actualWidth, _actualHeight);

	g.setColor(0, 0, 0);
	g.setFont(_font);

	int i;
	final int sz = _lines.size();
	if(_topLine == -1)
	{
	    if(sz < _linesPerPage)
	    {
		i = 0;
	    }
	    else
	    {
		i = sz - _linesPerPage;
	    }
	}
	else
	{
	    i = _topLine;
	}

	final int fh = _font.getHeight();
	final String text = _text.toString();

	if(i > 0)
	{
	    g.drawImage(_upImage, _textWidth + 2, 0, Graphics.TOP | Graphics.LEFT);
	}

	if(i + _linesPerPage < sz)
	{
	    g.drawImage(_downImage, _textWidth + 2, _actualHeight, Graphics.BOTTOM | Graphics.LEFT);
	}

	int y = 2;
	while(i < sz)
	{
	    Line l = (Line)_lines.elementAt(i);
	    if(l.len > 0)
	    {
		g.drawSubstring(text, l.start, l.len, 2, y, Graphics.TOP | Graphics.LEFT);
	    }
	    y += fh;
	    ++i;
	}

	g.drawRect(0, 0, _textWidth - 1, _actualHeight - 1);
    }

    protected synchronized void
    sizeChanged(int w, int h)
    {
	if(w != _actualWidth || h != _actualHeight)
	{
	    _actualWidth = w;
	    _actualHeight = h;
	    _textWidth = _actualWidth - _upImage.getWidth() - 2;
	    _linesPerPage = (_actualHeight - 2) / _font.getHeight();
	    _lines.removeAllElements();
	    formatText(0);
	}
    }

    protected boolean
    traverse(int dir, int viewportWidth, int viewportHeight, int[] visRect_inout)
    {
	if(_traversing)
	{
	    switch(dir)
	    {
	    case Canvas.UP:
		if(_topLine == -1 && _lines.size() > _linesPerPage)
		{
		    _topLine = _lines.size() - _linesPerPage - 1;
		    repaint();
		}
		else if(_topLine > 0)
		{
		    --_topLine;
		    repaint();
		}
		break;

	    case Canvas.DOWN:
		if(_topLine >= 0)
		{
		    ++_topLine;
		    if(_topLine + _linesPerPage > _lines.size())
		    {
			_topLine = -1;
		    }
		    repaint();
		}
		break;

	    case Canvas.LEFT:
	    case Canvas.RIGHT:
		_traversing = false;
		return false;
	    }
	}

	visRect_inout[0] = 0;
	visRect_inout[1] = 0;
	visRect_inout[2] = _actualWidth;
	visRect_inout[3] = _actualHeight;
	_traversing = true;
	return true;
    }

    private void
    formatText(int start)
    {
	final int len = _text.length();
	if(len <= start)
	{
	    return;
	}

	int pos = start;

	Line line = null;
	boolean addLine = false;
	if(!_lines.isEmpty())
	{
	    Line l = (Line)_lines.lastElement();
	    if(l.open)
	    {
		line = l;
	    }
	}

	final String text = _text.toString();
	final int maxWidth = _textWidth - 2;

	while(pos < len)
	{
	    if(line == null)
	    {
		line = new Line(pos, 0, true);
		addLine = true;
	    }

	    int end;
	    int nl = text.indexOf('\n', line.start);
	    if(nl == -1)
	    {
		end = len;
	    }
	    else if(nl == line.start) // Empty line.
	    {
		end = pos;
	    }
	    else
	    {
		end = nl;
	    }

	    //
	    // Determine the longest substring that will fit in maxWidth.
	    //
	    if(end > line.start && _font.substringWidth(text, line.start, end - line.start) > maxWidth)
	    {
		int low = line.start;
		int high = end;
		while(low <= high)
		{
		    int mid = (low + high) >> 1;

		    int w = _font.substringWidth(text, line.start, mid - line.start);
		    if(w > maxWidth)
		    {
			high = mid - 1;
		    }
		    else
		    {
			if(mid < end && (w + _font.charWidth(text.charAt(mid)) < maxWidth))
			{
			    low = mid + 1;
			}
			else
			{
			    end = mid;
			    break;
			}
		    }
		}
	    }

	    line.len = end - line.start;
	    line.open = nl == -1 || nl > end;
	    if(addLine)
	    {
		_lines.addElement(line);
	    }
	    if(!line.open)
	    {
		++end;
	    }
	    line = null;

	    pos = end;
	}
    }

    private static class Line
    {
	Line(int start, int len, boolean open)
	{
	    this.start = start;
	    this.len = len;
	    this.open = open;
	}

	int start;
	int len;
	boolean open;
    }

    private StringBuffer _text;
    private Font _font;
    private int _maxWidth;
    private int _minLines;
    private int _topLine;
    private int _actualWidth;
    private int _actualHeight;
    private int _textWidth;
    private int _linesPerPage;
    private Vector _lines = new Vector();
    private boolean _traversing;
    private static final Image _upImage;
    private static final Image _downImage;
    private static final byte[] _upImageData =
    {
        -119, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 7, 0, 0, 0, 4, 1, 3,
        0, 0, 0, 120, 8, -73, 62, 0, 0, 0, 4, 103, 65, 77, 65, 0, 0, -39, 4, -36, -78, -38, 2, 0, 0,
        0, 6, 80, 76, 84, 69, -1, -1, -1, 0, 0, 0, 85, -62, -45, 126, 0, 0, 0, 1, 116, 82, 78, 83, 0,
        64, -26, -40, 102, 0, 0, 0, 25, 116, 69, 88, 116, 83, 111, 102, 116, 119, 97, 114, 101, 0, 71,
        114, 97, 112, 104, 105, 99, 67, 111, 110, 118, 101, 114, 116, 101, 114, 53, 93, 72, -18, 0, 0,
        0, 22, 73, 68, 65, 84, 120, -100, 98, 16, 96, -80, 96, -88, 97, -8, 15, 0, 0, 0, -1, -1, 3, 0,
        4, 3, 1, -60, -99, 5, -68, 77, 0, 0, 0, 0, 73, 69, 78, 68, -82, 66, 96, -126
    };
    private static final byte[] _downImageData =
    {
        -119, 80, 78, 71, 13, 10, 26, 10, 0, 0, 0, 13, 73, 72, 68, 82, 0, 0, 0, 7, 0, 0, 0, 4, 1, 3,
        0, 0, 0, 120, 8, -73, 62, 0, 0, 0, 4, 103, 65, 77, 65, 0, 0, -39, 4, -36, -78, -38, 2, 0, 0,
        0, 6, 80, 76, 84, 69, -1, -1, -1, 0, 0, 0, 85, -62, -45, 126, 0, 0, 0, 1, 116, 82, 78, 83, 0,
        64, -26, -40, 102, 0, 0, 0, 25, 116, 69, 88, 116, 83, 111, 102, 116, 119, 97, 114, 101, 0, 71,
        114, 97, 112, 104, 105, 99, 67, 111, 110, 118, 101, 114, 116, 101, 114, 53, 93, 72, -18, 0, 0,
        0, 22, 73, 68, 65, 84, 120, -100, 98, -8, -57, 80, -53, 96, -55, 32, 0, 0, 0, 0, -1, -1, 3, 0,
        10, 38, 1, -59, -79, -40, -59, 58, 0, 0, 0, 0, 73, 69, 78, 68, -82, 66, 96, -126
    };
    static
    {
	_upImage = Image.createImage(_upImageData, 0, _upImageData.length);
	_downImage = Image.createImage(_downImageData, 0, _downImageData.length);
    }
}
