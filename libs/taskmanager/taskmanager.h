/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef __taskmanager_h__
#define __taskmanager_h__

#include <qpoint.h>
#include <qobject.h>
#include <qvaluelist.h>
#include <qlist.h>
#include <qpixmap.h>

#include <kwin.h>

class Task: public QObject
{
    Q_OBJECT

public:
    Task( WId win, QObject * parent, const char *name = 0 );
    virtual ~Task();

    WId window() const { return _win; }
    QPixmap pixmap() { return _pixmap; }
    QString name() { return _info.name; }
    QString visibleName() { return _info.visibleName; }
    QString visibleNameWithState() { return _info.visibleNameWithState(); }

    // state
    bool maximized() const;
    bool iconified() const;
    bool onCurrentDesktop() const;
    bool staysOnTop() const;
    bool active() const;

    // actions
    void maximize();
    void restore();
    void iconify();
    void deiconify();
    void close();

    // internal
    void refresh(bool icon = false);
    void addTransient( WId w ) { _transients.append( w ); }
    void removeTransient( WId w ) { _transients.remove( w ); }
    bool hasTransient( WId w ) const { return _transients.contains( w ); }
    void setActive(bool a) { _active = a; }

signals:
    void changed();

private:
    bool                _active;
    WId                 _win;
    QPixmap             _pixmap;
    KWin::Info          _info;
    QValueList<WId>     _transients;
};

class TaskManager : public QObject
{
    Q_OBJECT

public:
    TaskManager( QObject *parent = 0, const char *name = 0 );
    virtual ~TaskManager();

    QList<Task> taskList() { return _tasks; }

signals:
    void changed();
    void desktopChanged(int desktop);

protected slots:
    void windowAdded(WId);
    void windowRemoved(WId);
    void windowChanged(WId, unsigned int);

    void activeWindowChanged(WId);
    void currentDesktopChanged(int);

protected:
    Task* findTask(WId w);

private:
    Task*       _active;
    QList<Task> _tasks;
};

#endif
