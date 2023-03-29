/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2010 - Jesse van den Kieboom
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

#ifndef CTK_SOURCE_GUTTER_RENDERER_PIXBUF_H
#define CTK_SOURCE_GUTTER_RENDERER_PIXBUF_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctksourceview/ctksourcetypes.h>
#include <ctksourceview/ctksourcegutterrenderer.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_GUTTER_RENDERER_PIXBUF			(ctk_source_gutter_renderer_pixbuf_get_type ())
#define CTK_SOURCE_GUTTER_RENDERER_PIXBUF(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER_PIXBUF, CtkSourceGutterRendererPixbuf))
#define CTK_SOURCE_GUTTER_RENDERER_PIXBUF_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER_PIXBUF, CtkSourceGutterRendererPixbuf const))
#define CTK_SOURCE_GUTTER_RENDERER_PIXBUF_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_GUTTER_RENDERER_PIXBUF, CtkSourceGutterRendererPixbufClass))
#define CTK_SOURCE_IS_GUTTER_RENDERER_PIXBUF(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER_PIXBUF))
#define CTK_SOURCE_IS_GUTTER_RENDERER_PIXBUF_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_GUTTER_RENDERER_PIXBUF))
#define CTK_SOURCE_GUTTER_RENDERER_PIXBUF_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER_PIXBUF, CtkSourceGutterRendererPixbufClass))

typedef struct _CtkSourceGutterRendererPixbufClass	CtkSourceGutterRendererPixbufClass;
typedef struct _CtkSourceGutterRendererPixbufPrivate	CtkSourceGutterRendererPixbufPrivate;

struct _CtkSourceGutterRendererPixbuf
{
	/*< private >*/
	CtkSourceGutterRenderer parent;

	CtkSourceGutterRendererPixbufPrivate *priv;

	/*< public >*/
};

struct _CtkSourceGutterRendererPixbufClass
{
	/*< private >*/
	CtkSourceGutterRendererClass parent_class;

	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType ctk_source_gutter_renderer_pixbuf_get_type (void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceGutterRenderer *ctk_source_gutter_renderer_pixbuf_new (void);

CTK_SOURCE_AVAILABLE_IN_ALL
void         ctk_source_gutter_renderer_pixbuf_set_pixbuf       (CtkSourceGutterRendererPixbuf *renderer,
                                                                 GdkPixbuf                     *pixbuf);

CTK_SOURCE_AVAILABLE_IN_ALL
GdkPixbuf   *ctk_source_gutter_renderer_pixbuf_get_pixbuf       (CtkSourceGutterRendererPixbuf *renderer);

CTK_SOURCE_AVAILABLE_IN_ALL
void         ctk_source_gutter_renderer_pixbuf_set_gicon        (CtkSourceGutterRendererPixbuf *renderer,
                                                                 GIcon                         *icon);

CTK_SOURCE_AVAILABLE_IN_ALL
GIcon       *ctk_source_gutter_renderer_pixbuf_get_gicon        (CtkSourceGutterRendererPixbuf *renderer);

CTK_SOURCE_AVAILABLE_IN_ALL
void         ctk_source_gutter_renderer_pixbuf_set_icon_name    (CtkSourceGutterRendererPixbuf *renderer,
                                                                 const gchar                   *icon_name);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar *ctk_source_gutter_renderer_pixbuf_get_icon_name    (CtkSourceGutterRendererPixbuf *renderer);

G_END_DECLS

#endif /* CTK_SOURCE_GUTTER_RENDERER_TEXT_H */
