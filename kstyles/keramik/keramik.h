/* Keramik Style for KDE3
   Copyright (c) 2002 Malte Starostik <malte@kde.org>

   based on the KDE3 HighColor Style

   Copyright (C) 2001-2002 Karol Szwed      <gallium@kde.org>
             (C) 2001-2002 Fredrik H�glund  <fredrik@kde.org> 
 
   Drawing routines adapted from the KDE2 HCStyle,
   Copyright (C) 2000 Daniel M. Duley       <mosfet@kde.org>
             (C) 2000 Dirk Mueller          <mueller@kde.org>
             (C) 2001 Martijn Klingens      <mklingens@yahoo.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

// $Id$

#ifndef __keramik_h__
#define __keramik_h__

#include <kstyle.h>


class KeramikStyle : public KStyle
{
	Q_OBJECT

public:
	KeramikStyle();
	virtual ~KeramikStyle();

	void renderMenuBlendPixmap( KPixmap& pix, const QColorGroup &cg, const QPopupMenu* ) const;
	QPixmap stylePixmap(StylePixmap stylepixmap, const QWidget* widget, const QStyleOption& opt) const;

	void polish( QWidget* widget );
	void unPolish( QWidget* widget );
	void polish( QPalette& );

	void drawKStylePrimitive( KStylePrimitive kpe,
	                          QPainter* p,
	                          const QWidget* widget,
	                          const QRect& r,
	                          const QColorGroup& cg,
	                          SFlags flags = Style_Default,
	                          const QStyleOption& = QStyleOption::Default ) const;
		
	void drawPrimitive( PrimitiveElement pe,
	                    QPainter* p,
	                    const QRect& r,
	                    const QColorGroup& cg,
	                    SFlags flags = Style_Default,
	                    const QStyleOption& = QStyleOption::Default ) const;

	void drawControl( ControlElement element,
	                  QPainter* p,
	                  const QWidget* widget,
	                  const QRect& r,
	                  const QColorGroup& cg,
	                  SFlags flags = Style_Default,
	                  const QStyleOption& opt = QStyleOption::Default ) const;

	void drawControlMask( ControlElement element,
	                      QPainter* p,
	                      const QWidget* widget,
	                      const QRect& r,
	                      const QStyleOption& opt = QStyleOption::Default ) const;

	void drawComplexControl( ComplexControl control,
	                         QPainter* p,
	                         const QWidget* widget,
	                         const QRect& r,
	                         const QColorGroup& cg,
	                         SFlags flags = Style_Default,
	                         SCFlags controls = SC_All,
	                         SCFlags active = SC_None,
	                         const QStyleOption& = QStyleOption::Default ) const;

	void drawComplexControlMask( ComplexControl control,
	                             QPainter* p,
	                             const QWidget* widget,
	                             const QRect& r,
	                             const QStyleOption& = QStyleOption::Default ) const;

	int pixelMetric( PixelMetric m, const QWidget* widget = 0 ) const;
		
	QSize sizeFromContents( ContentsType contents,
	                        const QWidget* widget,
	                        const QSize& contentSize,
	                        const QStyleOption& opt ) const;

	SubControl querySubControl( ComplexControl control,
	                            const QWidget* widget,
	                            const QPoint& point,
						        const QStyleOption& opt = QStyleOption::Default ) const;

	QRect querySubControlMetrics( ComplexControl control,
	                              const QWidget* widget,
	                              SubControl subcontrol,
	                              const QStyleOption& opt = QStyleOption::Default ) const;
	// Fix Qt3's wacky image positions
/*	QPixmap stylePixmap( StylePixmap stylepixmap,
					const QWidget* widget = 0,
					const QStyleOption& = QStyleOption::Default ) const;*/

protected:
	bool eventFilter( QObject* object, QEvent* event );
	void renderGradient( QPainter* p, const QRect& r, QColor clr, bool horizontal, int px=0, int py=0, int pwidth=-1, int pheight=-1, bool reverse = false ) const;
	
private:
	QRect subRect(SubRect r, const QWidget *widget) const;
	
	// Disable copy constructor and = operator
	KeramikStyle( const KeramikStyle&  );
	KeramikStyle& operator=( const KeramikStyle&  );
};

#endif

// vim: ts=4 sw=4 noet
