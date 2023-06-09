/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- *
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2016, 2017 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * CtkSourceView is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * CtkSourceView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourceview-gresources.h"

#include "ctksourceinit.h"
#include <glib/gi18n-lib.h>
#include "ctksourcelanguagemanager.h"
#include "ctksourcestyleschememanager.h"

#ifdef G_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static HMODULE ctk_source_dll;

BOOL WINAPI DllMain (HINSTANCE hinstDLL,
		     DWORD     fdwReason,
		     LPVOID    lpvReserved);

BOOL WINAPI
DllMain (HINSTANCE hinstDLL,
	 DWORD     fdwReason,
	 LPVOID    lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			ctk_source_dll = hinstDLL;
			break;

		default:
			/* do nothing */
			break;
	}

	return TRUE;
}
#endif /* G_OS_WIN32 */

#ifdef OS_OSX
#include <Cocoa/Cocoa.h>

static gchar *
dirs_os_x_get_bundle_resource_dir (void)
{
	NSAutoreleasePool *pool;
	gchar *str = NULL;
	NSString *path;

	pool = [[NSAutoreleasePool alloc] init];

	if ([[NSBundle mainBundle] bundleIdentifier] == nil)
	{
		[pool release];
		return NULL;
	}

	path = [[NSBundle mainBundle] resourcePath];

	if (!path)
	{
		[pool release];
		return NULL;
	}

	str = g_strdup ([path UTF8String]);
	[pool release];
	return str;
}

static gchar *
dirs_os_x_get_locale_dir (void)
{
	gchar *res_dir;
	gchar *ret;

	res_dir = dirs_os_x_get_bundle_resource_dir ();

	if (res_dir == NULL)
	{
		ret = g_build_filename (DATADIR, "locale", NULL);
	}
	else
	{
		ret = g_build_filename (res_dir, "share", "locale", NULL);
		g_free (res_dir);
	}

	return ret;
}
#endif /* OS_OSX */

static gchar *
get_locale_dir (void)
{
	gchar *locale_dir;

#if defined (G_OS_WIN32)
	gchar *win32_dir;

	win32_dir = g_win32_get_package_installation_directory_of_module (ctk_source_dll);

	locale_dir = g_build_filename (win32_dir, "share", "locale", NULL);

	g_free (win32_dir);
#elif defined (OS_OSX)
	locale_dir = dirs_os_x_get_locale_dir ();
#else
	locale_dir = g_build_filename (DATADIR, "locale", NULL);
#endif

	return locale_dir;
}

/**
 * ctk_source_init:
 *
 * Initializes the CtkSourceView library (e.g. for the internationalization).
 *
 * This function can be called several times, but is meant to be called at the
 * beginning of main(), before any other CtkSourceView function call.
 *
 * Since: 4.0
 */
void
ctk_source_init (void)
{
	static gboolean done = FALSE;

	if (!done)
	{
		gchar *locale_dir;

		locale_dir = get_locale_dir ();
		bindtextdomain (GETTEXT_PACKAGE, locale_dir);
		g_free (locale_dir);

		bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

		done = TRUE;
	}
}

/**
 * ctk_source_finalize:
 *
 * Free the resources allocated by CtkSourceView. For example it unrefs the
 * singleton objects.
 *
 * It is not mandatory to call this function, it's just to be friendlier to
 * memory debugging tools. This function is meant to be called at the end of
 * main(). It can be called several times.
 *
 * Since: 4.0
 */

/* Another way is to use a DSO destructor, see gconstructor.h in GLib.
 *
 * The advantage of calling ctk_source_finalize() at the end of main() is that
 * gobject-list [1] correctly reports that all CtkSource* objects have been
 * finalized when quitting the application. On the other hand a DSO destructor
 * runs after the gobject-list's last output, so it's much less convenient, see:
 * commit e761de9c2bee90c232875bbc41e6e73e1f63e145
 *
 * [1] A tool for debugging the lifetime of GObjects:
 * https://github.com/danni/gobject-list
 */
void
ctk_source_finalize (void)
{
	static gboolean done = FALSE;

	/* Unref the singletons only once, even if this function is called
	 * multiple times, to see if a reference is not released correctly.
	 * Normally the singleton have a ref count of 1. If for some reason the
	 * ref count is increased somewhere, it needs to be decreased
	 * accordingly, at the right place.
	 */
	if (!done)
	{
		CtkSourceLanguageManager *language_manager;
		CtkSourceStyleSchemeManager *style_scheme_manager;

		g_resources_register (ctksourceview_get_resource ());

		language_manager = _ctk_source_language_manager_peek_default ();
		g_clear_object (&language_manager);

		style_scheme_manager = _ctk_source_style_scheme_manager_peek_default ();
		g_clear_object (&style_scheme_manager);

		done = TRUE;
	}
}
