/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2014, 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef CTK_SOURCE_ITER_H
#define CTK_SOURCE_ITER_H

#include <ctk/ctk.h>
#include "ctksourcetypes-private.h"

G_BEGIN_DECLS

/* Semi-public functions. */

CTK_SOURCE_INTERNAL
gboolean	_ctk_source_iter_forward_visible_word_end		(CtkTextIter *iter);

CTK_SOURCE_INTERNAL
gboolean	_ctk_source_iter_forward_visible_word_ends		(CtkTextIter *iter,
									 gint         count);

CTK_SOURCE_INTERNAL
gboolean	_ctk_source_iter_backward_visible_word_start		(CtkTextIter *iter);

CTK_SOURCE_INTERNAL
gboolean	_ctk_source_iter_backward_visible_word_starts		(CtkTextIter *iter,
									 gint         count);

CTK_SOURCE_INTERNAL
void		_ctk_source_iter_extend_selection_word			(const CtkTextIter *location,
									 CtkTextIter       *start,
									 CtkTextIter       *end);

CTK_SOURCE_INTERNAL
gboolean	_ctk_source_iter_starts_extra_natural_word		(const CtkTextIter *iter,
									 gboolean           visible);

CTK_SOURCE_INTERNAL
gboolean	_ctk_source_iter_ends_extra_natural_word		(const CtkTextIter *iter,
									 gboolean           visible);

CTK_SOURCE_INTERNAL
void		_ctk_source_iter_get_leading_spaces_end_boundary	(const CtkTextIter *iter,
									 CtkTextIter       *leading_end);

CTK_SOURCE_INTERNAL
void		_ctk_source_iter_get_trailing_spaces_start_boundary	(const CtkTextIter *iter,
									 CtkTextIter       *trailing_start);

/* Internal functions, in the header for unit tests. */

CTK_SOURCE_INTERNAL
void		_ctk_source_iter_forward_full_word_end			(CtkTextIter *iter);

CTK_SOURCE_INTERNAL
void		_ctk_source_iter_backward_full_word_start		(CtkTextIter *iter);

CTK_SOURCE_INTERNAL
gboolean	_ctk_source_iter_starts_full_word			(const CtkTextIter *iter);

CTK_SOURCE_INTERNAL
gboolean	_ctk_source_iter_ends_full_word				(const CtkTextIter *iter);

CTK_SOURCE_INTERNAL
void		_ctk_source_iter_forward_extra_natural_word_end		(CtkTextIter *iter);

CTK_SOURCE_INTERNAL
void		_ctk_source_iter_backward_extra_natural_word_start	(CtkTextIter *iter);

CTK_SOURCE_INTERNAL
gboolean	_ctk_source_iter_starts_word				(const CtkTextIter *iter);

CTK_SOURCE_INTERNAL
gboolean	_ctk_source_iter_ends_word				(const CtkTextIter *iter);

CTK_SOURCE_INTERNAL
gboolean	_ctk_source_iter_inside_word				(const CtkTextIter *iter);

G_END_DECLS

#endif /* CTK_SOURCE_ITER_H */
