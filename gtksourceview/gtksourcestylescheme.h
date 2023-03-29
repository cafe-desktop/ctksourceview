/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2003 - Paolo Maggi <paolo.maggi@polito.it>
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

 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CTK_SOURCE_STYLE_SCHEME_H
#define CTK_SOURCE_STYLE_SCHEME_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_STYLE_SCHEME             (ctk_source_style_scheme_get_type ())
#define CTK_SOURCE_STYLE_SCHEME(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_STYLE_SCHEME, CtkSourceStyleScheme))
#define CTK_SOURCE_STYLE_SCHEME_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_STYLE_SCHEME, CtkSourceStyleSchemeClass))
#define CTK_SOURCE_IS_STYLE_SCHEME(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_STYLE_SCHEME))
#define CTK_SOURCE_IS_STYLE_SCHEME_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_STYLE_SCHEME))
#define CTK_SOURCE_STYLE_SCHEME_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_STYLE_SCHEME, CtkSourceStyleSchemeClass))

typedef struct _CtkSourceStyleSchemePrivate      CtkSourceStyleSchemePrivate;
typedef struct _CtkSourceStyleSchemeClass        CtkSourceStyleSchemeClass;

struct _CtkSourceStyleScheme
{
	GObject base;
	CtkSourceStyleSchemePrivate *priv;
};

struct _CtkSourceStyleSchemeClass
{
	GObjectClass base_class;

	/* Padding for future expansion */
	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType			 ctk_source_style_scheme_get_type			(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar             *ctk_source_style_scheme_get_id				(CtkSourceStyleScheme *scheme);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar             *ctk_source_style_scheme_get_name			(CtkSourceStyleScheme *scheme);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar             *ctk_source_style_scheme_get_description		(CtkSourceStyleScheme *scheme);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar * const *	 ctk_source_style_scheme_get_authors			(CtkSourceStyleScheme *scheme);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar             *ctk_source_style_scheme_get_filename			(CtkSourceStyleScheme *scheme);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceStyle		*ctk_source_style_scheme_get_style			(CtkSourceStyleScheme *scheme,
										 const gchar          *style_id);

G_GNUC_INTERNAL
CtkSourceStyleScheme	*_ctk_source_style_scheme_new_from_file			(const gchar          *filename);

G_GNUC_INTERNAL
CtkSourceStyleScheme	*_ctk_source_style_scheme_get_default			(void);

G_GNUC_INTERNAL
const gchar		*_ctk_source_style_scheme_get_parent_id			(CtkSourceStyleScheme *scheme);

G_GNUC_INTERNAL
void			 _ctk_source_style_scheme_set_parent			(CtkSourceStyleScheme *scheme,
										 CtkSourceStyleScheme *parent_scheme);

G_GNUC_INTERNAL
void			 _ctk_source_style_scheme_apply				(CtkSourceStyleScheme *scheme,
										 CtkSourceView        *view);

G_GNUC_INTERNAL
void			 _ctk_source_style_scheme_unapply			(CtkSourceStyleScheme *scheme,
										 CtkSourceView        *view);

G_GNUC_INTERNAL
CtkSourceStyle		*_ctk_source_style_scheme_get_matching_brackets_style	(CtkSourceStyleScheme *scheme);

G_GNUC_INTERNAL
CtkSourceStyle		*_ctk_source_style_scheme_get_right_margin_style	(CtkSourceStyleScheme *scheme);

G_GNUC_INTERNAL
CtkSourceStyle          *_ctk_source_style_scheme_get_draw_spaces_style		(CtkSourceStyleScheme *scheme);

G_GNUC_INTERNAL
gboolean		 _ctk_source_style_scheme_get_current_line_color	(CtkSourceStyleScheme *scheme,
										 GdkRGBA              *color);

G_GNUC_INTERNAL
gboolean		 _ctk_source_style_scheme_get_background_pattern_color	(CtkSourceStyleScheme *scheme,
										 GdkRGBA              *color);

G_END_DECLS

#endif  /* CTK_SOURCE_STYLE_SCHEME_H */
