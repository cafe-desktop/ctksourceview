/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2007 - 2009 Jesús Barbero Rodríguez <chuchiperriman@gmail.com>
 * Copyright (C) 2009 - Jesse van den Kieboom <jessevdk@gnome.org>
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

#ifndef CTK_SOURCE_COMPLETION_H
#define CTK_SOURCE_COMPLETION_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

/*
 * Type checking and casting macros
 */
#define CTK_SOURCE_TYPE_COMPLETION              (ctk_source_completion_get_type())
#define CTK_SOURCE_COMPLETION(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_SOURCE_TYPE_COMPLETION, CtkSourceCompletion))
#define CTK_SOURCE_COMPLETION_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), CTK_SOURCE_TYPE_COMPLETION, CtkSourceCompletionClass))
#define CTK_SOURCE_IS_COMPLETION(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_SOURCE_TYPE_COMPLETION))
#define CTK_SOURCE_IS_COMPLETION_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_COMPLETION))
#define CTK_SOURCE_COMPLETION_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_SOURCE_TYPE_COMPLETION, CtkSourceCompletionClass))

/**
 * CTK_SOURCE_COMPLETION_ERROR:
 *
 * Error domain for the completion. Errors in this domain will be from the
 * #CtkSourceCompletionError enumeration. See #GError for more information on
 * error domains.
 */
#define CTK_SOURCE_COMPLETION_ERROR		(ctk_source_completion_error_quark ())

typedef struct _CtkSourceCompletionPrivate CtkSourceCompletionPrivate;
typedef struct _CtkSourceCompletionClass CtkSourceCompletionClass;

/**
 * CtkSourceCompletionError:
 * @CTK_SOURCE_COMPLETION_ERROR_ALREADY_BOUND: The #CtkSourceCompletionProvider
 * is already bound to the #CtkSourceCompletion object.
 * @CTK_SOURCE_COMPLETION_ERROR_NOT_BOUND: The #CtkSourceCompletionProvider is
 * not bound to the #CtkSourceCompletion object.
 *
 * An error code used with %CTK_SOURCE_COMPLETION_ERROR in a #GError returned
 * from a completion-related function.
 */
typedef enum _CtkSourceCompletionError
{
	CTK_SOURCE_COMPLETION_ERROR_ALREADY_BOUND = 0,
	CTK_SOURCE_COMPLETION_ERROR_NOT_BOUND
} CtkSourceCompletionError;

struct _CtkSourceCompletion
{
	GObject parent_instance;

	CtkSourceCompletionPrivate *priv;
};

struct _CtkSourceCompletionClass
{
	GObjectClass parent_class;

	gboolean 	(* proposal_activated)		(CtkSourceCompletion         *completion,
	                                                 CtkSourceCompletionProvider *provider,
							 CtkSourceCompletionProposal *proposal);
	void 		(* show)			(CtkSourceCompletion         *completion);
	void		(* hide)			(CtkSourceCompletion         *completion);
	void		(* populate_context)		(CtkSourceCompletion         *completion,
							 CtkSourceCompletionContext  *context);

	/* Actions */
	void		(* move_cursor)			(CtkSourceCompletion         *completion,
							 CtkScrollStep                step,
							 gint                         num);
	void		(* move_page)			(CtkSourceCompletion         *completion,
							 CtkScrollStep                step,
							 gint                         num);
	void		(* activate_proposal)		(CtkSourceCompletion         *completion);

	gpointer padding[20];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType		 ctk_source_completion_get_type			(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
GQuark		 ctk_source_completion_error_quark		(void);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	 ctk_source_completion_add_provider		(CtkSourceCompletion           *completion,
								 CtkSourceCompletionProvider   *provider,
								 GError                       **error);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	 ctk_source_completion_remove_provider		(CtkSourceCompletion           *completion,
								 CtkSourceCompletionProvider   *provider,
								 GError                       **error);

CTK_SOURCE_AVAILABLE_IN_ALL
GList		*ctk_source_completion_get_providers		(CtkSourceCompletion           *completion);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	 ctk_source_completion_start			(CtkSourceCompletion           *completion,
								 GList                         *providers,
								 CtkSourceCompletionContext    *context);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_completion_hide			(CtkSourceCompletion           *completion);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceCompletionInfo *
		 ctk_source_completion_get_info_window		(CtkSourceCompletion           *completion);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceView	*ctk_source_completion_get_view			(CtkSourceCompletion	       *completion);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceCompletionContext *
		 ctk_source_completion_create_context		(CtkSourceCompletion           *completion,
		 						 CtkTextIter                   *position);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_completion_block_interactive	(CtkSourceCompletion           *completion);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_completion_unblock_interactive	(CtkSourceCompletion           *completion);

G_GNUC_INTERNAL
void		 _ctk_source_completion_add_proposals		(CtkSourceCompletion           *completion,
								 CtkSourceCompletionContext    *context,
								 CtkSourceCompletionProvider   *provider,
								 GList                         *proposals,
								 gboolean                       finished);
G_END_DECLS

#endif /* CTK_SOURCE_COMPLETION_H */
