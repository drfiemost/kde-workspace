#include <kconfig.h>
#include "kwmthemeclient.h"
#include <kglobal.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qabstractlayout.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qdrawutil.h>
#include <kpixmapeffect.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <qbitmap.h>
#include <qtooltip.h>
#include "../../workspace.h"
#include "../../options.h"

extern "C"
{
    Client *allocate(Workspace *ws, WId w)
    {
        return(new KWMThemeClient(ws, w));
    }
}



enum FramePixmap{FrameTop=0, FrameBottom, FrameLeft, FrameRight, FrameTopLeft,
   FrameTopRight, FrameBottomLeft, FrameBottomRight};

static QPixmap *framePixmaps[8];
static QPixmap *menuPix, *iconifyPix, *closePix, *maxPix, *minmaxPix,
    *pinupPix, *pindownPix;
static KPixmap *aTitlePix = 0;
static KPixmap *iTitlePix = 0;
static KPixmapEffect::GradientType grType;
static int maxExtent, titleAlign;
static bool titleGradient = true;
static bool pixmaps_created = false;
static bool titleSunken = false;
static bool titleTransparent;

extern Options *options;

static void init_theme()
{
    const char *keys[] = {"wm_top", "wm_bottom", "wm_left", "wm_right",
    "wm_topleft", "wm_topright", "wm_bottomleft", "wm_bottomright"};

    if(pixmaps_created)
        return;
    pixmaps_created = true;
    
    KConfig *config = KGlobal::config();
    config->setGroup("General");
    QString tmpStr;

    for(int i=0; i < 8; ++i){
        framePixmaps[i] = new QPixmap(locate("appdata",
                                      "pics/"+config->readEntry(keys[i], " ")));
		if(framePixmaps[i]->isNull())
            qWarning("Unable to load frame pixmap for %s", keys[i]);
        else
            qWarning("Loaded pixmap %d", i+1);
    }
    maxExtent = framePixmaps[FrameTop]->height();
    if(framePixmaps[FrameBottom]->height() > maxExtent)
        maxExtent = framePixmaps[FrameBottom]->height();
    if(framePixmaps[FrameLeft]->width() > maxExtent)
        maxExtent = framePixmaps[FrameLeft]->width();
    if(framePixmaps[FrameRight]->width() > maxExtent)
        maxExtent = framePixmaps[FrameRight]->width();

    menuPix = new QPixmap(locate("appdata",
                                 "pics/"+config->readEntry("menu", " ")));
    iconifyPix = new QPixmap(locate("appdata",
                                    "pics/"+config->readEntry("iconify", " ")));
    maxPix = new QPixmap(locate("appdata",
                                "pics/"+config->readEntry("maximize", " ")));
    minmaxPix = new QPixmap(locate("appdata",
                                   "pics/"+config->readEntry("maximizedown", " ")));
    closePix = new QPixmap(locate("appdata",
                                  "pics/"+config->readEntry("close", " ")));
    pinupPix = new QPixmap(locate("appdata",
                                  "pics/"+config->readEntry("pinup", " ")));
    pindownPix = new QPixmap(locate("appdata",
                                    "pics/"+config->readEntry("pindown", " ")));
    if(menuPix->isNull())                           
        menuPix->load(locate("appdata", "pics/menu.png"));
    if(iconifyPix->isNull())
        iconifyPix->load(locate("appdata", "pics/iconify.png"));
    if(maxPix->isNull())
        maxPix->load(locate("appdata", "pics/maximize.png"));
    if(minmaxPix->isNull())
        minmaxPix->load(locate("appdata", "pics/maximizedown.png"));
    if(closePix->isNull())
        closePix->load(locate("appdata", "pics/close.png"));
    if(pinupPix->isNull())
        pinupPix->load(locate("appdata", "pics/pinup.png"));
    if(pindownPix->isNull())
        pindownPix->load(locate("appdata", "pics/pindown.png"));
    
    tmpStr = config->readEntry("TitleAlignment");
    if(tmpStr == "right")
        titleAlign = Qt::AlignRight | Qt::AlignVCenter;
    else if(tmpStr == "middle")
        titleAlign = Qt::AlignCenter;
    else
        titleAlign = Qt::AlignLeft | Qt::AlignVCenter;
    titleSunken = config->readBoolEntry("TitleFrameShaded", true);
    titleSunken = true; // FIXME
    titleTransparent = config->readBoolEntry("PixmapUnderTitleText", true);

    tmpStr = config->readEntry("TitlebarLook");
    if(tmpStr == "shadedVertical"){
        aTitlePix = new KPixmap;
        aTitlePix->resize(32, 20);
        KPixmapEffect::gradient(*aTitlePix,
                                options->color(Options::TitleBar, true),
                                options->color(Options::TitleBlend, true),
                                KPixmapEffect::VerticalGradient);
        iTitlePix = new KPixmap;
        iTitlePix->resize(32, 20);
        KPixmapEffect::gradient(*iTitlePix,
                                options->color(Options::TitleBar, false),
                                options->color(Options::TitleBlend, false),
                                KPixmapEffect::VerticalGradient);
        titleGradient = false; // we can just tile this

    }
    else if(tmpStr == "shadedHorizontal")
        grType = KPixmapEffect::HorizontalGradient;
    else if(tmpStr == "shadedDiagonal")
        grType = KPixmapEffect::DiagonalGradient;
    else if(tmpStr == "shadedCrossDiagonal")
        grType = KPixmapEffect::CrossDiagonalGradient;
    else if(tmpStr == "shadedPyramid")
        grType = KPixmapEffect::PyramidGradient;
    else if(tmpStr == "shadedRectangle")
        grType = KPixmapEffect::RectangleGradient;
    else if(tmpStr == "shadedPipeCross")
        grType = KPixmapEffect::PipeCrossGradient;
    else if(tmpStr == "shadedElliptic")
        grType = KPixmapEffect::EllipticGradient;
    else{
        titleGradient = false;
        tmpStr = config->readEntry("TitlebarPixmapActive", "");
        if(!tmpStr.isEmpty()){
            aTitlePix = new KPixmap;
            aTitlePix->load(locate("appdata", "pics/" + tmpStr));
        }
        else
            aTitlePix = NULL;
        tmpStr = config->readEntry("TitlebarPixmapInactive", "");
        if(!tmpStr.isEmpty()){
            iTitlePix = new KPixmap;
            iTitlePix->load(locate("appdata", "pics/" + tmpStr));
        }
        else
            iTitlePix = NULL;
    }
}

