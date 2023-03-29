/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- *
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

/**
 * SECTION:completioncontext
 * @title: CtkSourceCompletionContext
 * @short_description: The context of a completion
 *
 * Initially, the completion window is hidden. For a completion to occur, it has
 * to be activated. The different possible activations are listed in
 * #CtkSourceCompletionActivation. When an activation occurs, a
 * #CtkSourceCompletionContext object is created, and the eligible providers are
 * asked to add proposals with ctk_source_completion_context_add_proposals().
 *
 * If no proposals are added, the completion window remains hidden, and the
 * context is destroyed.
 *
 * On the other hand, if proposals are added, the completion window becomes
 * visible, and the user can choose a proposal. If the user is not happy with
 * the shown proposals, he or she can insert or delete characters, to modify the
 * completion context and therefore hoping to see the proposal he or she wants.
 * This means that when an insertion or deletion occurs in the #CtkTextBuffer
 * when the completion window is visible, the eligible providers are again asked
 * to add proposals. The #CtkSourceCompletionContext:activation remains the
 * same in this case.
 *
 * When the completion window is hidden, the interactive completion is triggered
 * only on insertion in the buffer, not on deletion. Once the completion window
 * is visible, then on each insertion or deletion, there is a new population and
 * the providers are asked to add proposals. If there are no more proposals, the
 * completion window disappears. So if you want to keep the completion window
 * visible, but there are no proposals, you can insert a dummy proposal named
 * "No proposals". For example, the user types progressively the name of
 * a function, and some proposals appear. The user types a bad character and
 * there are no proposals anymore. What the user wants is to delete the last
 * character, and see the previous proposals. If the completion window
 * disappears, the previous proposals will not reappear on the character
 * deletion.
 *
 * A #CtkTextIter is associated with the context, this is where the completion
 * takes place. With this #CtkTextIter, you can get the associated
 * #CtkTextBuffer with ctk_text_iter_get_buffer().
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourcecompletioncontext.h"
#include "ctksource-enumtypes.h"
#include "ctksourcecompletionprovider.h"
#include "ctksourcecompletion.h"

struct _CtkSourceCompletionContextPrivate
{
	CtkSourceCompletion *completion;

	CtkTextMark *mark;
	CtkSourceCompletionActivation activation;
};

enum
{
	PROP_0,
	PROP_COMPLETION,
	PROP_ITER,
	PROP_ACTIVATION
};

enum
{
	CANCELLED,
	N_SIGNALS
};

static guint context_signals[N_SIGNALS];

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceCompletionContext, ctk_source_completion_context, G_TYPE_INITIALLY_UNOWNED)

static void
ctk_source_completion_context_dispose (GObject *object)
{
	CtkSourceCompletionContext *context = CTK_SOURCE_COMPLETION_CONTEXT (object);

	if (context->priv->mark != NULL)
	{
		CtkTextBuffer *buffer = ctk_text_mark_get_buffer (context->priv->mark);

		if (buffer != NULL)
		{
			ctk_text_buffer_delete_mark (buffer, context->priv->mark);
		}

		g_object_unref (context->priv->mark);
		context->priv->mark = NULL;
	}

	g_clear_object (&context->priv->completion);

	G_OBJECT_CLASS (ctk_source_completion_context_parent_class)->dispose (object);
}

static void
set_iter (CtkSourceCompletionContext *context,
	  CtkTextIter                *iter)
{
	CtkTextBuffer *buffer;

	buffer = ctk_text_iter_get_buffer (iter);

	if (context->priv->mark != NULL)
	{
		CtkTextBuffer *old_buffer;

		old_buffer = ctk_text_mark_get_buffer (context->priv->mark);

		if (old_buffer != buffer)
		{
			g_object_unref (context->priv->mark);
			context->priv->mark = NULL;
		}
	}

	if (context->priv->mark == NULL)
	{
		context->priv->mark = ctk_text_buffer_create_mark (buffer, NULL, iter, FALSE);
		g_object_ref (context->priv->mark);
	}
	else
	{
		ctk_text_buffer_move_mark (buffer, context->priv->mark, iter);
	}

	g_object_notify (G_OBJECT (context), "iter");
}

static void
ctk_source_completion_context_set_property (GObject      *object,
                                            guint         prop_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
	CtkSourceCompletionContext *context = CTK_SOURCE_COMPLETION_CONTEXT (object);

	switch (prop_id)
	{
		case PROP_COMPLETION:
			context->priv->completion = g_value_dup_object (value);
			break;

		case PROP_ITER:
			set_iter (context, g_value_get_boxed (value));
			break;

		case PROP_ACTIVATION:
			context->priv->activation = g_value_get_flags (value);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
ctk_source_completion_context_get_property (GObject    *object,
                                            guint       prop_id,
                                            GValue     *value,
                                            GParamSpec *pspec)
{
	CtkSourceCompletionContext *context = CTK_SOURCE_COMPLETION_CONTEXT (object);

	switch (prop_id)
	{
		case PROP_COMPLETION:
			g_value_set_object (value, context->priv->completion);
			break;

		case PROP_ITER:
			{
				CtkTextIter iter;

				if (ctk_source_completion_context_get_iter (context, &iter))
				{
					g_value_set_boxed (value, &iter);
				}
			}
			break;

		case PROP_ACTIVATION:
			g_value_set_flags (value, context->priv->activation);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
}

static void
ctk_source_completion_context_class_init (CtkSourceCompletionContextClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = ctk_source_completion_context_set_property;
	object_class->get_property = ctk_source_completion_context_get_property;
	object_class->dispose = ctk_source_completion_context_dispose;

	/**
	 * CtkSourceCompletionContext::cancelled:
	 *
	 * Emitted when the current population of proposals has been cancelled.
	 * Providers adding proposals asynchronously should connect to this signal
	 * to know when to cancel running proposal queries.
	 **/
	context_signals[CANCELLED] =
		g_signal_new ("cancelled",
		              G_TYPE_FROM_CLASS (klass),
		              G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		              G_STRUCT_OFFSET (CtkSourceCompletionContextClass, cancelled),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0);
	g_signal_set_va_marshaller (context_signals[CANCELLED],
	                            G_TYPE_FROM_CLASS (klass),
	                            g_cclosure_marshal_VOID__VOIDv);

	/**
	 * CtkSourceCompletionContext:completion:
	 *
	 * The #CtkSourceCompletion associated with the context.
	 **/
	g_object_class_install_property (object_class,
	                                 PROP_COMPLETION,
	                                 g_param_spec_object ("completion",
	                                                      "Completion",
	                                                      "The completion object to which the context belongs",
	                                                      CTK_SOURCE_TYPE_COMPLETION,
	                                                      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceCompletionContext:iter:
	 *
	 * The #CtkTextIter at which the completion is invoked.
	 **/
	g_object_class_install_property (object_class,
	                                 PROP_ITER,
					 g_param_spec_boxed ("iter",
							     "Iterator",
							     "The CtkTextIter at which the completion was invoked",
							     CTK_TYPE_TEXT_ITER,
							     G_PARAM_READWRITE |
							     G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceCompletionContext:activation:
	 *
	 * The completion activation
	 **/
	g_object_class_install_property (object_class,
	                                 PROP_ACTIVATION,
	                                 g_param_spec_flags ("activation",
	                                                     "Activation",
	                                                     "The type of activation",
	                                                     CTK_SOURCE_TYPE_COMPLETION_ACTIVATION,
	                                                     CTK_SOURCE_COMPLETION_ACTIVATION_USER_REQUESTED,
	                                                     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));
}

static void
ctk_source_completion_context_init (CtkSourceCompletionContext *context)
{
	context->priv = ctk_source_completion_context_get_instance_private (context);
}

/**
 * ctk_source_completion_context_add_proposals:
 * @context: a #CtkSourceCompletionContext.
 * @provider: a #CtkSourceCompletionProvider.
 * @proposals: (nullable) (element-type CtkSource.CompletionProposal): The list of proposals to add.
 * @finished: Whether the provider is finished adding proposals.
 *
 * Providers can use this function to add proposals to the completion. They
 * can do so asynchronously by means of the @finished argument. Providers must
 * ensure that they always call this function with @finished set to %TRUE
 * once each population (even if no proposals need to be added).
 * Population occurs when the ctk_source_completion_provider_populate()
 * function is called.
 **/
void
ctk_source_completion_context_add_proposals (CtkSourceCompletionContext  *context,
                                             CtkSourceCompletionProvider *provider,
                                             GList                       *proposals,
                                             gboolean                     finished)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_CONTEXT (context));
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_PROVIDER (provider));

	_ctk_source_completion_add_proposals (context->priv->completion,
	                                      context,
	                                      provider,
	                                      proposals,
	                                      finished);
}

