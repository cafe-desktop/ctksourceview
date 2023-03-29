/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2010 - Jesse van den Kieboom
 * Copyright (C) 2010 - Krzesimir Nowak
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

#ifndef CTK_SOURCE_MARK_ATTRIBUTES_H
#define CTK_SOURCE_MARK_ATTRIBUTES_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_MARK_ATTRIBUTES			(ctk_source_mark_attributes_get_type ())
#define CTK_SOURCE_MARK_ATTRIBUTES(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_MARK_ATTRIBUTES, GtkSourceMarkAttributes))
#define CTK_SOURCE_MARK_ATTRIBUTES_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_MARK_ATTRIBUTES, GtkSourceMarkAttributesClass))
#define CTK_SOURCE_IS_MARK_ATTRIBUTES(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_MARK_ATTRIBUTES))
#define CTK_SOURCE_IS_MARK_ATTRIBUTES_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_MARK_ATTRIBUTES))
#define CTK_SOURCE_MARK_ATTRIBUTES_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_MARK_ATTRIBUTES, GtkSourceMarkAttributesClass))

typedef struct _GtkSourceMarkAttributesClass	GtkSourceMarkAttributesClass;
typedef struct _GtkSourceMarkAttributesPrivate	GtkSourceMarkAttributesPrivate;

struct _GtkSourceMarkAttributes
{
	/*< private >*/
	GObject parent;

	GtkSourceMarkAttributesPrivate *priv;

	/*< public >*/
};

struct _GtkSourceMarkAttributesClass
{
	/*< private >*/
	GObjectClass parent_class;

	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType ctk_source_mark_attributes_get_type (void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
GtkSourceMarkAttributes *ctk_source_mark_attributes_new (void);

CTK_SOURCE_AVAILABLE_IN_ALL
void             ctk_source_mark_attributes_set_background      (GtkSourceMarkAttributes *attributes,
                                                                 const GdkRGBA           *background);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean         ctk_source_mark_attributes_get_background      (GtkSourceMarkAttributes *attributes,
                                                                 GdkRGBA                 *background);

CTK_SOURCE_AVAILABLE_IN_ALL
void             ctk_source_mark_attributes_set_icon_name       (GtkSourceMarkAttributes *attributes,
                                                                 const gchar             *icon_name);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar     *ctk_source_mark_attributes_get_icon_name       (GtkSourceMarkAttributes *attributes);

CTK_SOURCE_AVAILABLE_IN_ALL
void             ctk_source_mark_attributes_set_gicon           (GtkSourceMarkAttributes *attributes,
                                                                 GIcon                   *gicon);

CTK_SOURCE_AVAILABLE_IN_ALL
GIcon           *ctk_source_mark_attributes_get_gicon           (GtkSourceMarkAttributes *attributes);

CTK_SOURCE_AVAILABLE_IN_ALL
void             ctk_source_mark_attributes_set_pixbuf          (GtkSourceMarkAttributes *attributes,
                                                                 const GdkPixbuf         *pixbuf);

CTK_SOURCE_AVAILABLE_IN_ALL
const GdkPixbuf *ctk_source_mark_attributes_get_pixbuf          (GtkSourceMarkAttributes *attributes);

CTK_SOURCE_AVAILABLE_IN_ALL
const GdkPixbuf *ctk_source_mark_attributes_render_icon         (GtkSourceMarkAttributes *attributes,
                                                                 GtkWidget               *widget,
                                                                 gint                   size);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar           *ctk_source_mark_attributes_get_tooltip_text    (GtkSourceMarkAttributes *attributes,
                                                                 GtkSourceMark           *mark);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar           *ctk_source_mark_attributes_get_tooltip_markup  (GtkSourceMarkAttributes *attributes,
                                                                 GtkSourceMark           *mark);

G_END_DECLS

#endif /* CTK_SOURCE_MARK_ATTRIBUTES_H */
