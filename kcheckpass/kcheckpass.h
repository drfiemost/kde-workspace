/*****************************************************************
 *
 *	kcheckpass
 *
 *	Simple password checker. Just invoke and send it
 *	the password on stdin.
 *
 *	If the password was accepted, the program exits with 0;
 *	if it was rejected, it exits with 1. Any other exit
 *	code signals an error.
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *	It's hopefully simple enough to allow it to be setuid
 *	root.
 *
 *	Compile with -DHAVE_VSYSLOG if you have vsyslog().
 *	Compile with -DHAVE_PAM if you have a PAM system,
 *	and link with -lpam -ldl.
 *	Compile with -DHAVE_SHADOW if you have a shadow
 *	password system.
 *
 *	Copyright (C) 1998, Caldera, Inc.
 *	Released under the GNU General Public License
 *
 *	Olaf Kirch <okir@caldera.de>      General Framework and PAM support
 *	Christian Esken <esken@kde.org>   Shadow and /etc/passwd support
 *
 *      Other parts were taken from kscreensaver's passwd.cpp
 *****************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif


#ifdef ultrix
#include <auth.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef OSF1_ENH_SEC
#include <sys/security.h>
#include <prot.h>
#endif

// Default back to ETCPASSWD
#if !defined(PAM) && !defined(SHADOW)
#define ETCPASSWD
#endif


/*****************************************************************
 * This function authenticates the user, returning
 *   1		if the password was accepted
 *   0		otherwise
 *****************************************************************/
int authenticate(const char *login, const char *pass);
void message(const char*, ...);
