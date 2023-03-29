/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 1999-2002 - Mikael Hermansson <tyan@linux.se>,
 *                           Chris Phelps <chicane@reninet.com> and
 *                           Jeroen Zwartepoorte <jeroen@xs4all.nl>
 * Copyright (C) 2003 - Paolo Maggi, Gustavo Giráldez
 * Copyright (C) 2014 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef CTK_SOURCE_BUFFER_H
#define CTK_SOURCE_BUFFER_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_BUFFER            (ctk_source_buffer_get_type ())
#define CTK_SOURCE_BUFFER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_BUFFER, CtkSourceBuffer))
#define CTK_SOURCE_BUFFER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_BUFFER, CtkSourceBufferClass))
#define CTK_SOURCE_IS_BUFFER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_BUFFER))
#define CTK_SOURCE_IS_BUFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_BUFFER))
#define CTK_SOURCE_BUFFER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_BUFFER, CtkSourceBufferClass))

typedef struct _CtkSourceBufferClass		CtkSourceBufferClass;
typedef struct _CtkSourceBufferPrivate		CtkSourceBufferPrivate;

/**
 * CtkSourceBracketMatchType:
 * @CTK_SOURCE_BRACKET_MATCH_NONE: there is no bracket to match.
 * @CTK_SOURCE_BRACKET_MATCH_OUT_OF_RANGE: matching a bracket
 *  failed because the maximum range was reached.
 * @CTK_SOURCE_BRACKET_MATCH_NOT_FOUND: a matching bracket was not found.
 * @CTK_SOURCE_BRACKET_MATCH_FOUND: a matching bracket was found.
 */
typedef enum _CtkSourceBracketMatchType
{
	CTK_SOURCE_BRACKET_MATCH_NONE,
	CTK_SOURCE_BRACKET_MATCH_OUT_OF_RANGE,
	CTK_SOURCE_BRACKET_MATCH_NOT_FOUND,
	CTK_SOURCE_BRACKET_MATCH_FOUND
} CtkSourceBracketMatchType;

/**
 * CtkSourceChangeCaseType:
 * @CTK_SOURCE_CHANGE_CASE_LOWER: change case to lowercase.
 * @CTK_SOURCE_CHANGE_CASE_UPPER: change case to uppercase.
 * @CTK_SOURCE_CHANGE_CASE_TOGGLE: toggle case of each character.
 * @CTK_SOURCE_CHANGE_CASE_TITLE: capitalize each word.
 *
 * Since: 3.12
 */
typedef enum _CtkSourceChangeCaseType
{
	CTK_SOURCE_CHANGE_CASE_LOWER,
	CTK_SOURCE_CHANGE_CASE_UPPER,
	CTK_SOURCE_CHANGE_CASE_TOGGLE,
	CTK_SOURCE_CHANGE_CASE_TITLE
} CtkSourceChangeCaseType;

/**
 * CtkSourceSortFlags:
 * @CTK_SOURCE_SORT_FLAGS_NONE: no flags specified
 * @CTK_SOURCE_SORT_FLAGS_CASE_SENSITIVE: case sensitive sort
 * @CTK_SOURCE_SORT_FLAGS_REVERSE_ORDER: sort in reverse order
 * @CTK_SOURCE_SORT_FLAGS_REMOVE_DUPLICATES: remove duplicates
 *
 * Since: 3.18
 */
typedef enum _CtkSourceSortFlags
{
	CTK_SOURCE_SORT_FLAGS_NONE              = 0,
	CTK_SOURCE_SORT_FLAGS_CASE_SENSITIVE    = 1 << 0,
	CTK_SOURCE_SORT_FLAGS_REVERSE_ORDER     = 1 << 1,
	CTK_SOURCE_SORT_FLAGS_REMOVE_DUPLICATES = 1 << 2,
} CtkSourceSortFlags;

struct _CtkSourceBuffer
{
	CtkTextBuffer parent_instance;

	CtkSourceBufferPrivate *priv;
};

struct _CtkSourceBufferClass
{
	CtkTextBufferClass parent_class;

	/* Signals */
	void (*undo) (CtkSourceBuffer *buffer);
	void (*redo) (CtkSourceBuffer *buffer);

	void (*bracket_matched) (CtkSourceBuffer           *buffer,
				 CtkTextIter               *iter,
				 CtkSourceBracketMatchType  state);

