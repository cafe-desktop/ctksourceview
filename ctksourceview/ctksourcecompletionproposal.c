/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourcecompletionproposal.h"

/**
 * SECTION:completionproposal
 * @title: CtkSourceCompletionProposal
 * @short_description: Completion proposal interface
 *
 * The proposal interface represents a completion item in the completion window.
 * It provides information on how to display the completion item and what action
 * should be taken when the completion item is activated.
 *
 * The proposal is displayed in the completion window with a label and
 * optionally an icon.
 * The label may be specified using plain text or markup by implementing
 * the corresponding get function. Only one of those get functions
 * should return a value different from %NULL.
 * The icon may be specified as a #GdkPixbuf, as an icon name or as a #GIcon by
 * implementing the corresponding get function. At most one of those get functions
 * should return a value different from %NULL, if they all return %NULL no icon
 * will be used.
 */

enum
{
	CHANGED,
	N_SIGNALS
};

static guint signals[N_SIGNALS];

typedef CtkSourceCompletionProposalIface CtkSourceCompletionProposalInterface;

G_DEFINE_INTERFACE (CtkSourceCompletionProposal, ctk_source_completion_proposal, G_TYPE_OBJECT)

static gchar *
ctk_source_completion_proposal_get_label_default (CtkSourceCompletionProposal *proposal)
{
	return NULL;
}

static gchar *
ctk_source_completion_proposal_get_markup_default (CtkSourceCompletionProposal *proposal)
{
	return NULL;
}

static gchar *
ctk_source_completion_proposal_get_text_default (CtkSourceCompletionProposal *proposal)
{
	return NULL;
}

static GdkPixbuf *
ctk_source_completion_proposal_get_icon_default (CtkSourceCompletionProposal *proposal)
{
	return NULL;
}

static const gchar *
ctk_source_completion_proposal_get_icon_name_default (CtkSourceCompletionProposal *proposal)
{
	return NULL;
}

static GIcon *
ctk_source_completion_proposal_get_gicon_default (CtkSourceCompletionProposal *proposal)
{
	return NULL;
}

static gchar *
ctk_source_completion_proposal_get_info_default (CtkSourceCompletionProposal *proposal)
{
	return NULL;
}

static guint
ctk_source_completion_proposal_hash_default (CtkSourceCompletionProposal *proposal)
{
	return g_direct_hash (proposal);
}

static gboolean
ctk_source_completion_proposal_equal_default (CtkSourceCompletionProposal *proposal,
                                              CtkSourceCompletionProposal *other)
{
	return g_direct_equal (proposal, other);
}

static void
ctk_source_completion_proposal_default_init (CtkSourceCompletionProposalIface *iface)
{
	static gboolean initialized = FALSE;

	iface->get_label = ctk_source_completion_proposal_get_label_default;
	iface->get_markup = ctk_source_completion_proposal_get_markup_default;
	iface->get_text = ctk_source_completion_proposal_get_text_default;
	iface->get_icon = ctk_source_completion_proposal_get_icon_default;
	iface->get_icon_name = ctk_source_completion_proposal_get_icon_name_default;
	iface->get_gicon = ctk_source_completion_proposal_get_gicon_default;
	iface->get_info = ctk_source_completion_proposal_get_info_default;
	iface->hash = ctk_source_completion_proposal_hash_default;
	iface->equal = ctk_source_completion_proposal_equal_default;

	if (!initialized)
	{
		/**
		 * CtkSourceCompletionProposal::changed:
		 * @proposal: The #CtkSourceCompletionProposal
		 *
		 * Emitted when the proposal has changed. The completion popup
		 * will react to this by updating the shown information.
		 *
		 */
		signals[CHANGED] =
			g_signal_new ("changed",
			      G_TYPE_FROM_INTERFACE (iface),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (CtkSourceCompletionProposalIface, changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
		g_signal_set_va_marshaller (signals[CHANGED],
		                            G_TYPE_FROM_INTERFACE (iface),
		                            g_cclosure_marshal_VOID__VOIDv);

		initialized = TRUE;
	}
}

/**
 * ctk_source_completion_proposal_get_label:
 * @proposal: a #CtkSourceCompletionProposal.
 *
 * Gets the label of @proposal. The label is shown in the list of proposals as
 * plain text. If you need any markup (such as bold or italic text), you have
 * to implement ctk_source_completion_proposal_get_markup(). The returned string
 * must be freed with g_free().
 *
 * Returns: a new string containing the label of @proposal.
 */
gchar *
ctk_source_completion_proposal_get_label (CtkSourceCompletionProposal *proposal)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (proposal), NULL);

	return CTK_SOURCE_COMPLETION_PROPOSAL_GET_INTERFACE (proposal)->get_label (proposal);
}

/**
 * ctk_source_completion_proposal_get_markup:
 * @proposal: a #CtkSourceCompletionProposal.
 *
 * Gets the label of @proposal with markup. The label is shown in the list of
 * proposals and may contain markup. This will be used instead of
 * ctk_source_completion_proposal_get_label() if implemented. The returned string
 * must be freed with g_free().
 *
 * Returns: a new string containing the label of @proposal with markup.
 */
