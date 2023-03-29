/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
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

#ifndef CTK_SOURCE_SEARCH_CONTEXT_H
#define CTK_SOURCE_SEARCH_CONTEXT_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_SEARCH_CONTEXT             (ctk_source_search_context_get_type ())
#define CTK_SOURCE_SEARCH_CONTEXT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_SEARCH_CONTEXT, CtkSourceSearchContext))
#define CTK_SOURCE_SEARCH_CONTEXT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_SEARCH_CONTEXT, CtkSourceSearchContextClass))
#define CTK_SOURCE_IS_SEARCH_CONTEXT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_SEARCH_CONTEXT))
#define CTK_SOURCE_IS_SEARCH_CONTEXT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_SEARCH_CONTEXT))
#define CTK_SOURCE_SEARCH_CONTEXT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_SEARCH_CONTEXT, CtkSourceSearchContextClass))

typedef struct _CtkSourceSearchContextClass    CtkSourceSearchContextClass;
typedef struct _CtkSourceSearchContextPrivate  CtkSourceSearchContextPrivate;

struct _CtkSourceSearchContext
{
	GObject parent;

	CtkSourceSearchContextPrivate *priv;
};

struct _CtkSourceSearchContextClass
{
	GObjectClass parent_class;

	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_3_10
GType			 ctk_source_search_context_get_type			(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_3_10
CtkSourceSearchContext	*ctk_source_search_context_new				(CtkSourceBuffer	 *buffer,
										 CtkSourceSearchSettings *settings);

CTK_SOURCE_AVAILABLE_IN_3_10
CtkSourceBuffer		*ctk_source_search_context_get_buffer			(CtkSourceSearchContext  *search);

CTK_SOURCE_AVAILABLE_IN_3_10
CtkSourceSearchSettings	*ctk_source_search_context_get_settings			(CtkSourceSearchContext	 *search);

CTK_SOURCE_AVAILABLE_IN_3_10
gboolean		 ctk_source_search_context_get_highlight		(CtkSourceSearchContext  *search);

CTK_SOURCE_AVAILABLE_IN_3_10
void			 ctk_source_search_context_set_highlight		(CtkSourceSearchContext  *search,
										 gboolean                 highlight);

CTK_SOURCE_AVAILABLE_IN_3_16
CtkSourceStyle		*ctk_source_search_context_get_match_style		(CtkSourceSearchContext  *search);

CTK_SOURCE_AVAILABLE_IN_3_16
void			 ctk_source_search_context_set_match_style		(CtkSourceSearchContext  *search,
										 CtkSourceStyle          *match_style);

CTK_SOURCE_AVAILABLE_IN_3_10
GError			*ctk_source_search_context_get_regex_error		(CtkSourceSearchContext	 *search);

CTK_SOURCE_AVAILABLE_IN_3_10
gint			 ctk_source_search_context_get_occurrences_count	(CtkSourceSearchContext	 *search);

CTK_SOURCE_AVAILABLE_IN_3_10
gint			 ctk_source_search_context_get_occurrence_position	(CtkSourceSearchContext	 *search,
										 const CtkTextIter	 *match_start,
										 const CtkTextIter	 *match_end);

CTK_SOURCE_AVAILABLE_IN_4_0
gboolean		 ctk_source_search_context_forward			(CtkSourceSearchContext *search,
										 const CtkTextIter      *iter,
										 CtkTextIter            *match_start,
										 CtkTextIter            *match_end,
										 gboolean               *has_wrapped_around);

CTK_SOURCE_AVAILABLE_IN_3_10
void			 ctk_source_search_context_forward_async		(CtkSourceSearchContext	 *search,
										 const CtkTextIter	 *iter,
										 GCancellable		 *cancellable,
										 GAsyncReadyCallback	  callback,
										 gpointer		  user_data);

CTK_SOURCE_AVAILABLE_IN_4_0
gboolean		 ctk_source_search_context_forward_finish		(CtkSourceSearchContext  *search,
										 GAsyncResult            *result,
										 CtkTextIter             *match_start,
										 CtkTextIter             *match_end,
										 gboolean                *has_wrapped_around,
										 GError                 **error);

CTK_SOURCE_AVAILABLE_IN_4_0
gboolean		 ctk_source_search_context_backward			(CtkSourceSearchContext *search,
										 const CtkTextIter      *iter,
										 CtkTextIter            *match_start,
										 CtkTextIter            *match_end,
										 gboolean               *has_wrapped_around);

CTK_SOURCE_AVAILABLE_IN_3_10
void			 ctk_source_search_context_backward_async		(CtkSourceSearchContext	 *search,
										 const CtkTextIter	 *iter,
										 GCancellable		 *cancellable,
										 GAsyncReadyCallback	  callback,
										 gpointer		  user_data);

CTK_SOURCE_AVAILABLE_IN_4_0
gboolean		 ctk_source_search_context_backward_finish		(CtkSourceSearchContext  *search,
										 GAsyncResult            *result,
										 CtkTextIter             *match_start,
										 CtkTextIter             *match_end,
										 gboolean                *has_wrapped_around,
										 GError                 **error);

CTK_SOURCE_AVAILABLE_IN_4_0
gboolean		 ctk_source_search_context_replace			(CtkSourceSearchContext  *search,
										 CtkTextIter             *match_start,
										 CtkTextIter             *match_end,
										 const gchar             *replace,
										 gint                     replace_length,
										 GError                 **error);

CTK_SOURCE_AVAILABLE_IN_3_10
guint			 ctk_source_search_context_replace_all			(CtkSourceSearchContext	 *search,
										 const gchar		 *replace,
										 gint			  replace_length,
										 GError			**error);

G_GNUC_INTERNAL
void			 _ctk_source_search_context_update_highlight		(CtkSourceSearchContext	 *search,
										 const CtkTextIter	 *start,
										 const CtkTextIter	 *end,
										 gboolean		  synchronous);

G_END_DECLS

#endif /* CTK_SOURCE_SEARCH_CONTEXT_H */
