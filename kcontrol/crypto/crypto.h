/**
 * crypto.h
 *
 * Copyright (c) 2000-2001 George Staikos <staikos@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _CRYPTO_H
#define _CRYPTO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kcmodule.h>

#include <ksimpleconfig.h>

#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qvbuttongroup.h>
#include <qlistview.h>
#include <qlineedit.h>
// #include <qvaluelist.h>
#include <qlist.h>
#include <qdatetime.h>

class KURLLabel;
class KCryptoConfig;
class QGridLayout;
class KSSLCertBox;
class QVButtonGroup;

class CipherItem : public QCheckListItem
{
public:
    CipherItem( QListView *view, const QString& cipher, int bits, int maxBits,
		KCryptoConfig *module );
    ~CipherItem() {}

    void setCipher( const QString& cipher ) { m_cipher = cipher; }
    const QString& cipher() const { return m_cipher; }
    void setBits( int bits ) { m_bits = bits; }
    int bits() const { return m_bits; }

    QString configName() const;

protected:
    virtual void stateChange( bool );

private:
    int m_bits;
    QString m_cipher;
    KCryptoConfig *m_module; // just to call configChanged()
};



class OtherCertItem : public QListViewItem
{
public:
    OtherCertItem(QListView *view, QString& sub, bool perm, int policy, QDateTime exp, KCryptoConfig *module );
    ~OtherCertItem() {}

    QString configName() const;
    QString& getSub() { return _sub; }
    int getPolicy() { return _policy; }
    bool isPermanent() { return _perm; }
    QDateTime getExpires() { return _exp; }
    void setPolicy(int x) { _policy = x; } 
    void setPermanent(bool x) { _perm = x; }
    void setExpires(QDateTime x) { _exp = x; }

protected:
    virtual void stateChange( bool );

private:
    QString _sub;
    KCryptoConfig *m_module; // just to call configChanged()
    QDateTime _exp;
    bool _perm;
    int _policy;
};




class YourCertItem : public QListViewItem
{
public:
    YourCertItem(QListView *view, QString pkcs, QString pass, QString name, KCryptoConfig *module );
    ~YourCertItem() {}

    QString configName() const;
    QString& getPKCS() { return _pkcs; }
    void setPKCS(QString pkcs) { _pkcs = pkcs; }
    QString& getPass() { return _pass; }
    void setPass(QString pass) { _pass = pass; }
    QString& getName() { return _name; }
    void setName(QString name) { _name = name; }

protected:
    virtual void stateChange( bool );

private:
    QString _pkcs;
    QString _pass;
    QString _name;
    KCryptoConfig *m_module; // just to call configChanged()
};





class KCryptoConfig : public KCModule
{
  Q_OBJECT
public:
  KCryptoConfig(QWidget *parent = 0L, const char *name = 0L);
  virtual ~KCryptoConfig();

  void load();
  void save();
  void defaults();

  int buttons();
  QString quickHelp() const;

#ifdef HAVE_SSL
  bool loadCiphers();
#endif

public slots:
  void configChanged();

  void slotGeneratePersonal();
  void slotUseEGD();
  void slotChooseEGD();
  void slotChooseOSSL();
  void slotCWcompatible();
  void slotCWus();
  void slotCWexp();
  void slotCWall();

  void slotTestOSSL();

  void slotExportCert();
  void slotRemoveCert();
  void slotVerifyCert();
  void slotOtherCertSelect();
  void slotPolicyChanged(int id);
  void slotPermanent();
  void slotUntil();
  void slotDatePick();

  void slotYourImport();
  void slotYourExport();
  void slotYourVerify();
  void slotYourRemove();
  void slotYourUnlock();
  void slotYourPass();
  void slotYourCertSelect();


private:
  QTabWidget *tabs;
  QWidget *tabSSL, *tabOSSL;

  QWidget *tabYourSSLCert, *tabOtherSSLCert, *tabSSLCA, *tabSSLCOpts;

  QListView *SSLv2Box, *SSLv3Box;
  QCheckBox *mUseTLS, *mUseSSLv2, *mUseSSLv3;
  QCheckBox *mWarnOnEnter, *mWarnOnLeave;

  /* EGD stuff */
  QPushButton *mChooseEGD;
  QLabel    *mEGDLabel;
  QLineEdit *mEGDPath;
  QCheckBox *mUseEGD;

  /* CipherWizards */
  QPushButton *mCWall, *mCWus, *mCWexp, *mCWcompatible;

  QCheckBox *mWarnOnUnencrypted, *mWarnOnMixed;
  QListView *yourSSLBox, *otherSSLBox, *caSSLBox;
  QCheckBox *mWarnSelfSigned, *mWarnExpired, *mWarnRevoked;
  QPushButton *macAdd, *macRemove, *macClear;
  QListBox *macBox;
  QPushButton *otherSSLExport, *otherSSLView, *otherSSLRemove, *otherSSLVerify;
  QPushButton *caSSLImport, *caSSLView, *caSSLRemove, *caSSLVerify;
  QPushButton *yourSSLImport, *yourSSLPass, *yourSSLRemove, *yourSSLExport,
              *yourSSLUnlock, *yourSSLVerify;
  QRadioButton *yourSSLUseDefault, *yourSSLList, *yourSSLDont;
  QLineEdit *macCert;
  KSSLCertBox *oSubject, *oIssuer;
  KSSLCertBox *ySubject, *yIssuer;
  QGridLayout *oGrid;

  QVButtonGroup *policyGroup;
  QRadioButton *policyAccept, *policyReject, *policyPrompt;
  QRadioButton *cacheUntil, *cachePerm;
  QLabel *validFrom, *validUntil;
  QLabel *yValidFrom, *yValidUntil;
  KURLLabel *untilDate;

  /* OpenSSL tab */
  QLabel    *oInfo;
  QLineEdit *oPath;
  QPushButton *oFind;
  QPushButton *oTest;
  QList<OtherCertItem> otherCertDelList;
  QList<YourCertItem> yourCertDelList;


  KConfig *config;
  KSimpleConfig *policies, *pcerts;
};

#endif