void
KWMThemeClient::slotReset()
{
   if (!pixmaps_created) return;
   pixmaps_created = false;

    for(int i=0; i < 8; ++i)
        delete framePixmaps[i];

   delete menuPix;
   delete iconifyPix;
   delete closePix;
   delete maxPix;
   delete minmaxPix;
   delete pinupPix;
   delete pindownPix;
   delete aTitlePix;
   aTitlePix = 0;
   delete iTitlePix;
   iTitlePix = 0;

   titleGradient = true;
   pixmaps_created = false;
   titleSunken = false;

   init_theme();   
}

void MyButton::drawButtonLabel(QPainter *p)
{
    if(pixmap()){
        style().drawItem(p, 0, 0, width(), height(), AlignCenter, colorGroup(),
                         true, pixmap(), QString::null);
    }
}

KWMThemeClient::KWMThemeClient( Workspace *ws, WId w, QWidget *parent,
                            const char *name )
    : Client( ws, w, parent, name, WResizeNoErase | WNorthWestGravity)
{
    init_theme();
    connect(options, SIGNAL(resetClients()), this, SLOT(slotReset())); 
    QGridLayout *layout = new QGridLayout(this);
    layout->addColSpacing(0, maxExtent);
    layout->addColSpacing(2, maxExtent);

    layout->addRowSpacing(0, maxExtent);

    layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed,
                                    QSizePolicy::Expanding));

    layout->addWidget(windowWrapper(), 2, 1);
    layout->addRowSpacing(3, maxExtent);
    layout->setRowStretch(2, 10);
    layout->setColStretch(1, 10);
    
    QHBoxLayout* hb = new QHBoxLayout;
    layout->addLayout( hb, 1, 1 );

    KConfig *config = KGlobal::config();
    config->setGroup("Buttons");
    QString val;
    MyButton *btn;
    int i;
    static const char *defaultButtons[]={"Menu","Sticky","Off","Iconify",
        "Maximize","Close"};
    static const char keyOffsets[]={"ABCDEF"};
    for(i=0; i < 6; ++i){
        if(i == 3){
            titlebar = new QSpacerItem(10, 20, QSizePolicy::Expanding,
                               QSizePolicy::Minimum );
            hb->addItem( titlebar );
        }
        QString key("Button");
        key += QChar(keyOffsets[i]);
        val = config->readEntry(key, defaultButtons[i]);
        if(val == "Menu"){
            btn = new MyButton(this, "menu");
            btn->setPixmap(*menuPix);
            hb->addWidget(btn);
            btn->setFixedSize(20, 20);
        }
        else if(val == "Sticky"){
            stickyBtn = new MyButton(this, "sticky");
            stickyBtn->setPixmap(*pinupPix);
            connect(stickyBtn, SIGNAL( clicked() ), this, SLOT(toggleSticky()));
            hb->addWidget(stickyBtn);
            stickyBtn->setFixedSize(20, 20);
        }
        else if(val == "Iconify"){
            btn = new MyButton(this, "iconify");
            btn->setPixmap(*iconifyPix);
            connect(btn, SIGNAL(clicked()), this, SLOT(iconify()));
            hb->addWidget(btn);
            btn->setFixedSize(20, 20);
        }
        else if(val == "Maximize"){
            maxBtn = new MyButton(this, "max");
            maxBtn->setPixmap(*maxPix);
            connect(maxBtn, SIGNAL(clicked()), this, SLOT(maximize()));
            hb->addWidget(maxBtn);
            maxBtn->setFixedSize(20, 20);
        }
        else if(val == "Close"){
            btn = new MyButton(this, "close");
            btn->setPixmap(*closePix);
            connect(btn, SIGNAL(clicked()), this, SLOT(closeWindow()));
            hb->addWidget(btn);
            btn->setFixedSize(20, 20);
        }
        else{
            if(val != "Off")
                qWarning("KWin: Unrecognized button value: %s", val.latin1());
        }
    }
    if(titleGradient){
        aGradient = new KPixmap;
        iGradient = new KPixmap;
    }
    else{
        aGradient = 0;
        iGradient = 0;
    }
        
    setBackgroundMode(NoBackground);
}

