/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 1997 to 2002 Cristian Tibirna <tibirna@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#include "workspace.h"
#include "client.h"
#include "options.h"
#include "placement.h"

#include <qrect.h>


namespace KWinInternal
{

Placement::Placement(Workspace* w)
    {
    m_WorkspacePtr = w;

    //CT initialize the cascading info
    for( int i = 0; i < m_WorkspacePtr->numberOfDesktops(); i++) 
        {
        DesktopCascadingInfo inf;
        inf.pos = QPoint(0,0);
        inf.col = 0;
        inf.row = 0;
        cci.append(inf);
        }

    }

/*!
  Places the client \a c according to the workspace's layout policy
 */
void Placement::place(Client* c)
    {
    if( c->isUtility())
        placeUtility(c);
    else if( c->isDialog())
        placeDialog(c);
    else if( c->isSplash())
        placeOnMainWindow( c ); // on mainwindow, if any, otherwise centered
    else
        placeInternal(c);
    }

void Placement::placeInternal(Client* c)
    {
    if (options->placement == Options::Random)               placeAtRandom(c);
    else if (options->placement == Options::Cascade)              placeCascaded(c);
    else if (options->placement == Options::Centered)     placeCentered(c);
    else if (options->placement == Options::ZeroCornered) placeZeroCornered(c);
    else                                                          placeSmart(c);
    }

/*!
  Place the client \a c according to a simply "random" placement algorithm.
 */
void Placement::placeAtRandom(Client* c)
    {
    const int step  = 24;
    static int px = step;
    static int py = 2 * step;
    int tx,ty;

    const QRect maxRect = m_WorkspacePtr->clientArea(Workspace::PlacementArea); // TODO use desktop number

    if (px < maxRect.x())
        px = maxRect.x();
    if (py < maxRect.y())
        py = maxRect.y();

    px += step;
    py += 2*step;

    if (px > maxRect.width()/2)
        px =  maxRect.x() + step;
    if (py > maxRect.height()/2)
        py =  maxRect.y() + step;
    tx = px;
    ty = py;
    if (tx + c->width() > maxRect.right())
        {
        tx = maxRect.right() - c->width();
        if (tx < 0)
            tx = 0;
        px =  maxRect.x();
        }
    if (ty + c->height() > maxRect.bottom())
        {
        ty = maxRect.bottom() - c->height();
        if (ty < 0)
            ty = 0;
        py =  maxRect.y();
        }
    c->move(tx, ty);
    }

/*!
  Place the client \a c according to a really smart placement algorithm :-)
*/
void Placement::placeSmart(Client* c)
    {
    /*
     * SmartPlacement by Cristian Tibirna (tibirna@kde.org)
     * adapted for kwm (16-19jan98) and for kwin (16Nov1999) using (with
     * permission) ideas from fvwm, authored by
     * Anthony Martin (amartin@engr.csulb.edu).
     * Xinerama supported added by Balaji Ramani (balaji@yablibli.com)
     * with ideas from xfce.
     */

    const int none = 0, h_wrong = -1, w_wrong = -2; // overlap types
    long int overlap, min_overlap = 0;
    int x_optimal, y_optimal;
    int possible;
    int desktop = c->desktop() == 0 || c->isOnAllDesktops() ? m_WorkspacePtr->currentDesktop() : c->desktop();

    int cxl, cxr, cyt, cyb;     //temp coords
    int  xl,  xr,  yt,  yb;     //temp coords
    int basket;                 //temp holder

    // get the maximum allowed windows space
    const QRect maxRect = m_WorkspacePtr->clientArea(Workspace::PlacementArea);
    int x = maxRect.left(), y = maxRect.top();
    x_optimal = x; y_optimal = y;

    //client gabarit
    int ch = c->height() - 1;
    int cw = c->width()  - 1;

    bool first_pass = true; //CT lame flag. Don't like it. What else would do?

    //loop over possible positions
    do 
        {
        //test if enough room in x and y directions
        if (y + ch > maxRect.bottom() && ch < maxRect.height())
            overlap = h_wrong; // this throws the algorithm to an exit
        else if(x + cw > maxRect.right())
            overlap = w_wrong;
        else 
            {
            overlap = none; //initialize

            cxl = x; cxr = x + cw;
            cyt = y; cyb = y + ch;
            ClientList::ConstIterator l;
            for(l = m_WorkspacePtr->stackingOrder().begin(); l != m_WorkspacePtr->stackingOrder().end() ; ++l) 
                {
                if((*l)->isOnDesktop(desktop) &&
                   (*l)->isShown() && (*l) != c) 
                    {

                    xl = (*l)->x();          yt = (*l)->y();
                    xr = xl + (*l)->width(); yb = yt + (*l)->height();

                    //if windows overlap, calc the overall overlapping
                    if((cxl < xr) && (cxr > xl) &&
                       (cyt < yb) && (cyb > yt)) 
                        {
                        xl = QMAX(cxl, xl); xr = QMIN(cxr, xr);
                        yt = QMAX(cyt, yt); yb = QMIN(cyb, yb);
                        if((*l)->keepAbove())
                            overlap += 16 * (xr - xl) * (yb - yt);
                        else if((*l)->keepBelow())
                            overlap += ((xr - xl) * (yb - yt))/4;
                        else
                            overlap += (xr - xl) * (yb - yt);
                        }
                    }
                }
            }

        //CT first time we get no overlap we stop.
        if (overlap == none) 
            {
            x_optimal = x;
            y_optimal = y;
            break;
            }

        if (first_pass) 
            {
            first_pass = false;
            min_overlap = overlap;
            }
        //CT save the best position and the minimum overlap up to now
        else if (overlap >= none && overlap < min_overlap) 
            {
            min_overlap = overlap;
            x_optimal = x;
            y_optimal = y;
            }

        // really need to loop? test if there's any overlap
        if (overlap > none) 
            {

            possible = maxRect.right();
            if (possible - cw > x) possible -= cw;

            // compare to the position of each client on the same desk
            ClientList::ConstIterator l;
            for(l = m_WorkspacePtr->stackingOrder().begin(); l != m_WorkspacePtr->stackingOrder().end() ; ++l) 
                {

                if ((*l)->isOnDesktop(desktop) &&
                     (*l)->isShown() &&  (*l) != c) 
                    {

                    xl = (*l)->x();          yt = (*l)->y();
                    xr = xl + (*l)->width(); yb = yt + (*l)->height();

                    // if not enough room above or under the current tested client
                    // determine the first non-overlapped x position
                    if((y < yb) && (yt < ch + y)) 
                        {

                        if((xr > x) && (possible > xr)) possible = xr;

                        basket = xl - cw;
                        if((basket > x) && (possible > basket)) possible = basket;
                        }
                    }
                }
            x = possible;
            }

        // ... else ==> not enough x dimension (overlap was wrong on horizontal)
        else if (overlap == w_wrong) 
            {
            x = maxRect.left();
            possible = maxRect.bottom();

            if (possible - ch > y) possible -= ch;

            //test the position of each window on the desk
            ClientList::ConstIterator l;
            for(l = m_WorkspacePtr->stackingOrder().begin(); l != m_WorkspacePtr->stackingOrder().end() ; ++l) 
                {
                if((*l)->isOnDesktop(desktop) &&
                    (*l) != c   &&  c->isShown()) 
                    {

                    xl = (*l)->x();          yt = (*l)->y();
                    xr = xl + (*l)->width(); yb = yt + (*l)->height();

                    // if not enough room to the left or right of the current tested client
                    // determine the first non-overlapped y position
                    if((yb > y) && (possible > yb)) possible = yb;

                    basket = yt - ch;
                    if((basket > y) && (possible > basket)) possible = basket;
                    }
                }
            y = possible;
            }
        }
    while((overlap != none) && (overlap != h_wrong) && (y < maxRect.bottom()));

    if(ch>= maxRect.height())
        y_optimal=maxRect.top();

    // place the window
    c->move(x_optimal, y_optimal);

    }

/*!
  Place windows in a cascading order, remembering positions for each desktop
*/
void Placement::placeCascaded (Client* c, bool re_init)
    {
    /* cascadePlacement by Cristian Tibirna (tibirna@kde.org) (30Jan98)
     */
    // work coords
    int xp, yp;

    //CT how do I get from the 'Client' class the size that NW squarish "handle"
    const int delta_x = 24;
    const int delta_y = 24;

    const int dn = c->desktop() == 0 || c->isOnAllDesktops() ? (m_WorkspacePtr->currentDesktop() - 1) : (c->desktop() - 1);

    // get the maximum allowed windows space and desk's origin
    //    (CT 20Nov1999 - is this common to all desktops?)
    QRect maxRect = m_WorkspacePtr->clientArea(Workspace::PlacementArea);

    // initialize often used vars: width and height of c; we gain speed
    const int ch = c->height();
    const int cw = c->width();
    const int X = maxRect.left();
    const int Y = maxRect.top();
    const int H = maxRect.height();
    const int W = maxRect.width();

  //initialize if needed
    if (re_init || cci[dn].pos.x() < X || cci[dn].pos.y() < Y ) 
        {
        cci[dn].pos = QPoint(X, Y);
        cci[dn].col = cci[dn].row = 0;
        }


    xp = cci[dn].pos.x();
    yp = cci[dn].pos.y();

    //here to touch in case people vote for resize on placement
    if ((yp + ch) > H) yp = Y;

    if ((xp + cw) > W)
        if (!yp) 
        {
        placeSmart(c);
        return;
        }
    else xp = X;

  //if this isn't the first window
    if (cci[dn].pos.x() != X && cci[dn].pos.y() != Y) 
        {
        /* The following statements cause an internal compiler error with
         * egcs-2.91.66 on SuSE Linux 6.3. The equivalent forms compile fine.
         * 22-Dec-1999 CS
         *
         * if (xp != X && yp == Y) xp = delta_x * (++(cci[dn].col));
         * if (yp != Y && xp == X) yp = delta_y * (++(cci[dn].row));
         */
    if (xp != X && yp == Y)
            {
            ++(cci[dn].col);
            xp = delta_x * cci[dn].col;
            }
        if (yp != Y && xp == X)
            {
            ++(cci[dn].row);
            yp = delta_y * cci[dn].row;
            }

        // last resort: if still doesn't fit, smart place it
        if (((xp + cw) > W - X) || ((yp + ch) > H - Y)) 
            {
            placeSmart(c);
            return;
            }
        }

    // place the window
    c->move(QPoint(xp, yp));

    // new position
    cci[dn].pos = QPoint(xp + delta_x,  yp + delta_y);
    }

/*!
  Place windows centered, on top of all others
*/
void Placement::placeCentered (Client* c)
    {

    // get the maximum allowed windows space and desk's origin
    //    (CT 20Nov1999 - is this common to all desktops?)
    const QRect maxRect = m_WorkspacePtr->clientArea(Workspace::PlacementArea);

    const int xp = maxRect.left() + (maxRect.width() -  c->width())  / 2;
    const int yp = maxRect.top()  + (maxRect.height() - c->height()) / 2;

    // place the window
    c->move(QPoint(xp, yp));
    }

/*!
  Place windows in the (0,0) corner, on top of all others
*/
void Placement::placeZeroCornered(Client* c)
    {
    // get the maximum allowed windows space and desk's origin
    //    (CT 20Nov1999 - is this common to all desktops?)
    const QRect maxRect = m_WorkspacePtr->clientArea(Workspace::PlacementArea);

    // place the window
    c->move(QPoint(maxRect.left(), maxRect.top()));
    }

void Placement::placeUtility(Client* c)
    {
// TODO kwin should try to place utility windows next to their mainwindow,
// preferably at the right edge, and going down if there are more of them
// if there's not enough place outside the mainwindow, it should prefer
// top-right corner
    // use the default placement for now
    placeInternal( c );
    }


void Placement::placeDialog(Client* c)
    {
    // if the dialog is actually non-NETWM transient window, don't apply placement to it,
    // it breaks with too many things (xmms, display)
    if( !c->hasNETSupport())
        return;
    placeOnMainWindow( c );
    }

void Placement::placeUnderMouse(Client* c)
    {
    QRect geom = c->geometry();
    geom.moveCenter( QCursor::pos());
    c->move( geom.topLeft());
    }

void Placement::placeOnMainWindow(Client* c)
    {
    ClientList mainwindows = c->mainClients();
    Client* place_on = NULL;
    for( ClientList::ConstIterator it = mainwindows.begin();
         it != mainwindows.end();
         ++it )
        {
        if( (*it)->isSpecialWindow() && !(*it)->isOverride())
            continue; // don't consider toolbars etc when placing
        if( (*it)->isOnCurrentDesktop())
            {
            if( place_on == NULL )
                place_on = *it;
            else
                { // two or more on current desktop -> center
                placeCentered( c );
                return;
                }
            }
        }
    if( place_on == NULL )
        {
        if( mainwindows.count() != 2 )
            {
            placeCentered( c );
            return;
            }
        place_on = mainwindows.first();
        }
    QRect geom = c->geometry();
    geom.moveCenter( place_on->geometry().center());
    c->move( geom.topLeft());
    }

// ********************
// Workspace
// ********************

/*!
  Moves active window left until in bumps into another window or workarea edge.
 */
void Workspace::slotWindowPackLeft()
    {
    if( active_client )
        active_client->move( packPositionLeft( active_client, active_client->geometry().left(), true ),
            active_client->y());
    }

void Workspace::slotWindowPackRight()
    {
    if( active_client )
        active_client->move( 
            packPositionRight( active_client, active_client->geometry().right(), true )
            - active_client->width() + 1, active_client->y());
    }

void Workspace::slotWindowPackUp()
    {
    if( active_client )
        active_client->move( active_client->x(),
            packPositionUp( active_client, active_client->geometry().top(), true ));
    }

void Workspace::slotWindowPackDown()
    {
    if( active_client )
        active_client->move( active_client->x(),
            packPositionDown( active_client, active_client->geometry().bottom(), true ) - active_client->height() + 1 );
    }

void Workspace::slotWindowGrowHorizontal()
    {
    if( active_client )
        active_client->growHorizontal();
    }

void Client::growHorizontal()
    {
    QRect geom = geometry();
    geom.setRight( workspace()->packPositionRight( this, geom.right(), true ));
    QSize adjsize = adjustedSize( geom.size());
    if( geometry().size() == adjsize && geom.size() != adjsize && xSizeHint.width_inc > 1 ) // take care of size increments
        {
        int newright = workspace()->packPositionRight( this, geom.right() + xSizeHint.width_inc - 1, true );
        if( workspace()->clientArea( Workspace::MovementArea, geometry().center(), desktop()).right() >= newright )
            geom.setRight( newright );
        }
    geom.setSize( adjustedSize( geom.size()));
    setGeometry( geom );
    }

void Workspace::slotWindowShrinkHorizontal()
    {
    if( active_client )
        active_client->shrinkHorizontal();
    }

void Client::shrinkHorizontal()
    {
    QRect geom = geometry();
    geom.setRight( workspace()->packPositionLeft( this, geom.right(), false ));
    if( geom.width() <= 1 )
        return;
    geom.setSize( adjustedSize( geom.size()));
    if( geom.width() > 20 )
        setGeometry( geom );
    }

void Workspace::slotWindowGrowVertical()
    {
    if( active_client )
        active_client->growVertical();
    }

void Client::growVertical()
    {
    QRect geom = geometry();
    geom.setBottom( workspace()->packPositionDown( this, geom.bottom(), true ));
    QSize adjsize = adjustedSize( geom.size());
    if( geometry().size() == adjsize && geom.size() != adjsize && xSizeHint.height_inc > 1 ) // take care of size increments
        {
        int newbottom = workspace()->packPositionDown( this, geom.bottom() + xSizeHint.height_inc - 1, true );
        if( workspace()->clientArea( Workspace::MovementArea, geometry().center(), desktop()).bottom() >= newbottom )
            geom.setBottom( newbottom );
        }
    geom.setSize( adjustedSize( geom.size()));
    setGeometry( geom );
    }


void Workspace::slotWindowShrinkVertical()
    {
    if( active_client )
        active_client->shrinkVertical();
    }

void Client::shrinkVertical()
    {
    QRect geom = geometry();
    geom.setBottom( workspace()->packPositionUp( this, geom.bottom(), false ));
    if( geom.height() <= 1 )
        return;
    geom.setSize( adjustedSize( geom.size()));
    if( geom.height() > 20 )
        setGeometry( geom );
    }

int Workspace::packPositionLeft( const Client* cl, int oldx, bool left_edge ) const
    {
    int newx = clientArea( MovementArea, cl->geometry().center(), cl->desktop()).left();
    if( oldx < newx )
        return oldx;
    for( ClientList::ConstIterator it = clients.begin();
         it != clients.end();
         ++it)
        {
        if( !(*it)->isShown() || !(*it)->isOnDesktop( active_client->desktop()))
            continue;
        int x = left_edge ? (*it)->geometry().right() + 1 : (*it)->geometry().left() - 1;
        if( x > newx && x < oldx
            && !( cl->geometry().top() > (*it)->geometry().bottom() // they overlap in Y direction
                || cl->geometry().bottom() < (*it)->geometry().top()))
            newx = x;
        }
    return newx;
    }

int Workspace::packPositionRight( const Client* cl, int oldx, bool right_edge ) const
    {
    int newx = clientArea( MovementArea, cl->geometry().center(), cl->desktop()).right();
    if( oldx > newx )
        return oldx;
    for( ClientList::ConstIterator it = clients.begin();
         it != clients.end();
         ++it)
        {
        if( !(*it)->isShown() || !(*it)->isOnDesktop( cl->desktop()))
            continue;
        int x = right_edge ? (*it)->geometry().left() - 1 : (*it)->geometry().right() + 1;
        if( x < newx && x > oldx
            && !( cl->geometry().top() > (*it)->geometry().bottom()
                || cl->geometry().bottom() < (*it)->geometry().top()))
            newx = x;
        }
    return newx;
    }

int Workspace::packPositionUp( const Client* cl, int oldy, bool top_edge ) const
    {
    int newy = clientArea( MovementArea, cl->geometry().center(), cl->desktop()).top();
    if( oldy < newy )
        return oldy;
    for( ClientList::ConstIterator it = clients.begin();
         it != clients.end();
         ++it)
        {
        if( !(*it)->isShown() || !(*it)->isOnDesktop( cl->desktop()))
            continue;
        int y = top_edge ? (*it)->geometry().bottom() + 1 : (*it)->geometry().top() - 1;
        if( y > newy && y < oldy
            && !( cl->geometry().left() > (*it)->geometry().right() // they overlap in X direction
                || cl->geometry().right() < (*it)->geometry().left()))
            newy = y;
        }
    return newy;
    }

int Workspace::packPositionDown( const Client* cl, int oldy, bool bottom_edge ) const
    {
    int newy = clientArea( MovementArea, cl->geometry().center(), cl->desktop()).bottom();
    if( oldy > newy )
        return oldy;
    for( ClientList::ConstIterator it = clients.begin();
         it != clients.end();
         ++it)
        {
        if( !(*it)->isShown() || !(*it)->isOnDesktop( cl->desktop()))
            continue;
        int y = bottom_edge ? (*it)->geometry().top() - 1 : (*it)->geometry().bottom() + 1;
        if( y < newy && y > oldy
            && !( cl->geometry().left() > (*it)->geometry().right()
                || cl->geometry().right() < (*it)->geometry().left()))
            newy = y;
        }
    return newy;
    }

/*!
  Asks the internal positioning object to place a client
*/
void Workspace::place(Client* c)
    {
    initPositioning->place(c);
    }

void Workspace::placeSmart(Client* c)
    {
    initPositioning->placeSmart(c);
    }


} // namespace
