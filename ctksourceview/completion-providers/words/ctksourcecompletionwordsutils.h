/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- *
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2009 - Jesse van den Kieboom
 * Copyright (C) 2013 - Sébastien Wilmet
 *
 * ctksourceview is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ctksourceview is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CTK_SOURCE_COMPLETION_WORDS_UTILS_H
#define CTK_SOURCE_COMPLETION_WORDS_UTILS_H

#include <ctk/ctk.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
GSList		*_ctk_source_completion_words_utils_scan_words		(gchar *text,
									 guint  minimum_word_size);

G_GNUC_INTERNAL
gchar		*_ctk_source_completion_words_utils_get_end_word	(gchar *text);

G_GNUC_INTERNAL
void		 _ctk_source_completion_words_utils_adjust_region	(CtkTextIter *start,
									 CtkTextIter *end);

G_GNUC_INTERNAL
void		 _ctk_source_completion_words_utils_check_scan_region	(const CtkTextIter *start,
									 const CtkTextIter *end);

G_END_DECLS

#endif /* CTK_SOURCE_COMPLETION_WORDS_UTILS_H */
