/*

    $Id$

    Copyright (C) 2000 Charles Samuels <charles@altair.dhs.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/
#include "eventview.h"
#include <qlistview.h>
#include <eventconfig.h>
#include <kstddirs.h>
#include "eventconfig.moc"

EventView *Programs::eventview=0;
QListView *Programs::programs=0;
QListView *Programs::events=0;

void EventConfig::load(KConfig &conf)
{
	friendly=conf.readEntry("friendly", "Unknown Name");
	description=conf.readEntry("description", "No Description");
	
	{ // Load the presentation
		present=conf.readNumEntry("presentation", -1);
		if (present==-1)
			present=conf.readNumEntry("default_presentation", 0);
	}

	{ // Load the files
		soundfile=conf.readEntry("soundfile");
		if (soundfile.isNull())
			soundfile=conf.readEntry("default_soundfile");
	}
	
	{ // Load the files
		logfile=conf.readEntry("logfile");
		if (logfile.isNull())
			logfile=conf.readEntry("default_logfile");
	}
}

ProgramConfig::~ProgramConfig()
{
	eventlist.setAutoDelete(true);
}

void ProgramConfig::load(KConfig &conf)
{
	// Load the Names
	appname=conf.readEntry("appname", "Unknown Title");
	description=conf.readEntry("description", "No Description");

	// Load all the events	
	QStringList conflist=conf.groupList();
	conflist.remove(QString("!Global!"));
	conflist.remove(QString("<default>"));
	
	for (QStringList::Iterator it=conflist.begin(); it!=conflist.end(); ++it)
	{
		conf.setGroup(*it);
		EventConfig *e=new EventConfig(this);
		e->load(conf);
		eventlist.append(e);
		kapp->processEvents();
	}

}

Programs::Programs(EventView *_eventview, QListView *_programs,
	              QListView *_events)
{
	if (_eventview)
		eventview=_eventview;
	if (_programs)
		programs=_programs;
	if (_events)
		events=_events;
	
	QStringList dirs(locate("config", "eventsrc")); // load system-wide eventsrc
	dirs+=KGlobal::dirs()->findAllResources("data", "*/eventsrc");
	
	for (QStringList::Iterator it=dirs.begin(); it!=dirs.end(); ++it)
	{
		KConfig conf(*it);
		conf.setGroup("!Global!");
		ProgramConfig *prog=new ProgramConfig;
		programlist.append(prog);
		prog->configfile=*it;
		prog->load(conf);
	}
}

Programs::~Programs()
{
	programlist.setAutoDelete(true);
}

void Programs::show()
{
	// Unload what we have now
	
	// Load the new goods.

	// Put them in the app list
	for (ProgramConfig *prog=programlist.first(); prog != 0; prog=programlist.next())
		new ProgramConfig::ProgramListViewItem(prog);
	
	Programs::programs->setSelected(Programs::programs->firstChild(),true);
}

void ProgramConfig::show()
{
	// Unload the old events
	
	// and show the new ones
	for (EventConfig *ev=eventlist.first(); ev != 0; ev=eventlist.next())
		new EventConfig::EventListViewItem(ev);
	
	Programs::events->setSelected(Programs::events->firstChild(),true);
}

ProgramConfig::ProgramListViewItem::ProgramListViewItem(const ProgramConfig *prog)
	: QListViewItem(Programs::programs, prog->appname, prog->description),
	  program(prog)
{}

EventConfig::EventListViewItem::EventListViewItem(const EventConfig *ev)
	: QListViewItem(Programs::events, ev->friendly, ev->description),
	  event(ev)
{}
