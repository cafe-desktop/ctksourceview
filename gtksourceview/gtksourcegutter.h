/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- *
 *
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2009 - Jesse van den Kieboom
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

#ifndef CTK_SOURCE_GUTTER_H
#define CTK_SOURCE_GUTTER_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_GUTTER			(ctk_source_gutter_get_type ())
#define CTK_SOURCE_GUTTER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_GUTTER, GtkSourceGutter))
#define CTK_SOURCE_GUTTER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_GUTTER, GtkSourceGutterClass))
#define CTK_SOURCE_IS_GUTTER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_GUTTER))
#define CTK_SOURCE_IS_GUTTER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_GUTTER))
#define CTK_SOURCE_GUTTER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_GUTTER, GtkSourceGutterClass))

typedef struct _GtkSourceGutterClass	GtkSourceGutterClass;
typedef struct _GtkSourceGutterPrivate	GtkSourceGutterPrivate;

struct _GtkSourceGutter
{
	GObject parent;

	GtkSourceGutterPrivate *priv;
};

struct _GtkSourceGutterClass
{
	GObjectClass parent_class;

	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType ctk_source_gutter_get_type 		(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_3_24
GtkSourceView *
     ctk_source_gutter_get_view			(GtkSourceGutter         *gutter);

CTK_SOURCE_AVAILABLE_IN_3_24
GtkTextWindowType
     ctk_source_gutter_get_window_type		(GtkSourceGutter         *gutter);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean ctk_source_gutter_insert               (GtkSourceGutter         *gutter,
                                                 GtkSourceGutterRenderer *renderer,
                                                 gint                     position);

CTK_SOURCE_AVAILABLE_IN_ALL
void ctk_source_gutter_reorder			(GtkSourceGutter	 *gutter,
                                                 GtkSourceGutterRenderer *renderer,
                                                 gint                     position);

CTK_SOURCE_AVAILABLE_IN_ALL
void ctk_source_gutter_remove			(GtkSourceGutter         *gutter,
                                                 GtkSourceGutterRenderer *renderer);

CTK_SOURCE_AVAILABLE_IN_ALL
void ctk_source_gutter_queue_draw		(GtkSourceGutter         *gutter);

CTK_SOURCE_AVAILABLE_IN_ALL
GtkSourceGutterRenderer *
     ctk_source_gutter_get_renderer_at_pos      (GtkSourceGutter         *gutter,
                                                 gint                     x,
                                                 gint                     y);

G_END_DECLS

#endif /* CTK_SOURCE_GUTTER_H */
