/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2007 - 2009 Jesús Barbero Rodríguez <chuchiperriman@gmail.com>
 * Copyright (C) 2009 - Jesse van den Kieboom <jessevdk@gnome.org>
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
#define CTK_SOURCE_COMPLETION(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_SOURCE_TYPE_COMPLETION, GtkSourceCompletion))
#define CTK_SOURCE_COMPLETION_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), CTK_SOURCE_TYPE_COMPLETION, GtkSourceCompletionClass))
#define CTK_SOURCE_IS_COMPLETION(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_SOURCE_TYPE_COMPLETION))
#define CTK_SOURCE_IS_COMPLETION_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_COMPLETION))
#define CTK_SOURCE_COMPLETION_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), CTK_SOURCE_TYPE_COMPLETION, GtkSourceCompletionClass))

/**
 * CTK_SOURCE_COMPLETION_ERROR:
 *
 * Error domain for the completion. Errors in this domain will be from the
 * #GtkSourceCompletionError enumeration. See #GError for more information on
 * error domains.
 */
#define CTK_SOURCE_COMPLETION_ERROR		(ctk_source_completion_error_quark ())

typedef struct _GtkSourceCompletionPrivate GtkSourceCompletionPrivate;
typedef struct _GtkSourceCompletionClass GtkSourceCompletionClass;

/**
 * GtkSourceCompletionError:
 * @CTK_SOURCE_COMPLETION_ERROR_ALREADY_BOUND: The #GtkSourceCompletionProvider
 * is already bound to the #GtkSourceCompletion object.
 * @CTK_SOURCE_COMPLETION_ERROR_NOT_BOUND: The #GtkSourceCompletionProvider is
 * not bound to the #GtkSourceCompletion object.
 *
 * An error code used with %CTK_SOURCE_COMPLETION_ERROR in a #GError returned
 * from a completion-related function.
 */
typedef enum _GtkSourceCompletionError
{
	CTK_SOURCE_COMPLETION_ERROR_ALREADY_BOUND = 0,
	CTK_SOURCE_COMPLETION_ERROR_NOT_BOUND
} GtkSourceCompletionError;

struct _GtkSourceCompletion
{
	GObject parent_instance;

	GtkSourceCompletionPrivate *priv;
};

struct _GtkSourceCompletionClass
{
	GObjectClass parent_class;

	gboolean 	(* proposal_activated)		(GtkSourceCompletion         *completion,
	                                                 GtkSourceCompletionProvider *provider,
							 GtkSourceCompletionProposal *proposal);
	void 		(* show)			(GtkSourceCompletion         *completion);
	void		(* hide)			(GtkSourceCompletion         *completion);
	void		(* populate_context)		(GtkSourceCompletion         *completion,
							 GtkSourceCompletionContext  *context);

	/* Actions */
	void		(* move_cursor)			(GtkSourceCompletion         *completion,
							 GtkScrollStep                step,
							 gint                         num);
	void		(* move_page)			(GtkSourceCompletion         *completion,
							 GtkScrollStep                step,
							 gint                         num);
	void		(* activate_proposal)		(GtkSourceCompletion         *completion);

	gpointer padding[20];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType		 ctk_source_completion_get_type			(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
GQuark		 ctk_source_completion_error_quark		(void);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	 ctk_source_completion_add_provider		(GtkSourceCompletion           *completion,
								 GtkSourceCompletionProvider   *provider,
								 GError                       **error);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	 ctk_source_completion_remove_provider		(GtkSourceCompletion           *completion,
								 GtkSourceCompletionProvider   *provider,
								 GError                       **error);

CTK_SOURCE_AVAILABLE_IN_ALL
GList		*ctk_source_completion_get_providers		(GtkSourceCompletion           *completion);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	 ctk_source_completion_start			(GtkSourceCompletion           *completion,
								 GList                         *providers,
								 GtkSourceCompletionContext    *context);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_completion_hide			(GtkSourceCompletion           *completion);

CTK_SOURCE_AVAILABLE_IN_ALL
GtkSourceCompletionInfo *
		 ctk_source_completion_get_info_window		(GtkSourceCompletion           *completion);

CTK_SOURCE_AVAILABLE_IN_ALL
GtkSourceView	*ctk_source_completion_get_view			(GtkSourceCompletion	       *completion);

CTK_SOURCE_AVAILABLE_IN_ALL
GtkSourceCompletionContext *
		 ctk_source_completion_create_context		(GtkSourceCompletion           *completion,
		 						 GtkTextIter                   *position);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_completion_block_interactive	(GtkSourceCompletion           *completion);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_completion_unblock_interactive	(GtkSourceCompletion           *completion);

G_GNUC_INTERNAL
void		 _ctk_source_completion_add_proposals		(GtkSourceCompletion           *completion,
								 GtkSourceCompletionContext    *context,
								 GtkSourceCompletionProvider   *provider,
								 GList                         *proposals,
								 gboolean                       finished);
G_END_DECLS

#endif /* CTK_SOURCE_COMPLETION_H */
