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

#include "ctksourcecompletionprovider.h"
#include "ctksourcecompletionproposal.h"
#include "ctksourcecompletioninfo.h"

/**
 * SECTION:completionprovider
 * @title: CtkSourceCompletionProvider
 * @short_description: Completion provider interface
 *
 * You must implement this interface to provide proposals to #CtkSourceCompletion
 *
 * The provider may be displayed in the completion window as a header row, showing
 * its name and optionally an icon.
 * The icon may be specified as a #GdkPixbuf, as an icon name or as a #GIcon by
 * implementing the corresponding get function. At most one of those get functions
 * should return a value different from %NULL, if they all return %NULL no icon
 * will be used.
 */

typedef CtkSourceCompletionProviderIface CtkSourceCompletionProviderInterface;

G_DEFINE_INTERFACE(CtkSourceCompletionProvider, ctk_source_completion_provider, G_TYPE_OBJECT)

/* Default implementations */
static gchar *
ctk_source_completion_provider_get_name_default (CtkSourceCompletionProvider *provider)
{
	g_return_val_if_reached (NULL);
}

static GdkPixbuf *
ctk_source_completion_provider_get_icon_default (CtkSourceCompletionProvider *provider)
{
	return NULL;
}

static const gchar *
ctk_source_completion_provider_get_icon_name_default (CtkSourceCompletionProvider *provider)
{
	return NULL;
}

static GIcon *
ctk_source_completion_provider_get_gicon_default (CtkSourceCompletionProvider *provider)
{
	return NULL;
}

static void
ctk_source_completion_provider_populate_default (CtkSourceCompletionProvider *provider,
                                                 CtkSourceCompletionContext  *context)
{
	ctk_source_completion_context_add_proposals (context, provider, NULL, TRUE);
}

static CtkSourceCompletionActivation
ctk_source_completion_provider_get_activation_default (CtkSourceCompletionProvider *provider)
{
	return CTK_SOURCE_COMPLETION_ACTIVATION_INTERACTIVE |
	       CTK_SOURCE_COMPLETION_ACTIVATION_USER_REQUESTED;
}

static gboolean
ctk_source_completion_provider_match_default (CtkSourceCompletionProvider *provider,
                                              CtkSourceCompletionContext  *context)
{
	return TRUE;
}

static CtkWidget *
ctk_source_completion_provider_get_info_widget_default (CtkSourceCompletionProvider *provider,
                                                        CtkSourceCompletionProposal *proposal)
{
	return NULL;
}

static void
ctk_source_completion_provider_update_info_default (CtkSourceCompletionProvider *provider,
                                                    CtkSourceCompletionProposal *proposal,
                                                    CtkSourceCompletionInfo     *info)
{
}

static gboolean
ctk_source_completion_provider_get_start_iter_default (CtkSourceCompletionProvider *provider,
                                                       CtkSourceCompletionContext  *context,
                                                       CtkSourceCompletionProposal *proposal,
                                                       CtkTextIter                 *iter)
{
	return FALSE;
}

static gboolean
ctk_source_completion_provider_activate_proposal_default (CtkSourceCompletionProvider *provider,
                                                          CtkSourceCompletionProposal *proposal,
                                                          CtkTextIter                 *iter)
{
	return FALSE;
}

static gint
ctk_source_completion_provider_get_interactive_delay_default (CtkSourceCompletionProvider *provider)
{
	/* -1 means the default value in the completion object */
	return -1;
}

static gint
ctk_source_completion_provider_get_priority_default (CtkSourceCompletionProvider *provider)
{
	return 0;
}

static void
ctk_source_completion_provider_default_init (CtkSourceCompletionProviderIface *iface)
{
	iface->get_name = ctk_source_completion_provider_get_name_default;

	iface->get_icon = ctk_source_completion_provider_get_icon_default;
	iface->get_icon_name = ctk_source_completion_provider_get_icon_name_default;
	iface->get_gicon = ctk_source_completion_provider_get_gicon_default;

	iface->populate = ctk_source_completion_provider_populate_default;

	iface->match = ctk_source_completion_provider_match_default;
	iface->get_activation = ctk_source_completion_provider_get_activation_default;

	iface->get_info_widget = ctk_source_completion_provider_get_info_widget_default;
	iface->update_info = ctk_source_completion_provider_update_info_default;

	iface->get_start_iter = ctk_source_completion_provider_get_start_iter_default;
	iface->activate_proposal = ctk_source_completion_provider_activate_proposal_default;

	iface->get_interactive_delay = ctk_source_completion_provider_get_interactive_delay_default;
	iface->get_priority = ctk_source_completion_provider_get_priority_default;
}

/**
 * ctk_source_completion_provider_get_name:
 * @provider: a #CtkSourceCompletionProvider.
 *
 * Get the name of the provider. This should be a translatable name for
 * display to the user. For example: _("Document word completion provider"). The
 * returned string must be freed with g_free().
 *
 * Returns: a new string containing the name of the provider.
 */
gchar *
ctk_source_completion_provider_get_name (CtkSourceCompletionProvider *provider)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROVIDER (provider), NULL);

	return CTK_SOURCE_COMPLETION_PROVIDER_GET_INTERFACE (provider)->get_name (provider);
}

