/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef CTK_SOURCE_SPACE_DRAWER_H
#define CTK_SOURCE_SPACE_DRAWER_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_SPACE_DRAWER             (ctk_source_space_drawer_get_type ())
#define CTK_SOURCE_SPACE_DRAWER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_SPACE_DRAWER, GtkSourceSpaceDrawer))
#define CTK_SOURCE_SPACE_DRAWER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_SPACE_DRAWER, GtkSourceSpaceDrawerClass))
#define CTK_SOURCE_IS_SPACE_DRAWER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_SPACE_DRAWER))
#define CTK_SOURCE_IS_SPACE_DRAWER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_SPACE_DRAWER))
#define CTK_SOURCE_SPACE_DRAWER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_SPACE_DRAWER, GtkSourceSpaceDrawerClass))

typedef struct _GtkSourceSpaceDrawerClass    GtkSourceSpaceDrawerClass;
typedef struct _GtkSourceSpaceDrawerPrivate  GtkSourceSpaceDrawerPrivate;

struct _GtkSourceSpaceDrawer
{
	GObject parent;

	GtkSourceSpaceDrawerPrivate *priv;
};

struct _GtkSourceSpaceDrawerClass
{
	GObjectClass parent_class;

	gpointer padding[20];
};

/**
 * GtkSourceSpaceTypeFlags:
 * @CTK_SOURCE_SPACE_TYPE_NONE: No flags.
 * @CTK_SOURCE_SPACE_TYPE_SPACE: Space character.
 * @CTK_SOURCE_SPACE_TYPE_TAB: Tab character.
 * @CTK_SOURCE_SPACE_TYPE_NEWLINE: Line break character. If the
 *   #GtkSourceBuffer:implicit-trailing-newline property is %TRUE,
 *   #GtkSourceSpaceDrawer also draws a line break at the end of the buffer.
 * @CTK_SOURCE_SPACE_TYPE_NBSP: Non-breaking space character.
 * @CTK_SOURCE_SPACE_TYPE_ALL: All white spaces.
 *
 * #GtkSourceSpaceTypeFlags contains flags for white space types.
 *
 * Since: 3.24
 */
typedef enum _GtkSourceSpaceTypeFlags
{
	CTK_SOURCE_SPACE_TYPE_NONE	= 0,
	CTK_SOURCE_SPACE_TYPE_SPACE	= 1 << 0,
	CTK_SOURCE_SPACE_TYPE_TAB	= 1 << 1,
	CTK_SOURCE_SPACE_TYPE_NEWLINE	= 1 << 2,
	CTK_SOURCE_SPACE_TYPE_NBSP	= 1 << 3,
	CTK_SOURCE_SPACE_TYPE_ALL	= 0xf
} GtkSourceSpaceTypeFlags;

/**
 * GtkSourceSpaceLocationFlags:
 * @CTK_SOURCE_SPACE_LOCATION_NONE: No flags.
 * @CTK_SOURCE_SPACE_LOCATION_LEADING: Leading white spaces on a line, i.e. the
 *   indentation.
 * @CTK_SOURCE_SPACE_LOCATION_INSIDE_TEXT: White spaces inside a line of text.
 * @CTK_SOURCE_SPACE_LOCATION_TRAILING: Trailing white spaces on a line.
 * @CTK_SOURCE_SPACE_LOCATION_ALL: White spaces anywhere.
 *
 * #GtkSourceSpaceLocationFlags contains flags for white space locations.
 *
 * If a line contains only white spaces (no text), the white spaces match both
 * %CTK_SOURCE_SPACE_LOCATION_LEADING and %CTK_SOURCE_SPACE_LOCATION_TRAILING.
 *
 * Since: 3.24
 */
typedef enum _GtkSourceSpaceLocationFlags
{
	CTK_SOURCE_SPACE_LOCATION_NONE		= 0,
	CTK_SOURCE_SPACE_LOCATION_LEADING	= 1 << 0,
	CTK_SOURCE_SPACE_LOCATION_INSIDE_TEXT	= 1 << 1,
	CTK_SOURCE_SPACE_LOCATION_TRAILING	= 1 << 2,
	CTK_SOURCE_SPACE_LOCATION_ALL		= 0x7
} GtkSourceSpaceLocationFlags;

CTK_SOURCE_AVAILABLE_IN_3_24
GType			ctk_source_space_drawer_get_type		(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_3_24
GtkSourceSpaceDrawer *	ctk_source_space_drawer_new			(void);

CTK_SOURCE_AVAILABLE_IN_3_24
GtkSourceSpaceTypeFlags	ctk_source_space_drawer_get_types_for_locations	(GtkSourceSpaceDrawer        *drawer,
									 GtkSourceSpaceLocationFlags  locations);

CTK_SOURCE_AVAILABLE_IN_3_24
void			ctk_source_space_drawer_set_types_for_locations	(GtkSourceSpaceDrawer        *drawer,
									 GtkSourceSpaceLocationFlags  locations,
									 GtkSourceSpaceTypeFlags      types);

CTK_SOURCE_AVAILABLE_IN_3_24
GVariant *		ctk_source_space_drawer_get_matrix		(GtkSourceSpaceDrawer *drawer);

CTK_SOURCE_AVAILABLE_IN_3_24
void			ctk_source_space_drawer_set_matrix		(GtkSourceSpaceDrawer *drawer,
									 GVariant             *matrix);

CTK_SOURCE_AVAILABLE_IN_3_24
gboolean		ctk_source_space_drawer_get_enable_matrix	(GtkSourceSpaceDrawer *drawer);

CTK_SOURCE_AVAILABLE_IN_3_24
void			ctk_source_space_drawer_set_enable_matrix	(GtkSourceSpaceDrawer *drawer,
									 gboolean              enable_matrix);

CTK_SOURCE_AVAILABLE_IN_3_24
void			ctk_source_space_drawer_bind_matrix_setting	(GtkSourceSpaceDrawer *drawer,
									 GSettings            *settings,
									 const gchar          *key,
									 GSettingsBindFlags    flags);

G_END_DECLS

#endif /* CTK_SOURCE_SPACE_DRAWER_H */
