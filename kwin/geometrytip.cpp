/*
 *  $Id$
 *  Window Geometry Display
 *  Copyright (c) 2003, Karol Szwed <kszwed@kde.org>
 */

#include "geometrytip.h"
#include "options.h"
#include <X11/Xlib.h>

using namespace KWinInternal;

extern Options* options;


GeometryTip::GeometryTip( const Client* client, const XSizeHints* xSizeHints, bool resizing ):
    QLabel(NULL, "kwingeometry", WStyle_Customize | WStyle_StaysOnTop |
		    		 WStyle_NoBorder  | WX11BypassWM )
{
    // Enable SaveUnder so that we don't get those nasty unpainted 
    // areas when moving/resizing the window in non-opaque modes.
    // For this to work properly have 'Option "backingstore"' set in the Screen
    // section of your XF86Config. (some drivers may not support saveunder, 
    // oh well, we tried.)

    // Only do this in transparent mode, as it slows down opaque mode...
    if ( (resizing  && options->resizeMode == Options::Transparent) ||
	 (!resizing && options->moveMode == Options::Transparent) )
    {
	XSetWindowAttributes wsa;
	wsa.save_under = True;
	XChangeWindowAttributes( qt_xdisplay(), winId(), CWSaveUnder, &wsa );
    }

    c = client;
    setMargin(1);
    setIndent(0);
    setLineWidth(1);
    setFrameStyle( QFrame::Raised | QFrame::StyledPanel );
    setAlignment( AlignCenter | AlignTop );
    sizeHints = xSizeHints;
}

GeometryTip::~GeometryTip()
{
}

void GeometryTip::setGeometry( const QRect& geom )
{
    int w, h;
    int bw, bh;
    QWidget* wrap = c->windowWrapper();

    w = wrap->width();
    h = wrap->height();

    if (sizeHints) {
	if (!(sizeHints->flags & PBaseSize)) {
	    bw = 0;
	    bh = 0;
	} else {
	    bw = sizeHints->base_width;
	    bh = sizeHints->base_height;
	}

	if (sizeHints->flags & PResizeInc) {
	    if (sizeHints->width_inc > 0)
		w = (w - bw) / sizeHints->width_inc;
	    if (sizeHints->height_inc > 0) 
		h = (h - bh) / sizeHints->height_inc; 

	}
    }

    QString pos;
    pos.sprintf( "%+d,%+d&nbsp;(<b>%d&nbsp;x&nbsp;%d</b>)",
    		 geom.x(), geom.y(), w, h );
    setText( pos );
    adjustSize();
    move( geom.x() + ((geom.width()  - width())  / 2),
	  geom.y() + ((geom.height() - height()) / 2) );
}

#include "geometrytip.moc"