/**
 * ctk_source_completion_provider_get_icon:
 * @provider: The #CtkSourceCompletionProvider
 *
 * Get the #GdkPixbuf for the icon of the @provider.
 *
 * Returns: (nullable) (transfer none): The icon to be used for the provider,
 *          or %NULL if the provider does not have a special icon.
 */
GdkPixbuf *
ctk_source_completion_provider_get_icon (CtkSourceCompletionProvider *provider)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROVIDER (provider), NULL);

	return CTK_SOURCE_COMPLETION_PROVIDER_GET_INTERFACE (provider)->get_icon (provider);
}

/**
 * ctk_source_completion_provider_get_icon_name:
 * @provider: The #CtkSourceCompletionProvider
 *
 * Gets the icon name of @provider.
 *
 * Returns: (nullable) (transfer none): The icon name to be used for the provider,
 *          or %NULL if the provider does not have a special icon.
 *
 * Since: 3.18
 */
const gchar *
ctk_source_completion_provider_get_icon_name (CtkSourceCompletionProvider *provider)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROVIDER (provider), NULL);

	return CTK_SOURCE_COMPLETION_PROVIDER_GET_INTERFACE (provider)->get_icon_name (provider);
}

/**
 * ctk_source_completion_provider_get_gicon:
 * @provider: The #CtkSourceCompletionProvider
 *
 * Gets the #GIcon for the icon of @provider.
 *
 * Returns: (nullable) (transfer none): The icon to be used for the provider,
 *          or %NULL if the provider does not have a special icon.
 *
 * Since: 3.18
 */
GIcon *
ctk_source_completion_provider_get_gicon (CtkSourceCompletionProvider *provider)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROVIDER (provider), NULL);

	return CTK_SOURCE_COMPLETION_PROVIDER_GET_INTERFACE (provider)->get_gicon (provider);
}

/**
 * ctk_source_completion_provider_populate:
 * @provider: a #CtkSourceCompletionProvider.
 * @context: a #CtkSourceCompletionContext.
 *
 * Populate @context with proposals from @provider added with the
 * ctk_source_completion_context_add_proposals() function.
 */
void
ctk_source_completion_provider_populate (CtkSourceCompletionProvider *provider,
                                         CtkSourceCompletionContext  *context)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_PROVIDER (provider));

	CTK_SOURCE_COMPLETION_PROVIDER_GET_INTERFACE (provider)->populate (provider, context);
}

/**
 * ctk_source_completion_provider_get_activation:
 * @provider: a #CtkSourceCompletionProvider.
 *
 * Get with what kind of activation the provider should be activated.
 *
 * Returns: a combination of #CtkSourceCompletionActivation.
 **/
CtkSourceCompletionActivation
ctk_source_completion_provider_get_activation (CtkSourceCompletionProvider *provider)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROVIDER (provider), CTK_SOURCE_COMPLETION_ACTIVATION_NONE);

	return CTK_SOURCE_COMPLETION_PROVIDER_GET_INTERFACE (provider)->get_activation (provider);
}

/**
 * ctk_source_completion_provider_match:
 * @provider: a #CtkSourceCompletionProvider.
 * @context: a #CtkSourceCompletionContext.
 *
 * Get whether the provider match the context of completion detailed in
 * @context.
 *
 * Returns: %TRUE if @provider matches the completion context, %FALSE otherwise.
 */
gboolean
ctk_source_completion_provider_match (CtkSourceCompletionProvider *provider,
                                      CtkSourceCompletionContext  *context)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROVIDER (provider), TRUE);

	return CTK_SOURCE_COMPLETION_PROVIDER_GET_INTERFACE (provider)->match (provider,
	                                                                       context);
}

/**
 * ctk_source_completion_provider_get_info_widget:
 * @provider: a #CtkSourceCompletionProvider.
 * @proposal: a currently selected #CtkSourceCompletionProposal.
 *
 * Get a customized info widget to show extra information of a proposal.
 * This allows for customized widgets on a proposal basis, although in general
 * providers will have the same custom widget for all their proposals and
 * @proposal can be ignored. The implementation of this function is optional.
 *
 * If this function is not implemented, the default widget is a #CtkLabel. The
 * return value of ctk_source_completion_proposal_get_info() is used as the
 * content of the #CtkLabel.
 *
 * <note>
 *   <para>
 *     If implemented, ctk_source_completion_provider_update_info()
 *     <emphasis>must</emphasis> also be implemented.
 *   </para>
 * </note>
 *
 * Returns: (nullable) (transfer none): a custom #CtkWidget to show extra
 * information about @proposal, or %NULL if the provider does not have a special
 * info widget.
 */
CtkWidget *
ctk_source_completion_provider_get_info_widget (CtkSourceCompletionProvider *provider,
                                                CtkSourceCompletionProposal *proposal)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROVIDER (provider), NULL);
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (proposal), NULL);

	return CTK_SOURCE_COMPLETION_PROVIDER_GET_INTERFACE (provider)->get_info_widget (provider, proposal);
}

