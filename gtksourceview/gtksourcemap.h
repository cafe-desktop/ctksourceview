/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
 * Copyright (C) 2015 Ignacio Casal Quinteiro <icq@gnome.org>
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

#ifndef CTK_SOURCE_MAP_H
#define CTK_SOURCE_MAP_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>
#include <ctksourceview/ctksourceview.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_MAP            (ctk_source_map_get_type())
#define CTK_SOURCE_MAP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_MAP, GtkSourceMap))
#define CTK_SOURCE_MAP_CONST(obj)      (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_MAP, GtkSourceMap const))
#define CTK_SOURCE_MAP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  CTK_SOURCE_TYPE_MAP, GtkSourceMapClass))
#define CTK_SOURCE_IS_MAP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_MAP))
#define CTK_SOURCE_IS_MAP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  CTK_SOURCE_TYPE_MAP))
#define CTK_SOURCE_MAP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  CTK_SOURCE_TYPE_MAP, GtkSourceMapClass))

typedef struct _GtkSourceMapClass GtkSourceMapClass;

struct _GtkSourceMap
{
	GtkSourceView parent_instance;
};

struct _GtkSourceMapClass
{
	GtkSourceViewClass parent_class;

	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_3_18
GType			 ctk_source_map_get_type	(void);

CTK_SOURCE_AVAILABLE_IN_3_18
GtkWidget		*ctk_source_map_new		(void);

CTK_SOURCE_AVAILABLE_IN_3_18
void			 ctk_source_map_set_view	(GtkSourceMap  *map,
							 GtkSourceView *view);

CTK_SOURCE_AVAILABLE_IN_3_18
GtkSourceView		*ctk_source_map_get_view	(GtkSourceMap  *map);

G_END_DECLS

#endif /* CTK_SOURCE_MAP_H */
