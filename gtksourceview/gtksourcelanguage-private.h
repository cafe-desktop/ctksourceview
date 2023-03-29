/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2003 - Paolo Maggi <paolo.maggi@polito.it>
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

#ifndef CTK_SOURCE_LANGUAGE_PRIVATE_H
#define CTK_SOURCE_LANGUAGE_PRIVATE_H

#include <glib.h>
#include "ctksourcecontextengine.h"
#include "ctksourcelanguagemanager.h"

G_BEGIN_DECLS

#define CTK_SOURCE_LANGUAGE_VERSION_1_0  100
#define CTK_SOURCE_LANGUAGE_VERSION_2_0  200

typedef struct _CtkSourceStyleInfo CtkSourceStyleInfo;

struct _CtkSourceStyleInfo
{
	gchar *name;
	gchar *map_to;
};

struct _CtkSourceLanguagePrivate
{
	gchar                    *lang_file_name;
	gchar                    *translation_domain;

	gchar                    *id;
	gchar                    *name;
	gchar                    *section;

	/* Maps ids to CtkSourceStyleInfo objects */
	/* Names of styles defined in other lang files are not stored */
	GHashTable               *styles;
	gboolean		  styles_loaded;

	gint                      version;
	gboolean                  hidden;

	GHashTable               *properties;

	CtkSourceLanguageManager *language_manager;

	CtkSourceContextData     *ctx_data;
};

G_GNUC_INTERNAL
CtkSourceLanguage 	 *_ctk_source_language_new_from_file 		(const gchar		   *filename,
									 CtkSourceLanguageManager  *lm);

G_GNUC_INTERNAL
CtkSourceLanguageManager *_ctk_source_language_get_language_manager 	(CtkSourceLanguage        *language);

G_GNUC_INTERNAL
const gchar		 *_ctk_source_language_manager_get_rng_file	(CtkSourceLanguageManager *lm);

G_GNUC_INTERNAL
gchar       		 *_ctk_source_language_translate_string 	(CtkSourceLanguage        *language,
									 const gchar              *string);

G_GNUC_INTERNAL
void 			  _ctk_source_language_define_language_styles	(CtkSourceLanguage        *language);

G_GNUC_INTERNAL
gboolean 		  _ctk_source_language_file_parse_version2	(CtkSourceLanguage        *language,
									 CtkSourceContextData     *ctx_data);

G_GNUC_INTERNAL
CtkSourceEngine 	 *_ctk_source_language_create_engine		(CtkSourceLanguage	  *language);

/* Utility functions for CtkSourceStyleInfo */
G_GNUC_INTERNAL
CtkSourceStyleInfo 	 *_ctk_source_style_info_new 			(const gchar		  *name,
									 const gchar              *map_to);

G_GNUC_INTERNAL
CtkSourceStyleInfo 	 *_ctk_source_style_info_copy			(CtkSourceStyleInfo       *info);

G_GNUC_INTERNAL
void			  _ctk_source_style_info_free			(CtkSourceStyleInfo       *info);

G_END_DECLS

#endif  /* CTK_SOURCE_LANGUAGE_PRIVATE_H */