/**
 * ctk_source_completion_provider_update_info:
 * @provider: a #CtkSourceCompletionProvider.
 * @proposal: a #CtkSourceCompletionProposal.
 * @info: a #CtkSourceCompletionInfo.
 *
 * Update extra information shown in @info for @proposal.
 *
 * <note>
 *   <para>
 *     This function <emphasis>must</emphasis> be implemented when
 *     ctk_source_completion_provider_get_info_widget() is implemented.
 *   </para>
 * </note>
 */
void
ctk_source_completion_provider_update_info (CtkSourceCompletionProvider *provider,
                                            CtkSourceCompletionProposal *proposal,
                                            CtkSourceCompletionInfo     *info)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_PROVIDER (provider));
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (proposal));
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_INFO (info));

	CTK_SOURCE_COMPLETION_PROVIDER_GET_INTERFACE (provider)->update_info (provider, proposal, info);
}

/**
 * ctk_source_completion_provider_get_start_iter:
 * @provider: a #CtkSourceCompletionProvider.
 * @proposal: a #CtkSourceCompletionProposal.
 * @context: a #CtkSourceCompletionContext.
 * @iter: (out): a #CtkTextIter.
 *
 * Get the #CtkTextIter at which the completion for @proposal starts. When
 * implemented, this information is used to position the completion window
 * accordingly when a proposal is selected in the completion window. The
 * @proposal text inside the completion window is aligned on @iter.
 *
 * If this function is not implemented, the word boundary is taken to position
 * the completion window. See ctk_source_completion_provider_activate_proposal()
 * for an explanation on the word boundaries.
 *
 * When the @proposal is activated, the default handler uses @iter as the start
 * of the word to replace. See
 * ctk_source_completion_provider_activate_proposal() for more information.
 *
 * Returns: %TRUE if @iter was set for @proposal, %FALSE otherwise.
 */
gboolean
ctk_source_completion_provider_get_start_iter (CtkSourceCompletionProvider *provider,
                                               CtkSourceCompletionContext  *context,
                                               CtkSourceCompletionProposal *proposal,
                                               CtkTextIter                 *iter)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROVIDER (provider), FALSE);
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_CONTEXT (context), FALSE);
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (proposal), FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);

	return CTK_SOURCE_COMPLETION_PROVIDER_GET_INTERFACE (provider)->get_start_iter (provider,
	                                                                                context,
	                                                                                proposal,
	                                                                                iter);
}

/**
 * ctk_source_completion_provider_activate_proposal:
 * @provider: a #CtkSourceCompletionProvider.
 * @proposal: a #CtkSourceCompletionProposal.
 * @iter: a #CtkTextIter.
 *
 * Activate @proposal at @iter. When this functions returns %FALSE, the default
 * activation of @proposal will take place which replaces the word at @iter
 * with the text of @proposal (see ctk_source_completion_proposal_get_text()).
 *
 * Here is how the default activation selects the boundaries of the word to
 * replace. The end of the word is @iter. For the start of the word, it depends
 * on whether a start iter is defined for @proposal (see
 * ctk_source_completion_provider_get_start_iter()). If a start iter is defined,
 * the start of the word is the start iter. Else, the word (as long as possible)
 * will contain only alphanumerical and the "_" characters.
 *
 * Returns: %TRUE to indicate that the proposal activation has been handled,
 *          %FALSE otherwise.
 */
gboolean
ctk_source_completion_provider_activate_proposal (CtkSourceCompletionProvider *provider,
                                                  CtkSourceCompletionProposal *proposal,
                                                  CtkTextIter                 *iter)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROVIDER (provider), FALSE);
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROPOSAL (proposal), FALSE);

	return CTK_SOURCE_COMPLETION_PROVIDER_GET_INTERFACE (provider)->activate_proposal (provider,
	                                                                                   proposal,
	                                                                                   iter);
}

/**
 * ctk_source_completion_provider_get_interactive_delay:
 * @provider: a #CtkSourceCompletionProvider.
 *
 * Get the delay in milliseconds before starting interactive completion for
 * this provider. A value of -1 indicates to use the default value as set
 * by the #CtkSourceCompletion:auto-complete-delay property.
 *
 * Returns: the interactive delay in milliseconds.
 **/
gint
ctk_source_completion_provider_get_interactive_delay (CtkSourceCompletionProvider *provider)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROVIDER (provider), -1);

	return CTK_SOURCE_COMPLETION_PROVIDER_GET_INTERFACE (provider)->get_interactive_delay (provider);
}

/**
 * ctk_source_completion_provider_get_priority:
 * @provider: a #CtkSourceCompletionProvider.
 *
 * Get the provider priority. The priority determines the order in which
 * proposals appear in the completion popup. Higher priorities are sorted
 * before lower priorities. The default priority is 0.
 *
 * Returns: the provider priority.
 **/
gint
ctk_source_completion_provider_get_priority (CtkSourceCompletionProvider *provider)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_PROVIDER (provider), 0);

	return CTK_SOURCE_COMPLETION_PROVIDER_GET_INTERFACE (provider)->get_priority (provider);
}
