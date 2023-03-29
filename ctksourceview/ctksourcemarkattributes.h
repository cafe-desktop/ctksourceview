/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2010 - Jesse van den Kieboom
 * Copyright (C) 2010 - Krzesimir Nowak
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

#ifndef CTK_SOURCE_MARK_ATTRIBUTES_H
#define CTK_SOURCE_MARK_ATTRIBUTES_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_MARK_ATTRIBUTES			(ctk_source_mark_attributes_get_type ())
#define CTK_SOURCE_MARK_ATTRIBUTES(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_MARK_ATTRIBUTES, CtkSourceMarkAttributes))
#define CTK_SOURCE_MARK_ATTRIBUTES_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_MARK_ATTRIBUTES, CtkSourceMarkAttributesClass))
#define CTK_SOURCE_IS_MARK_ATTRIBUTES(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_MARK_ATTRIBUTES))
#define CTK_SOURCE_IS_MARK_ATTRIBUTES_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_MARK_ATTRIBUTES))
#define CTK_SOURCE_MARK_ATTRIBUTES_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_MARK_ATTRIBUTES, CtkSourceMarkAttributesClass))

typedef struct _CtkSourceMarkAttributesClass	CtkSourceMarkAttributesClass;
typedef struct _CtkSourceMarkAttributesPrivate	CtkSourceMarkAttributesPrivate;

struct _CtkSourceMarkAttributes
{
	/*< private >*/
	GObject parent;

	CtkSourceMarkAttributesPrivate *priv;

	/*< public >*/
};

struct _CtkSourceMarkAttributesClass
{
	/*< private >*/
	GObjectClass parent_class;

	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType ctk_source_mark_attributes_get_type (void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceMarkAttributes *ctk_source_mark_attributes_new (void);

CTK_SOURCE_AVAILABLE_IN_ALL
void             ctk_source_mark_attributes_set_background      (CtkSourceMarkAttributes *attributes,
                                                                 const CdkRGBA           *background);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean         ctk_source_mark_attributes_get_background      (CtkSourceMarkAttributes *attributes,
                                                                 CdkRGBA                 *background);

CTK_SOURCE_AVAILABLE_IN_ALL
void             ctk_source_mark_attributes_set_icon_name       (CtkSourceMarkAttributes *attributes,
                                                                 const gchar             *icon_name);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar     *ctk_source_mark_attributes_get_icon_name       (CtkSourceMarkAttributes *attributes);

CTK_SOURCE_AVAILABLE_IN_ALL
void             ctk_source_mark_attributes_set_gicon           (CtkSourceMarkAttributes *attributes,
                                                                 GIcon                   *gicon);

CTK_SOURCE_AVAILABLE_IN_ALL
GIcon           *ctk_source_mark_attributes_get_gicon           (CtkSourceMarkAttributes *attributes);

CTK_SOURCE_AVAILABLE_IN_ALL
void             ctk_source_mark_attributes_set_pixbuf          (CtkSourceMarkAttributes *attributes,
                                                                 const GdkPixbuf         *pixbuf);

CTK_SOURCE_AVAILABLE_IN_ALL
const GdkPixbuf *ctk_source_mark_attributes_get_pixbuf          (CtkSourceMarkAttributes *attributes);

CTK_SOURCE_AVAILABLE_IN_ALL
const GdkPixbuf *ctk_source_mark_attributes_render_icon         (CtkSourceMarkAttributes *attributes,
                                                                 CtkWidget               *widget,
                                                                 gint                   size);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar           *ctk_source_mark_attributes_get_tooltip_text    (CtkSourceMarkAttributes *attributes,
                                                                 CtkSourceMark           *mark);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar           *ctk_source_mark_attributes_get_tooltip_markup  (CtkSourceMarkAttributes *attributes,
                                                                 CtkSourceMark           *mark);

G_END_DECLS

#endif /* CTK_SOURCE_MARK_ATTRIBUTES_H */