	/* Padding for future expansion */
	gpointer padding[20];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType			 ctk_source_buffer_get_type				(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceBuffer	 	*ctk_source_buffer_new					(CtkTextTagTable        *table);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceBuffer 	*ctk_source_buffer_new_with_language			(CtkSourceLanguage      *language);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		 ctk_source_buffer_get_highlight_syntax			(CtkSourceBuffer        *buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_buffer_set_highlight_syntax			(CtkSourceBuffer        *buffer,
										 gboolean                highlight);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		 ctk_source_buffer_get_highlight_matching_brackets	(CtkSourceBuffer        *buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_buffer_set_highlight_matching_brackets	(CtkSourceBuffer        *buffer,
										 gboolean                highlight);

CTK_SOURCE_AVAILABLE_IN_ALL
gint			 ctk_source_buffer_get_max_undo_levels			(CtkSourceBuffer        *buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_buffer_set_max_undo_levels			(CtkSourceBuffer        *buffer,
										 gint                    max_undo_levels);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceLanguage 	*ctk_source_buffer_get_language				(CtkSourceBuffer        *buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_buffer_set_language				(CtkSourceBuffer        *buffer,
										 CtkSourceLanguage      *language);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		 ctk_source_buffer_can_undo				(CtkSourceBuffer        *buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		 ctk_source_buffer_can_redo				(CtkSourceBuffer        *buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceStyleScheme    *ctk_source_buffer_get_style_scheme			(CtkSourceBuffer        *buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_buffer_set_style_scheme			(CtkSourceBuffer        *buffer,
										 CtkSourceStyleScheme   *scheme);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_buffer_ensure_highlight			(CtkSourceBuffer        *buffer,
										 const CtkTextIter      *start,
										 const CtkTextIter      *end);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_buffer_undo					(CtkSourceBuffer        *buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_buffer_redo					(CtkSourceBuffer        *buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_buffer_begin_not_undoable_action		(CtkSourceBuffer	*buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_buffer_end_not_undoable_action		(CtkSourceBuffer	*buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceMark		*ctk_source_buffer_create_source_mark			(CtkSourceBuffer        *buffer,
										 const gchar            *name,
										 const gchar            *category,
										 const CtkTextIter      *where);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		 ctk_source_buffer_forward_iter_to_source_mark		(CtkSourceBuffer        *buffer,
										 CtkTextIter            *iter,
										 const gchar            *category);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		 ctk_source_buffer_backward_iter_to_source_mark		(CtkSourceBuffer        *buffer,
										 CtkTextIter            *iter,
										 const gchar            *category);

CTK_SOURCE_AVAILABLE_IN_ALL
GSList			*ctk_source_buffer_get_source_marks_at_iter		(CtkSourceBuffer        *buffer,
										 CtkTextIter            *iter,
										 const gchar            *category);

CTK_SOURCE_AVAILABLE_IN_ALL
GSList			*ctk_source_buffer_get_source_marks_at_line		(CtkSourceBuffer        *buffer,
										 gint 			 line,
										 const gchar		*category);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_buffer_remove_source_marks			(CtkSourceBuffer        *buffer,
										 const CtkTextIter      *start,
										 const CtkTextIter      *end,
										 const gchar            *category);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		 ctk_source_buffer_iter_has_context_class		(CtkSourceBuffer	*buffer,
										 const CtkTextIter	*iter,
										 const gchar            *context_class);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar		       **ctk_source_buffer_get_context_classes_at_iter		(CtkSourceBuffer	*buffer,
										 const CtkTextIter	*iter);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		 ctk_source_buffer_iter_forward_to_context_class_toggle	(CtkSourceBuffer	*buffer,
										 CtkTextIter		*iter,
										 const gchar		*context_class);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		 ctk_source_buffer_iter_backward_to_context_class_toggle
										(CtkSourceBuffer	*buffer,
										 CtkTextIter		*iter,
										 const gchar		*context_class);

CTK_SOURCE_AVAILABLE_IN_3_12
void			 ctk_source_buffer_change_case				(CtkSourceBuffer        *buffer,
										 CtkSourceChangeCaseType case_type,
										 CtkTextIter            *start,
										 CtkTextIter            *end);

CTK_SOURCE_AVAILABLE_IN_3_16
void			 ctk_source_buffer_join_lines				(CtkSourceBuffer        *buffer,
										 CtkTextIter            *start,
										 CtkTextIter            *end);

CTK_SOURCE_AVAILABLE_IN_3_18
void			 ctk_source_buffer_sort_lines				(CtkSourceBuffer        *buffer,
										 CtkTextIter            *start,
										 CtkTextIter            *end,
										 CtkSourceSortFlags     flags,
										 gint                   column);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceUndoManager	*ctk_source_buffer_get_undo_manager			(CtkSourceBuffer	*buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_buffer_set_undo_manager			(CtkSourceBuffer	*buffer,
										 CtkSourceUndoManager	*manager);

CTK_SOURCE_AVAILABLE_IN_3_14
void			 ctk_source_buffer_set_implicit_trailing_newline	(CtkSourceBuffer        *buffer,
										 gboolean                implicit_trailing_newline);

CTK_SOURCE_AVAILABLE_IN_3_14
gboolean		 ctk_source_buffer_get_implicit_trailing_newline	(CtkSourceBuffer        *buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkTextTag		*ctk_source_buffer_create_source_tag			(CtkSourceBuffer        *buffer,
										 const gchar            *tag_name,
										 const gchar            *first_property_name,
										 ...);

G_END_DECLS

#endif /* CTK_SOURCE_BUFFER_H */
