/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#ifndef KWIN_TOPLEVEL_H
#define KWIN_TOPLEVEL_H

#include <assert.h>
#include <qobject.h>
#include <kdecoration.h>

#include "utils.h"

namespace KWinInternal
{

class Workspace;

class Toplevel
    : public QObject, public KDecorationDefines
    {
    Q_OBJECT
    public:
        Toplevel( Workspace *ws );
        Window handle() const;
        Workspace* workspace() const;
        QRect geometry() const;
        QSize size() const;
        QPoint pos() const;
        QRect rect() const;
        int x() const;
        int y() const;
        int width() const;
        int height() const;

    // prefer isXXX() instead
        virtual NET::WindowType windowType( bool direct = false, int supported_types = SUPPORTED_WINDOW_TYPES_MASK ) const = 0;
        bool hasNETSupport() const;
        bool isDesktop() const;
        bool isDock() const;
        bool isToolbar() const;
        bool isTopMenu() const;
        bool isMenu() const;
        bool isNormalWindow() const; // normal as in 'NET::Normal or NET::Unknown non-transient'
        bool isDialog() const;
        bool isSplash() const;
        bool isUtility() const;

        Pixmap windowPixmap( bool allow_create = true ); // for use with compositing
        Visual* visual() const;
        bool shape() const;
        virtual double opacity() const = 0;
        int depth() const;
        bool hasAlpha() const;
        void setupCompositing();
        void finishCompositing( bool discard_pixmap = true );
        void addDamage( const QRect& r );
        void addDamage( int x, int y, int w, int h );
        void addDamageFull();
        QRegion damage() const;
        void resetDamage( const QRect& r );

        // used by effects to keep the window around for e.g. fadeout effects when it's destroyed
        void refWindow();
        virtual void unrefWindow() = 0;
        bool deleting() const;

    protected:
        virtual ~Toplevel();
        void setHandle( Window id );
        void detectShape( Window id );
        void damageNotifyEvent( XDamageNotifyEvent* e );
        Pixmap createWindowPixmap() const;
        void discardWindowPixmap();
        QRect geom;
        Visual* vis;
        int bit_depth;
        int delete_refcount;
        virtual void debug( kdbgstream& stream ) const = 0;
        friend kdbgstream& operator<<( kdbgstream& stream, const Toplevel* );
    private:
        Window id;
        Workspace* wspace;
        Pixmap window_pix;
        Damage damage_handle;
        QRegion damage_region;
        bool is_shape;
    };

inline Window Toplevel::handle() const
    {
    return id;
    }

inline void Toplevel::setHandle( Window w )
    {
    assert( id == None && w != None );
    id = w;
    }

inline Workspace* Toplevel::workspace() const
    {
    return wspace;
    }

inline QRect Toplevel::geometry() const
    {
    return geom;
    }

inline QSize Toplevel::size() const
    {
    return geom.size();
    }

inline QPoint Toplevel::pos() const
    {
    return geom.topLeft();
    }

inline int Toplevel::x() const
    {
    return geom.x();
    }

inline int Toplevel::y() const
    {
    return geom.y();
    }

inline int Toplevel::width() const
    {
    return geom.width();
    }

inline int Toplevel::height() const
    {
    return geom.height();
    }

inline QRect Toplevel::rect() const
    {
    return QRect( 0, 0, width(), height());
    }

inline Visual* Toplevel::visual() const
    {
    return vis;
    }

inline bool Toplevel::isDesktop() const
    {
    return windowType() == NET::Desktop;
    }

inline bool Toplevel::isDock() const
    {
    return windowType() == NET::Dock;
    }

inline bool Toplevel::isTopMenu() const
    {
    return windowType() == NET::TopMenu;
    }

inline bool Toplevel::isMenu() const
    {
    return windowType() == NET::Menu && !isTopMenu(); // because of backwards comp.
    }

inline bool Toplevel::isToolbar() const
    {
    return windowType() == NET::Toolbar;
    }

inline bool Toplevel::isSplash() const
    {
    return windowType() == NET::Splash;
    }

inline bool Toplevel::isUtility() const
    {
    return windowType() == NET::Utility;
    }

inline bool Toplevel::isDialog() const
    {
    return windowType() == NET::Dialog;
    }

inline bool Toplevel::isNormalWindow() const
    {
    return windowType() == NET::Normal;
    }

inline Pixmap Toplevel::windowPixmap( bool allow_create )
    {
    if( window_pix == None && allow_create )
        window_pix = createWindowPixmap();
    return window_pix;
    }

inline QRegion Toplevel::damage() const
    {
    return damage_region;
    }

inline bool Toplevel::shape() const
    {
    return is_shape;
    }

inline int Toplevel::depth() const
    {
    return bit_depth;
    }

inline bool Toplevel::hasAlpha() const
    {
    return depth() == 32;
    }

inline void Toplevel::refWindow()
    {
    assert( delete_refcount >= 0 );
    ++delete_refcount;
    }

inline bool Toplevel::deleting() const
    {
    return delete_refcount >= 0;
    }

#ifdef NDEBUG
inline
kndbgstream& operator<<( kndbgstream& stream, const Toplevel* ) { return stream; }
inline
kndbgstream& operator<<( kndbgstream& stream, const ToplevelList& ) { return stream; }
inline
kndbgstream& operator<<( kndbgstream& stream, const ConstToplevelList& ) { return stream; }
#else
kdbgstream& operator<<( kdbgstream& stream, const Toplevel* );
kdbgstream& operator<<( kdbgstream& stream, const ToplevelList& );
kdbgstream& operator<<( kdbgstream& stream, const ConstToplevelList& );
#endif

KWIN_COMPARE_PREDICATE( HandleMatchPredicate, Toplevel, Window, cl->handle() == value );

} // namespace

#endif