void KWMThemeClient::drawTitle(QPainter &p)
{
    QRect r = titlebar->geometry();

    if(titleSunken){
        qWarning("Title is sunken");
        qDrawShadeRect(&p, r, options->colorGroup(Options::Frame, isActive()),
                       true, 1, 0);
        r.setRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2);
    }
    else
        qWarning("Title is not sunken");
    
    KPixmap *fill = isActive() ? aTitlePix : iTitlePix;
    if(fill)
        p.drawTiledPixmap(r, *fill);
    else if(titleGradient){
        fill = isActive() ? aGradient : iGradient;
        if(fill->width() != r.width()){
            fill->resize(r.width(), 20);
            KPixmapEffect::gradient(*fill,
                                    options->color(Options::TitleBar, isActive()),
                                    options->color(Options::TitleBlend, isActive()),
                                    grType);
        }
        p.drawTiledPixmap(r, *fill);
    }
    else{
        p.fillRect(r, options->colorGroup(Options::TitleBar, isActive()).
                   brush(QColorGroup::Button));
    }
    p.setFont(options->font(isActive()));
    p.setPen(options->color(Options::Font, isActive()));
    p.drawText(r, titleAlign, caption());
}

void KWMThemeClient::resizeEvent( QResizeEvent* e)
{
    Client::resizeEvent( e );
    doShape();
    if ( isVisibleToTLW() && !testWFlags( WNorthWestGravity )) {
        QPainter p( this );
	QRect t = titlebar->geometry();
        t.setTop( 0 );
        QRegion r = rect();
	r = r.subtract( t );
        p.setClipRegion( r );
        p.eraseRect( rect() );
    }
}

