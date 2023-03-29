/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2013, 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef CTK_SOURCE_BUFFER_PRIVATE_H
#define CTK_SOURCE_BUFFER_PRIVATE_H

#include <ctk/ctk.h>

#include "ctksourcetypes.h"
#include "ctksourcetypes-private.h"
#include "ctksourcebuffer.h"

G_BEGIN_DECLS

CTK_SOURCE_INTERNAL
void			 _ctk_source_buffer_update_syntax_highlight	(CtkSourceBuffer        *buffer,
									 const CtkTextIter      *start,
									 const CtkTextIter      *end,
									 gboolean                synchronous);

CTK_SOURCE_INTERNAL
void			 _ctk_source_buffer_update_search_highlight	(CtkSourceBuffer        *buffer,
									 const CtkTextIter      *start,
									 const CtkTextIter      *end,
									 gboolean                synchronous);

CTK_SOURCE_INTERNAL
CtkSourceMark		*_ctk_source_buffer_source_mark_next		(CtkSourceBuffer        *buffer,
									 CtkSourceMark          *mark,
									 const gchar            *category);

CTK_SOURCE_INTERNAL
CtkSourceMark		*_ctk_source_buffer_source_mark_prev		(CtkSourceBuffer        *buffer,
									 CtkSourceMark          *mark,
									 const gchar            *category);

CTK_SOURCE_INTERNAL
CtkTextTag		*_ctk_source_buffer_get_bracket_match_tag	(CtkSourceBuffer        *buffer);

CTK_SOURCE_INTERNAL
void			 _ctk_source_buffer_add_search_context		(CtkSourceBuffer        *buffer,
									 CtkSourceSearchContext *search_context);

CTK_SOURCE_INTERNAL
void			 _ctk_source_buffer_set_as_invalid_character	(CtkSourceBuffer        *buffer,
									 const CtkTextIter      *start,
									 const CtkTextIter      *end);

CTK_SOURCE_INTERNAL
gboolean		 _ctk_source_buffer_has_invalid_chars		(CtkSourceBuffer        *buffer);

CTK_SOURCE_INTERNAL
CtkSourceBracketMatchType
			 _ctk_source_buffer_find_bracket_match		(CtkSourceBuffer        *buffer,
									 const CtkTextIter      *pos,
									 CtkTextIter            *bracket,
									 CtkTextIter            *bracket_match);

CTK_SOURCE_INTERNAL
void			 _ctk_source_buffer_save_and_clear_selection	(CtkSourceBuffer        *buffer);

CTK_SOURCE_INTERNAL
void			 _ctk_source_buffer_restore_selection		(CtkSourceBuffer        *buffer);

CTK_SOURCE_INTERNAL
gboolean		 _ctk_source_buffer_is_undo_redo_enabled	(CtkSourceBuffer        *buffer);

CTK_SOURCE_INTERNAL
gboolean		_ctk_source_buffer_has_source_marks		(CtkSourceBuffer        *buffer);

CTK_SOURCE_INTERNAL
gboolean		_ctk_source_buffer_has_spaces_tag		(CtkSourceBuffer        *buffer);

G_END_DECLS

#endif /* CTK_SOURCE_BUFFER_PRIVATE_H */
