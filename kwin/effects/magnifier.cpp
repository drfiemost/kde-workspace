/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#include <config-X11.h> // HAVE_OPENGL

#include "magnifier.h"

#include <kaction.h>
#include <kactioncollection.h>
#include <kstandardaction.h>

#ifdef HAVE_OPENGL
#include <GL/gl.h>
#endif

namespace KWin
{

KWIN_EFFECT( magnifier, MagnifierEffect )

const int FRAME_WIDTH = 5;

MagnifierEffect::MagnifierEffect()
    : zoom( 1 )
    , target_zoom( 1 )
    {
    KActionCollection* actionCollection = new KActionCollection( this );
    KAction* a;
    a = static_cast< KAction* >( actionCollection->addAction( KStandardAction::ZoomIn, this, SLOT( zoomIn())));
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Equal));
    a = static_cast< KAction* >( actionCollection->addAction( KStandardAction::ZoomOut, this, SLOT( zoomOut())));
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Minus));
    a = static_cast< KAction* >( actionCollection->addAction( KStandardAction::ActualSize, this, SLOT( toggle())));
    a->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_0));
    magnifier_size = QSize( 200, 200 ); // TODO config option
    }

void MagnifierEffect::prePaintScreen( ScreenPrePaintData& data, int time )
    {
    if( zoom != target_zoom )
        {
        float diff = time / 500.0;
        if( target_zoom > zoom )
            zoom = qMin( zoom * qMax( 1 + diff, 1.2f ), target_zoom );
        else
            zoom = qMax( zoom * qMin( 1 - diff, 0.8f ), target_zoom );
        }
    effects->prePaintScreen( data, time );
    if( zoom != 1.0 )
        data.paint |= magnifierArea().adjusted( -FRAME_WIDTH, -FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH );
    }

void MagnifierEffect::paintScreen( int mask, QRegion region, ScreenPaintData& data )
    {
    ScreenPaintData data2 = data;
    effects->paintScreen( mask, region, data ); // paint normal screen
    if( zoom != 1.0 )
        { // paint magnifier
// ## TODO this should be inside HAVE_OPENGL
        glPushAttrib( GL_ENABLE_BIT );
        QRect area = magnifierArea();
        glEnable( GL_SCISSOR_TEST );
        int dh = displayHeight();
        // Scissor rect has to be given in OpenGL coords
        glScissor( area.x(), dh - area.y() - area.height(), area.width(), area.height());
        mask |= PAINT_SCREEN_TRANSFORMED;
        data2.xScale *= zoom;
        data2.yScale *= zoom;
        QPoint cursor = cursorPos();
        // set the position so that the cursor is in the same position in the scaled view
        data2.xTranslate = - int( cursor.x() * ( zoom - 1 ));
        data2.yTranslate = - int( cursor.y() * ( zoom - 1 ));
        effects->paintScreen( mask, region, data2 );
        glPopAttrib();
        glPushAttrib( GL_CURRENT_BIT );
        glColor4f( 0, 0, 0, 1 ); // black
        glBegin( GL_QUADS );
        glVertex2i( area.left() - FRAME_WIDTH, area.top() - FRAME_WIDTH ); // top frame
        glVertex2i( area.right() + FRAME_WIDTH, area.top() - FRAME_WIDTH );
        glVertex2i( area.right() + FRAME_WIDTH, area.top() - 1 );
        glVertex2i( area.left() - FRAME_WIDTH, area.top() - 1 );
        glVertex2i( area.left() - FRAME_WIDTH, area.top() - FRAME_WIDTH ); // left frame
        glVertex2i( area.left() - 1, area.top() - FRAME_WIDTH );
        glVertex2i( area.left() - 1, area.bottom() + FRAME_WIDTH );
        glVertex2i( area.left() - FRAME_WIDTH, area.bottom() + FRAME_WIDTH );
        glVertex2i( area.right() + 1, area.top() - FRAME_WIDTH ); // right frame
        glVertex2i( area.right() + FRAME_WIDTH, area.top() - FRAME_WIDTH );
        glVertex2i( area.right() + FRAME_WIDTH, area.bottom() + FRAME_WIDTH );
        glVertex2i( area.right() + 1, area.bottom() + FRAME_WIDTH );
        glVertex2i( area.left() - FRAME_WIDTH, area.bottom() + 1 ); // bottom frame
        glVertex2i( area.right() + FRAME_WIDTH, area.bottom() + 1 );
        glVertex2i( area.right() + FRAME_WIDTH, area.bottom() + FRAME_WIDTH );
        glVertex2i( area.left() - FRAME_WIDTH, area.bottom() + FRAME_WIDTH );
        glEnd();
        glPopAttrib();
        }
    }

void MagnifierEffect::postPaintScreen()
    {
    if( zoom != target_zoom )
        effects->addRepaint( magnifierArea());
    effects->postPaintScreen();
    }

QRect MagnifierEffect::magnifierArea( const QPoint &pos ) const
    {
    return QRect( pos.x() - magnifier_size.width() / 2, pos.y() - magnifier_size.height() / 2,
        magnifier_size.width(), magnifier_size.height());
    }

void MagnifierEffect::zoomIn()
    {
    target_zoom *= 1.2;
    effects->addRepaint( magnifierArea().adjusted( -FRAME_WIDTH, -FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH ));
    }

void MagnifierEffect::zoomOut()
    {
    target_zoom /= 1.2;
    if( target_zoom < 1 )
        target_zoom = 1;
    effects->addRepaint( magnifierArea().adjusted( -FRAME_WIDTH, -FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH ));
    }

void MagnifierEffect::toggle()
    {
    if( target_zoom == 1.0 )
        target_zoom = 2;
    else
        target_zoom = 1;
    effects->addRepaint( magnifierArea().adjusted( -FRAME_WIDTH, -FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH ));
    }

void MagnifierEffect::mouseChanged( const QPoint& pos, const QPoint& old, Qt::MouseButtons, Qt::KeyboardModifiers )
    {
    if( pos != old && zoom != 1 )
        effects->addRepaint( magnifierArea( old ).adjusted( -FRAME_WIDTH, -FRAME_WIDTH, FRAME_WIDTH, FRAME_WIDTH ));
    }

} // namespace

#include "magnifier.moc"
