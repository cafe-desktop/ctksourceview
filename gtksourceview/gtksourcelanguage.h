/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2003 - Paolo Maggi <paolo.maggi@polito.it>
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

#ifndef CTK_SOURCE_LANGUAGE_H
#define CTK_SOURCE_LANGUAGE_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_LANGUAGE		(ctk_source_language_get_type ())
#define CTK_SOURCE_LANGUAGE(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_SOURCE_TYPE_LANGUAGE, GtkSourceLanguage))
#define CTK_SOURCE_LANGUAGE_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), CTK_SOURCE_TYPE_LANGUAGE, GtkSourceLanguageClass))
#define CTK_SOURCE_IS_LANGUAGE(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_SOURCE_TYPE_LANGUAGE))
#define CTK_SOURCE_IS_LANGUAGE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_LANGUAGE))
#define CTK_SOURCE_LANGUAGE_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_LANGUAGE, GtkSourceLanguageClass))


typedef struct _GtkSourceLanguageClass		GtkSourceLanguageClass;
typedef struct _GtkSourceLanguagePrivate	GtkSourceLanguagePrivate;

struct _GtkSourceLanguage
{
	GObject parent_instance;

	GtkSourceLanguagePrivate *priv;
};

struct _GtkSourceLanguageClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType		  ctk_source_language_get_type 		(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar	 *ctk_source_language_get_id		(GtkSourceLanguage *language);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar	 *ctk_source_language_get_name		(GtkSourceLanguage *language);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar	 *ctk_source_language_get_section	(GtkSourceLanguage *language);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	  ctk_source_language_get_hidden 	(GtkSourceLanguage *language);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar	 *ctk_source_language_get_metadata	(GtkSourceLanguage *language,
							 const gchar       *name);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar		**ctk_source_language_get_mime_types	(GtkSourceLanguage *language);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar		**ctk_source_language_get_globs		(GtkSourceLanguage *language);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar		**ctk_source_language_get_style_ids 	(GtkSourceLanguage *language);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar	*ctk_source_language_get_style_name	(GtkSourceLanguage *language,
							 const gchar       *style_id);

CTK_SOURCE_AVAILABLE_IN_3_4
const gchar	*ctk_source_language_get_style_fallback	(GtkSourceLanguage *language,
							 const gchar       *style_id);

G_END_DECLS

#endif /* CTK_SOURCE_LANGUAGE_H */

