/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 * 
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef __BGDialogs_h_Included__
#define __BGDialogs_h_Included__

#include <qmap.h>
#include <qstring.h>
#include <qevent.h>

#include <kdialogbase.h>

class QListView;
class QListViewItem;
class QLineEdit;
class QSpinBox;
class QGroupBox;
class QListBox;
class KBackgroundProgram;
class KBackgroundSettings;


/**
 * Dialog to select a background program.
 */
class KProgramSelectDialog: public KDialogBase
{
    Q_OBJECT

public:
    KProgramSelectDialog(QWidget *parent=0L, char *name=0L);

    void setCurrent(QString program);
    QString program() { return m_Current; }

public slots:
    void slotAdd();
    void slotRemove();
    void slotModify();
    void slotItemClicked(QListViewItem *);
    void slotItemDoubleClicked(QListViewItem *);

private:
    void updateItem(QString name, bool select=false);

    QMap<QString,QListViewItem *> m_Items;
    QListView *m_ListView;
    QString m_Current;
};


/**
 * Dialog to edit a background program.
 */
class KProgramEditDialog: public KDialogBase
{
    Q_OBJECT

public:
    KProgramEditDialog(QString program=QString::null, QWidget *parent=0L, 
	    char *name=0L);

    /** The program name is here in case the user changed it */
    QString program();

public slots:
    void slotOk();

private:
    QString m_Program;
    QLineEdit *m_NameEdit, *m_CommentEdit;
    QLineEdit *m_ExecEdit, *m_CommandEdit;
    QLineEdit *m_PreviewEdit; 
    QSpinBox *m_RefreshEdit;
    KBackgroundProgram *m_Prog;
};


/**
 * Dialog to select a background pattern.
 */
class KPatternSelectDialog: public KDialogBase
{
    Q_OBJECT

public:
    KPatternSelectDialog(QWidget *parent=0L, char *name=0L);

    void setCurrent(QString program);
    QString pattern() { return m_Current; }

public slots:
    void slotAdd();
    void slotRemove();
    void slotModify();
    void slotItemClicked(QListViewItem *);
    void slotItemDoubleClicked(QListViewItem *);

private:
    void updateItem(QString name, bool select=false);

    QMap<QString,QListViewItem *> m_Items;
    QListView *m_ListView;
    QString m_Current;
};


/**
 * Dialog to edit a background pattern.
 */
class KPatternEditDialog: public KDialogBase
{
    Q_OBJECT

public:
    KPatternEditDialog(QString pattern=QString::null, QWidget *parent=0L, 
	    char *name=0L);

    /** The program name is here in case the user changed it */
    QString pattern();

public slots:
    void slotOk();
    void slotBrowse();

private:
    QString m_Pattern;
    QLineEdit *m_NameEdit, *m_FileEdit;
    QLineEdit *m_CommentEdit;
};
    

/**
 * QListBox with DND
 */

class KMultiWallpaperList: public QListBox
{
public:
    KMultiWallpaperList(QWidget *parent=0L, char *name=0L);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dropEvent(QDropEvent *);
};



#endif // __BGDialogs_h_Included__

