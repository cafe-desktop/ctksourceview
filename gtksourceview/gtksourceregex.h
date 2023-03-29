/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2003 - Gustavo Giráldez <gustavo.giraldez@gmx.net>
 * Copyright (C) 2005, 2006 - Marco Barisione, Emanuele Aina
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

#ifndef CTK_SOURCE_REGEX_H
#define CTK_SOURCE_REGEX_H

#include <glib.h>
#include "ctksourcetypes-private.h"

G_BEGIN_DECLS

CTK_SOURCE_INTERNAL
GtkSourceRegex	*_ctk_source_regex_new		(const gchar         *pattern,
						 GRegexCompileFlags   flags,
						 GError             **error);

CTK_SOURCE_INTERNAL
GtkSourceRegex	*_ctk_source_regex_ref		(GtkSourceRegex *regex);

CTK_SOURCE_INTERNAL
void		 _ctk_source_regex_unref	(GtkSourceRegex *regex);

CTK_SOURCE_INTERNAL
GtkSourceRegex	*_ctk_source_regex_resolve	(GtkSourceRegex *regex,
						 GtkSourceRegex *start_regex,
						 const gchar    *matched_text);

CTK_SOURCE_INTERNAL
gboolean	 _ctk_source_regex_is_resolved	(GtkSourceRegex *regex);

CTK_SOURCE_INTERNAL
gboolean	_ctk_source_regex_match		(GtkSourceRegex *regex,
						 const gchar    *line,
						 gint             byte_length,
						 gint             byte_pos);

CTK_SOURCE_INTERNAL
gchar		*_ctk_source_regex_fetch	(GtkSourceRegex *regex,
						 gint            num);

CTK_SOURCE_INTERNAL
void		 _ctk_source_regex_fetch_pos	(GtkSourceRegex *regex,
						 const gchar    *text,
						 gint            num,
						 gint           *start_pos, /* character offsets */
						 gint           *end_pos);  /* character offsets */

CTK_SOURCE_INTERNAL
void		 _ctk_source_regex_fetch_pos_bytes (GtkSourceRegex *regex,
						    gint            num,
						    gint           *start_pos_p, /* byte offsets */
						    gint           *end_pos_p);  /* byte offsets */

CTK_SOURCE_INTERNAL
void		 _ctk_source_regex_fetch_named_pos (GtkSourceRegex *regex,
						    const gchar    *text,
						    const gchar    *name,
						    gint           *start_pos, /* character offsets */
						    gint           *end_pos);  /* character offsets */

CTK_SOURCE_INTERNAL
const gchar	*_ctk_source_regex_get_pattern	(GtkSourceRegex *regex);

G_END_DECLS

#endif /* CTK_SOURCE_REGEX_H */