void KWMThemeClient::captionChange( const QString& )
{
    repaint( titlebar->geometry(), false );
}

void KWMThemeClient::paintEvent( QPaintEvent* )
{
    QPainter p;
    p.begin(this);

    // first the corners
    int w1 = framePixmaps[FrameTopLeft]->width();
    int h1 = framePixmaps[FrameTopLeft]->height();
    if (w1 > width()/2) w1 = width()/2;
    if (h1 > height()/2) h1 = height()/2;
    bitBlt(this, 0,0, framePixmaps[FrameTopLeft],
           0,0, w1, h1);

    int w2 = framePixmaps[FrameTopRight]->width();
    int h2 = framePixmaps[FrameTopRight]->height();
    if (w2 > width()/2) w2 = width()/2;
    if (h2 > height()/2) h2 = height()/2;
    bitBlt(this, width()-w2,0, framePixmaps[FrameTopRight],
           framePixmaps[FrameTopRight]->width()-w2,0,w2, h2);

    int w3 = framePixmaps[FrameBottomLeft]->width();
    int h3 = framePixmaps[FrameBottomLeft]->height();
    if (w3 > width()/2) w3 = width()/2;
    if (h3 > height()/2) h3 = height()/2;
    bitBlt(this, 0,height()-h3, framePixmaps[FrameBottomLeft],
           0,framePixmaps[FrameBottomLeft]->height()-h3,w3, h3);

    int w4 = framePixmaps[FrameBottomRight]->width();
    int h4 = framePixmaps[FrameBottomRight]->height();
    if (w4 > width()/2) w4 = width()/2;
    if (h4 > height()/2) h4 = height()/2;
    bitBlt(this, width()-w4,height()-h4, framePixmaps[FrameBottomRight],
           framePixmaps[FrameBottomRight]->width()-w4,
           framePixmaps[FrameBottomRight]->height()-h4,
           w4, h4);

    QPixmap *curPix = framePixmaps[FrameTop];
    p.drawTiledPixmap(w1, maxExtent-curPix->height()-1, width()-w2-w1,
                      curPix->height(), *curPix);
    curPix = framePixmaps[FrameBottom];
    p.drawTiledPixmap(w3, height()-maxExtent+1, width()-w3-w4,
                      curPix->height(), *curPix);
    curPix = framePixmaps[FrameLeft];
    p.drawTiledPixmap(maxExtent-curPix->width()-1, h1, curPix->width(),
                      height()-h1-h3, *curPix);

    curPix = framePixmaps[FrameRight];
    p.drawTiledPixmap(width()-maxExtent+1, h2, curPix->width(),
                      height()-h2-h4, *curPix);
    drawTitle(p);
    p.end();
}

