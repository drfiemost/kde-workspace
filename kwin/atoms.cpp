/*****************************************************************
kwin - the KDE window manager

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/
#include <qapplication.h>
#include "atoms.h"

Atoms::Atoms()
{

    const int max = 20;
    Atom* atoms[max];
    char* names[max];
    Atom atoms_return[max];
    int n = 0;

    atoms[n] = &wm_protocols;
    names[n++] = (char *) "WM_PROTOCOLS";

    atoms[n] = &wm_delete_window;
    names[n++] = (char *) "WM_DELETE_WINDOW";

    atoms[n] = &wm_take_focus;
    names[n++] = (char *) "WM_TAKE_FOCUS";

    atoms[n] = &wm_change_state;
    names[n++] = (char *) "WM_CHANGE_STATE";

     // compatibility
    atoms[n] = &kwm_win_icon;
    names[n++] = (char *) "KWM_WIN_ICON";

     // compatibility
    atoms[n] = &kwm_running;
    names[n++] = (char *) "KWM_RUNNING";

    atoms[n] = &kwm_command;
    names[n++] = (char *) "KWM_COMMAND";
    
    atoms[n] = &motif_wm_hints;
    names[n++] = (char *) "_MOTIF_WM_HINTS";

    atoms[n] = &net_number_of_desktops;
    names[n++] = (char *) "_NET_NUMBER_OF_DESKTOPS";

    atoms[n] = &net_current_desktop;
    names[n++] = (char *) "_NET_CURRENT_DESKTOP";

    atoms[n] = &net_active_window;
    names[n++] = (char *) "_NET_ACTIVE_WINDOW";

    atoms[n] = &net_wm_context_help;
    names[n++] = (char *) "_NET_WM_CONTEXT_HELP";

    atoms[n] = &net_client_list;
    names[n++] = (char *) "_NET_CLIENT_LIST";

    atoms[n] = &net_client_list_stacking;
    names[n++] = (char *) "_NET_CLIENT_LIST_STACKING";

    atoms[n] = &net_kde_docking_windows;
    names[n++] = (char *) "_NET_KDE_DOCKING_WINDOWS";

    XInternAtoms( qt_xdisplay(), names, n, FALSE, atoms_return );
    for (int i = 0; i < n; i++ )
	*atoms[i] = atoms_return[i];


}
