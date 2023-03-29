/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2013 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * GtkSourceView is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * GtkSourceView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CTK_SOURCE_SEARCH_SETTINGS_H
#define CTK_SOURCE_SEARCH_SETTINGS_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <glib-object.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_SEARCH_SETTINGS             (ctk_source_search_settings_get_type ())
#define CTK_SOURCE_SEARCH_SETTINGS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_SEARCH_SETTINGS, GtkSourceSearchSettings))
#define CTK_SOURCE_SEARCH_SETTINGS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_SEARCH_SETTINGS, GtkSourceSearchSettingsClass))
#define CTK_SOURCE_IS_SEARCH_SETTINGS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_SEARCH_SETTINGS))
#define CTK_SOURCE_IS_SEARCH_SETTINGS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_SEARCH_SETTINGS))
#define CTK_SOURCE_SEARCH_SETTINGS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_SEARCH_SETTINGS, GtkSourceSearchSettingsClass))

typedef struct _GtkSourceSearchSettingsClass    GtkSourceSearchSettingsClass;
typedef struct _GtkSourceSearchSettingsPrivate  GtkSourceSearchSettingsPrivate;

struct _GtkSourceSearchSettings
{
	GObject parent;

	GtkSourceSearchSettingsPrivate *priv;
};

struct _GtkSourceSearchSettingsClass
{
	GObjectClass parent_class;

	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType			 ctk_source_search_settings_get_type			(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
GtkSourceSearchSettings *ctk_source_search_settings_new				(void);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_search_settings_set_search_text		(GtkSourceSearchSettings *settings,
										 const gchar		 *search_text);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar		*ctk_source_search_settings_get_search_text		(GtkSourceSearchSettings *settings);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_search_settings_set_case_sensitive		(GtkSourceSearchSettings *settings,
										 gboolean		  case_sensitive);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		 ctk_source_search_settings_get_case_sensitive		(GtkSourceSearchSettings *settings);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_search_settings_set_at_word_boundaries	(GtkSourceSearchSettings *settings,
										 gboolean		  at_word_boundaries);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		 ctk_source_search_settings_get_at_word_boundaries	(GtkSourceSearchSettings *settings);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_search_settings_set_wrap_around		(GtkSourceSearchSettings *settings,
										 gboolean		  wrap_around);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		 ctk_source_search_settings_get_wrap_around		(GtkSourceSearchSettings *settings);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_search_settings_set_regex_enabled		(GtkSourceSearchSettings *settings,
										 gboolean		  regex_enabled);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		 ctk_source_search_settings_get_regex_enabled		(GtkSourceSearchSettings *settings);

G_END_DECLS

#endif /* CTK_SOURCE_SEARCH_SETTINGS_H */