void KWMThemeClient::doShape()
{
    QBitmap mask(width(), height());
    mask.fill(color0);
    QPainter p;

    // first the corners
    int w1 = framePixmaps[FrameTopLeft]->width();
    int h1 = framePixmaps[FrameTopLeft]->height();
    if (w1 > width()/2) w1 = width()/2;
    if (h1 > height()/2) h1 = height()/2;
    bitBlt(&mask, 0,0,framePixmaps[FrameTopLeft]->mask(),
           0,0,w1, h1);
    int w2 = framePixmaps[FrameTopRight]->width();
    int h2 = framePixmaps[FrameTopRight]->height();
    if (w2 > width()/2) w2 = width()/2;
    if (h2 > height()/2) h2 = height()/2;
    bitBlt(&mask, width()-w2,0,framePixmaps[FrameTopRight]->mask(),
           framePixmaps[FrameTopRight]->width()-w2,0,w2, h2);

    int w3 = framePixmaps[FrameBottomLeft]->width();
    int h3 = framePixmaps[FrameBottomLeft]->height();
    if (w3 > width()/2) w3 = width()/2;
    if (h3 > height()/2) h3 = height()/2;
    bitBlt(&mask, 0,height()-h3,framePixmaps[FrameBottomLeft]->mask(),
           0, framePixmaps[FrameBottomLeft]->height()-h3,w3, h3);

    int w4 = framePixmaps[FrameBottomRight]->width();
    int h4 = framePixmaps[FrameBottomRight]->height();
    if (w4 > width()/2) w4 = width()/2;
    if (h4 > height()/2) h4 = height()/2;
    bitBlt(&mask, width()-w4,height()-h4,framePixmaps[FrameBottomRight]->mask(),
           framePixmaps[FrameBottomRight]->width()-w4,
           framePixmaps[FrameBottomRight]->height()-h4,
           w4, h4);

    p.begin(&mask);
    const QBitmap *curPix = framePixmaps[FrameTop]->mask();
    p.drawTiledPixmap(w1, maxExtent-curPix->height()-1, width()-w2-w1,
                      curPix->height(), *curPix);
    curPix = framePixmaps[FrameBottom]->mask();
    p.drawTiledPixmap(w3, height()-maxExtent+1, width()-w3-w4,
                      curPix->height(), *curPix);
    curPix = framePixmaps[FrameLeft]->mask();
    p.drawTiledPixmap(maxExtent-curPix->width()-1, h1, curPix->width(),
                      height()-h1-h3, *curPix);
    curPix = framePixmaps[FrameRight]->mask();
    p.drawTiledPixmap(width()-maxExtent+1, h2, curPix->width(),
                      height()-h2-h4, *curPix);
    p.setBrush(color1);
    p.setPen(color1);
    p.fillRect(maxExtent-1, maxExtent-1, width()-2*maxExtent+2,
               height()-2*maxExtent+2, color1);
    p.end();
    setMask(mask);
}


void KWMThemeClient::showEvent(QShowEvent *ev)
{
    Client::showEvent(ev);
    doShape();
    repaint(false);
}

void KWMThemeClient::windowWrapperShowEvent( QShowEvent* )
{
    doShape();
}                                                                               

void KWMThemeClient::mouseDoubleClickEvent( QMouseEvent * e )
{
    if (titlebar->geometry().contains( e->pos() ) )
        setShade( !isShade() );
    workspace()->requestFocus( this );
}

void KWMThemeClient::stickyChange(bool on)
{
    stickyBtn->setPixmap(on ? *pinupPix : *pindownPix);
}

void KWMThemeClient::maximizeChange(bool m)
{
    maxBtn->setPixmap(m ? *minmaxPix : *maxPix);
}

Client::MousePosition KWMThemeClient::mousePosition(const QPoint &p) const
{
    MousePosition m = Client::mousePosition(p);
    // corners
    if(p.y() < framePixmaps[FrameTop]->height() &&
       p.x() < framePixmaps[FrameLeft]->width()){
        m = TopLeft;
    }
    else if(p.y() < framePixmaps[FrameTop]->height() &&
            p.x() > width()-framePixmaps[FrameRight]->width()){
        m = TopRight;
    }
    else if(p.y() > height()-framePixmaps[FrameBottom]->height() &&
            p.x() < framePixmaps[FrameLeft]->width()){
        m = BottomLeft;
    }
    else if(p.y() > height()-framePixmaps[FrameBottom]->height() &&
            p.x() > width()-framePixmaps[FrameRight]->width()){
        m = BottomRight;
    } // edges
    else if(p.y() < framePixmaps[FrameTop]->height())
        m = Top;
    else if(p.y() > height()-framePixmaps[FrameBottom]->height())
        m = Bottom;
    else if(p.x()  < framePixmaps[FrameLeft]->width())
        m = Left;
    else if(p.x() > width()-framePixmaps[FrameRight]->width())
        m = Right;
    return(m);
}

void KWMThemeClient::init()
{
    //
}


#include "kwmthemeclient.moc"