/**
 * ctk_source_completion_context_get_iter:
 * @context: a #CtkSourceCompletionContext.
 * @iter: (out): a #CtkTextIter.
 *
 * Get the iter at which the completion was invoked. Providers can use this
 * to determine how and if to match proposals.
 *
 * Returns: %TRUE if @iter is correctly set, %FALSE otherwise.
 **/
gboolean
ctk_source_completion_context_get_iter (CtkSourceCompletionContext *context,
                                        CtkTextIter                *iter)
{
	CtkTextBuffer *mark_buffer;
	CtkSourceView *view;
	CtkTextBuffer *completion_buffer;

	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_CONTEXT (context), FALSE);

	if (context->priv->mark == NULL)
	{
		/* This should never happen: context should be always be created
		   providing a position iter */
		g_warning ("Completion context without mark");
		return FALSE;
	}

	mark_buffer = ctk_text_mark_get_buffer (context->priv->mark);

	if (mark_buffer == NULL)
	{
		return FALSE;
	}

	view = ctk_source_completion_get_view (context->priv->completion);
	if (view == NULL)
	{
		return FALSE;
	}

	completion_buffer = ctk_text_view_get_buffer (CTK_TEXT_VIEW (view));

	if (completion_buffer != mark_buffer)
	{
		return FALSE;
	}

	ctk_text_buffer_get_iter_at_mark (mark_buffer, iter, context->priv->mark);
	return TRUE;
}

/**
 * ctk_source_completion_context_get_activation:
 * @context: a #CtkSourceCompletionContext.
 *
 * Get the context activation.
 *
 * Returns: The context activation.
 */
CtkSourceCompletionActivation
ctk_source_completion_context_get_activation (CtkSourceCompletionContext *context)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_CONTEXT (context),
			      CTK_SOURCE_COMPLETION_ACTIVATION_NONE);

	return context->priv->activation;
}

void
_ctk_source_completion_context_cancel (CtkSourceCompletionContext *context)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_CONTEXT (context));

	g_signal_emit (context, context_signals[CANCELLED], 0);
}

CtkSourceCompletionContext *
_ctk_source_completion_context_new (CtkSourceCompletion *completion,
				    CtkTextIter         *position)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION (completion), NULL);
	g_return_val_if_fail (position != NULL, NULL);

	return g_object_new (CTK_SOURCE_TYPE_COMPLETION_CONTEXT,
	                     "completion", completion,
	                     "iter", position,
	                      NULL);
}
