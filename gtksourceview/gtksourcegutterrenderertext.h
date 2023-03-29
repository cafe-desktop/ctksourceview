/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2010 - Jesse van den Kieboom
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

#ifndef CTK_SOURCE_GUTTER_RENDERER_TEXT_H
#define CTK_SOURCE_GUTTER_RENDERER_TEXT_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctksourceview/ctksourcetypes.h>
#include <ctksourceview/ctksourcegutterrenderer.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_GUTTER_RENDERER_TEXT		(ctk_source_gutter_renderer_text_get_type ())
#define CTK_SOURCE_GUTTER_RENDERER_TEXT(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER_TEXT, GtkSourceGutterRendererText))
#define CTK_SOURCE_GUTTER_RENDERER_TEXT_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER_TEXT, GtkSourceGutterRendererText const))
#define CTK_SOURCE_GUTTER_RENDERER_TEXT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_GUTTER_RENDERER_TEXT, GtkSourceGutterRendererTextClass))
#define CTK_SOURCE_IS_GUTTER_RENDERER_TEXT(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER_TEXT))
#define CTK_SOURCE_IS_GUTTER_RENDERER_TEXT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_GUTTER_RENDERER_TEXT))
#define CTK_SOURCE_GUTTER_RENDERER_TEXT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER_TEXT, GtkSourceGutterRendererTextClass))

typedef struct _GtkSourceGutterRendererTextClass	GtkSourceGutterRendererTextClass;
typedef struct _GtkSourceGutterRendererTextPrivate	GtkSourceGutterRendererTextPrivate;

struct _GtkSourceGutterRendererText
{
	/*< private >*/
	GtkSourceGutterRenderer parent;

	GtkSourceGutterRendererTextPrivate *priv;

	/*< public >*/
};

struct _GtkSourceGutterRendererTextClass
{
	/*< private >*/
	GtkSourceGutterRendererClass parent_class;

	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType ctk_source_gutter_renderer_text_get_type (void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
GtkSourceGutterRenderer *ctk_source_gutter_renderer_text_new (void);

CTK_SOURCE_AVAILABLE_IN_ALL
void ctk_source_gutter_renderer_text_set_markup (GtkSourceGutterRendererText *renderer,
                                                 const gchar                 *markup,
                                                 gint                         length);

CTK_SOURCE_AVAILABLE_IN_ALL
void ctk_source_gutter_renderer_text_set_text (GtkSourceGutterRendererText *renderer,
                                               const gchar                 *text,
                                               gint                         length);

CTK_SOURCE_AVAILABLE_IN_ALL
void ctk_source_gutter_renderer_text_measure (GtkSourceGutterRendererText *renderer,
                                              const gchar                 *text,
                                              gint                        *width,
                                              gint                        *height);

CTK_SOURCE_AVAILABLE_IN_ALL
void ctk_source_gutter_renderer_text_measure_markup (GtkSourceGutterRendererText *renderer,
                                                     const gchar                 *markup,
                                                     gint                        *width,
                                                     gint                        *height);

G_END_DECLS

#endif /* CTK_SOURCE_GUTTER_RENDERER_TEXT_H */
