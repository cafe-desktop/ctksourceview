/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
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

#ifndef CTK_SOURCE_COMPLETION_CONTEXT_H
#define CTK_SOURCE_COMPLETION_CONTEXT_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_COMPLETION_CONTEXT		(ctk_source_completion_context_get_type ())
#define CTK_SOURCE_COMPLETION_CONTEXT(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_CONTEXT, CtkSourceCompletionContext))
#define CTK_SOURCE_COMPLETION_CONTEXT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_COMPLETION_CONTEXT, CtkSourceCompletionContextClass))
#define CTK_SOURCE_IS_COMPLETION_CONTEXT(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_COMPLETION_CONTEXT))
#define CTK_SOURCE_IS_COMPLETION_CONTEXT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_COMPLETION_CONTEXT))
#define CTK_SOURCE_COMPLETION_CONTEXT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_COMPLETION_CONTEXT, CtkSourceCompletionContextClass))

typedef struct _CtkSourceCompletionContextClass		CtkSourceCompletionContextClass;
typedef struct _CtkSourceCompletionContextPrivate	CtkSourceCompletionContextPrivate;

/**
 * CtkSourceCompletionActivation:
 * @CTK_SOURCE_COMPLETION_ACTIVATION_NONE: None.
 * @CTK_SOURCE_COMPLETION_ACTIVATION_INTERACTIVE: Interactive activation. By
 * default, it occurs on each insertion in the #CtkTextBuffer. This can be
 * blocked temporarily with ctk_source_completion_block_interactive().
 * @CTK_SOURCE_COMPLETION_ACTIVATION_USER_REQUESTED: User requested activation.
 * By default, it occurs when the user presses
 * <keycombo><keycap>Control</keycap><keycap>space</keycap></keycombo>.
 */
typedef enum _CtkSourceCompletionActivation
{
	CTK_SOURCE_COMPLETION_ACTIVATION_NONE = 0,
	CTK_SOURCE_COMPLETION_ACTIVATION_INTERACTIVE = 1 << 0,
	CTK_SOURCE_COMPLETION_ACTIVATION_USER_REQUESTED = 1 << 1
} CtkSourceCompletionActivation;

struct _CtkSourceCompletionContext {
	GInitiallyUnowned parent;

	CtkSourceCompletionContextPrivate *priv;
};

struct _CtkSourceCompletionContextClass {
	GInitiallyUnownedClass parent_class;

	void (*cancelled) 	(CtkSourceCompletionContext          *context);

	/* Padding for future expansion */
	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType		 ctk_source_completion_context_get_type (void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_completion_context_add_proposals 	(CtkSourceCompletionContext   *context,
								 CtkSourceCompletionProvider  *provider,
								 GList                        *proposals,
								 gboolean                      finished);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	 ctk_source_completion_context_get_iter		(CtkSourceCompletionContext   *context,
								 CtkTextIter                  *iter);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceCompletionActivation
		 ctk_source_completion_context_get_activation	(CtkSourceCompletionContext   *context);

G_GNUC_INTERNAL
CtkSourceCompletionContext *
		_ctk_source_completion_context_new		(CtkSourceCompletion          *completion,
								 CtkTextIter                  *position);

G_GNUC_INTERNAL
void		_ctk_source_completion_context_cancel		(CtkSourceCompletionContext   *context);

G_END_DECLS

#endif /* CTK_SOURCE_COMPLETION_CONTEXT_H */
