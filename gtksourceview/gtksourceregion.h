/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * ctksourceregion.h - GtkTextMark-based region utility
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2002 Gustavo Giráldez <gustavo.giraldez@gmx.net>
 * Copyright (C) 2016 Sébastien Wilmet <swilmet@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CTK_SOURCE_REGION_H
#define CTK_SOURCE_REGION_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourceversion.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_REGION (ctk_source_region_get_type ())

CTK_SOURCE_AVAILABLE_IN_3_22
G_DECLARE_DERIVABLE_TYPE (GtkSourceRegion, ctk_source_region,
			  CTK_SOURCE, REGION,
			  GObject)

struct _GtkSourceRegionClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	gpointer padding[10];
};

/**
 * GtkSourceRegionIter:
 *
 * #GtkSourceRegionIter is an opaque datatype; ignore all its fields.
 * Initialize the iter with ctk_source_region_get_start_region_iter().
 *
 * Since: 3.22
 */
typedef struct _GtkSourceRegionIter GtkSourceRegionIter;
struct _GtkSourceRegionIter
{
	/*< private >*/
	gpointer dummy1;
	guint32  dummy2;
	gpointer dummy3;
};

CTK_SOURCE_AVAILABLE_IN_3_22
GtkSourceRegion *	ctk_source_region_new			(GtkTextBuffer *buffer);

CTK_SOURCE_AVAILABLE_IN_3_22
GtkTextBuffer *		ctk_source_region_get_buffer		(GtkSourceRegion *region);

CTK_SOURCE_AVAILABLE_IN_3_22
void			ctk_source_region_add_subregion		(GtkSourceRegion   *region,
								 const GtkTextIter *_start,
								 const GtkTextIter *_end);

CTK_SOURCE_AVAILABLE_IN_3_22
void			ctk_source_region_add_region		(GtkSourceRegion *region,
								 GtkSourceRegion *region_to_add);

CTK_SOURCE_AVAILABLE_IN_3_22
void			ctk_source_region_subtract_subregion	(GtkSourceRegion   *region,
								 const GtkTextIter *_start,
								 const GtkTextIter *_end);

CTK_SOURCE_AVAILABLE_IN_3_22
void			ctk_source_region_subtract_region	(GtkSourceRegion *region,
								 GtkSourceRegion *region_to_subtract);

CTK_SOURCE_AVAILABLE_IN_3_22
GtkSourceRegion *	ctk_source_region_intersect_subregion	(GtkSourceRegion   *region,
								 const GtkTextIter *_start,
								 const GtkTextIter *_end);

CTK_SOURCE_AVAILABLE_IN_3_22
GtkSourceRegion *	ctk_source_region_intersect_region	(GtkSourceRegion *region1,
								 GtkSourceRegion *region2);

CTK_SOURCE_AVAILABLE_IN_3_22
gboolean		ctk_source_region_is_empty		(GtkSourceRegion *region);

CTK_SOURCE_AVAILABLE_IN_3_22
gboolean		ctk_source_region_get_bounds		(GtkSourceRegion *region,
								 GtkTextIter     *start,
								 GtkTextIter     *end);

CTK_SOURCE_AVAILABLE_IN_3_22
void			ctk_source_region_get_start_region_iter	(GtkSourceRegion     *region,
								 GtkSourceRegionIter *iter);

CTK_SOURCE_AVAILABLE_IN_3_22
gboolean		ctk_source_region_iter_is_end		(GtkSourceRegionIter *iter);

CTK_SOURCE_AVAILABLE_IN_3_22
gboolean		ctk_source_region_iter_next		(GtkSourceRegionIter *iter);

CTK_SOURCE_AVAILABLE_IN_3_22
gboolean		ctk_source_region_iter_get_subregion	(GtkSourceRegionIter *iter,
								 GtkTextIter         *start,
								 GtkTextIter         *end);

CTK_SOURCE_AVAILABLE_IN_3_22
gchar *			ctk_source_region_to_string		(GtkSourceRegion *region);

G_END_DECLS

#endif /* CTK_SOURCE_REGION_H */