gchar *
ctk_source_completion_proposal_get_markup (CtkSourceCompletionProposal *proposal)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (proposal), NULL);

	return CTK_SOURCE_COMPLETION_PROPOSAL_GET_INTERFACE (proposal)->get_markup (proposal);
}

/**
 * ctk_source_completion_proposal_get_text:
 * @proposal: a #CtkSourceCompletionProposal.
 *
 * Gets the text of @proposal. The text that is inserted into
 * the text buffer when the proposal is activated by the default activation.
 * You are free to implement a custom activation handler in the provider and
 * not implement this function. For more information, see
 * ctk_source_completion_provider_activate_proposal(). The returned string must
 * be freed with g_free().
 *
 * Returns: a new string containing the text of @proposal.
 */
gchar *
ctk_source_completion_proposal_get_text (CtkSourceCompletionProposal *proposal)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (proposal), NULL);

	return CTK_SOURCE_COMPLETION_PROPOSAL_GET_INTERFACE (proposal)->get_text (proposal);
}

/**
 * ctk_source_completion_proposal_get_icon:
 * @proposal: a #CtkSourceCompletionProposal.
 *
 * Gets the #GdkPixbuf for the icon of @proposal.
 *
 * Returns: (nullable) (transfer none): A #GdkPixbuf with the icon of @proposal.
 */
GdkPixbuf *
ctk_source_completion_proposal_get_icon (CtkSourceCompletionProposal *proposal)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (proposal), NULL);

	return CTK_SOURCE_COMPLETION_PROPOSAL_GET_INTERFACE (proposal)->get_icon (proposal);
}

/**
 * ctk_source_completion_proposal_get_icon_name:
 * @proposal: a #CtkSourceCompletionProposal.
 *
 * Gets the icon name of @proposal.
 *
 * Returns: (nullable) (transfer none): The icon name of @proposal.
 *
 * Since: 3.18
 */
const gchar *
ctk_source_completion_proposal_get_icon_name (CtkSourceCompletionProposal *proposal)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (proposal), NULL);

	return CTK_SOURCE_COMPLETION_PROPOSAL_GET_INTERFACE (proposal)->get_icon_name (proposal);
}

/**
 * ctk_source_completion_proposal_get_gicon:
 * @proposal: a #CtkSourceCompletionProposal.
 *
 * Gets the #GIcon for the icon of @proposal.
 *
 * Returns: (nullable) (transfer none): A #GIcon with the icon of @proposal.
 *
 * Since: 3.18
 */
GIcon *
ctk_source_completion_proposal_get_gicon (CtkSourceCompletionProposal *proposal)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (proposal), NULL);

	return CTK_SOURCE_COMPLETION_PROPOSAL_GET_INTERFACE (proposal)->get_gicon (proposal);
}

/**
 * ctk_source_completion_proposal_get_info:
 * @proposal: a #CtkSourceCompletionProposal.
 *
 * Gets extra information associated to the proposal. This information will be
 * used to present the user with extra, detailed information about the
 * selected proposal. The returned string must be freed with g_free().
 *
 * Returns: (nullable) (transfer full): a newly-allocated string containing
 * extra information of @proposal or %NULL if no extra information is associated
 * to @proposal.
 */
gchar *
ctk_source_completion_proposal_get_info (CtkSourceCompletionProposal *proposal)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (proposal), NULL);

	return CTK_SOURCE_COMPLETION_PROPOSAL_GET_INTERFACE (proposal)->get_info (proposal);
}

/**
 * ctk_source_completion_proposal_hash:
 * @proposal: a #CtkSourceCompletionProposal.
 *
 * Get the hash value of @proposal. This is used to (together with
 * ctk_source_completion_proposal_equal()) to match proposals in the completion
 * model. By default, it uses a direct hash (g_direct_hash()).
 *
 * Returns: The hash value of @proposal.
 */
guint
ctk_source_completion_proposal_hash (CtkSourceCompletionProposal *proposal)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (proposal), 0);

	return CTK_SOURCE_COMPLETION_PROPOSAL_GET_INTERFACE (proposal)->hash (proposal);
}

/**
 * ctk_source_completion_proposal_equal:
 * @proposal: a #CtkSourceCompletionProposal.
 * @other: a #CtkSourceCompletionProposal.
 *
 * Get whether two proposal objects are the same.  This is used to (together
 * with ctk_source_completion_proposal_hash()) to match proposals in the
 * completion model. By default, it uses direct equality (g_direct_equal()).
 *
 * Returns: %TRUE if @proposal and @object are the same proposal
 */
gboolean
ctk_source_completion_proposal_equal (CtkSourceCompletionProposal *proposal,
                                      CtkSourceCompletionProposal *other)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (proposal), FALSE);
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (other), FALSE);

	return CTK_SOURCE_COMPLETION_PROPOSAL_GET_INTERFACE (proposal)->equal (proposal, other);
}

/**
 * ctk_source_completion_proposal_changed:
 * @proposal: a #CtkSourceCompletionProposal.
 *
 * Emits the "changed" signal on @proposal. This should be called by
 * implementations whenever the name, icon or info of the proposal has
 * changed.
 */
void
ctk_source_completion_proposal_changed (CtkSourceCompletionProposal *proposal)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (proposal));
	g_signal_emit (proposal, signals[CHANGED], 0);
}
