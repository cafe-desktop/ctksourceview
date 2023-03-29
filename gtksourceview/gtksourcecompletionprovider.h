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

#ifndef CTK_SOURCE_COMPLETION_PROVIDER_H
#define CTK_SOURCE_COMPLETION_PROVIDER_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcecompletioncontext.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_COMPLETION_PROVIDER 			(ctk_source_completion_provider_get_type ())
#define CTK_SOURCE_COMPLETION_PROVIDER(obj) 			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_PROVIDER, CtkSourceCompletionProvider))
#define CTK_SOURCE_IS_COMPLETION_PROVIDER(obj) 			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_COMPLETION_PROVIDER))
#define CTK_SOURCE_COMPLETION_PROVIDER_GET_INTERFACE(obj) 	(G_TYPE_INSTANCE_GET_INTERFACE ((obj), CTK_SOURCE_TYPE_COMPLETION_PROVIDER, CtkSourceCompletionProviderIface))

typedef struct _CtkSourceCompletionProviderIface CtkSourceCompletionProviderIface;

/**
 * CtkSourceCompletionProviderIface:
 * @g_iface: The parent interface.
 * @get_name: The virtual function pointer for ctk_source_completion_provider_get_name().
 * Must be implemented.
 * @get_icon: The virtual function pointer for ctk_source_completion_provider_get_icon().
 * By default, %NULL is returned.
 * @get_icon_name: The virtual function pointer for ctk_source_completion_provider_get_icon_name().
 * By default, %NULL is returned.
 * @get_gicon: The virtual function pointer for ctk_source_completion_provider_get_gicon().
 * By default, %NULL is returned.
 * @populate: The virtual function pointer for ctk_source_completion_provider_populate().
 * Add no proposals by default.
 * @match: The virtual function pointer for ctk_source_completion_provider_match().
 * By default, %TRUE is returned.
 * @get_activation: The virtual function pointer for ctk_source_completion_provider_get_activation().
 * The combination of all #CtkSourceCompletionActivation is returned by default.
 * @get_info_widget: The virtual function pointer for ctk_source_completion_provider_get_info_widget().
 * By default, %NULL is returned.
 * @update_info: The virtual function pointer for ctk_source_completion_provider_update_info().
 * Does nothing by default.
 * @get_start_iter: The virtual function pointer for ctk_source_completion_provider_get_start_iter().
 * By default, %FALSE is returned.
 * @activate_proposal: The virtual function pointer for ctk_source_completion_provider_activate_proposal().
 * By default, %FALSE is returned.
 * @get_interactive_delay: The virtual function pointer for ctk_source_completion_provider_get_interactive_delay().
 * By default, -1 is returned.
 * @get_priority: The virtual function pointer for ctk_source_completion_provider_get_priority().
 * By default, 0 is returned.
 *
 * The virtual function table for #CtkSourceCompletionProvider.
 */
struct _CtkSourceCompletionProviderIface
{
	GTypeInterface g_iface;

	gchar		*(*get_name)       	(CtkSourceCompletionProvider *provider);

	CdkPixbuf	*(*get_icon)       	(CtkSourceCompletionProvider *provider);
	const gchar	*(*get_icon_name)   (CtkSourceCompletionProvider *provider);
	GIcon		*(*get_gicon)       (CtkSourceCompletionProvider *provider);

	void 		 (*populate) 		(CtkSourceCompletionProvider *provider,
						 CtkSourceCompletionContext  *context);

	gboolean 	 (*match)		(CtkSourceCompletionProvider *provider,
	                                         CtkSourceCompletionContext  *context);

	CtkSourceCompletionActivation
		         (*get_activation)	(CtkSourceCompletionProvider *provider);

	CtkWidget 	*(*get_info_widget)	(CtkSourceCompletionProvider *provider,
						 CtkSourceCompletionProposal *proposal);
	void		 (*update_info)		(CtkSourceCompletionProvider *provider,
						 CtkSourceCompletionProposal *proposal,
						 CtkSourceCompletionInfo     *info);

	gboolean	 (*get_start_iter)	(CtkSourceCompletionProvider *provider,
						 CtkSourceCompletionContext  *context,
						 CtkSourceCompletionProposal *proposal,
						 CtkTextIter                 *iter);
	gboolean	 (*activate_proposal)	(CtkSourceCompletionProvider *provider,
						 CtkSourceCompletionProposal *proposal,
						 CtkTextIter                 *iter);

	gint		 (*get_interactive_delay) (CtkSourceCompletionProvider *provider);
	gint		 (*get_priority)	(CtkSourceCompletionProvider *provider);
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType		 ctk_source_completion_provider_get_type	(void);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar		*ctk_source_completion_provider_get_name	(CtkSourceCompletionProvider *provider);

CTK_SOURCE_AVAILABLE_IN_ALL
CdkPixbuf	*ctk_source_completion_provider_get_icon	(CtkSourceCompletionProvider *provider);

CTK_SOURCE_AVAILABLE_IN_3_18
const gchar	*ctk_source_completion_provider_get_icon_name	(CtkSourceCompletionProvider *provider);

CTK_SOURCE_AVAILABLE_IN_3_18
GIcon		*ctk_source_completion_provider_get_gicon	(CtkSourceCompletionProvider *provider);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_completion_provider_populate	(CtkSourceCompletionProvider *provider,
								 CtkSourceCompletionContext  *context);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceCompletionActivation
		 ctk_source_completion_provider_get_activation (CtkSourceCompletionProvider *provider);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	 ctk_source_completion_provider_match 		(CtkSourceCompletionProvider *provider,
		                                                 CtkSourceCompletionContext  *context);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkWidget	*ctk_source_completion_provider_get_info_widget	(CtkSourceCompletionProvider *provider,
								 CtkSourceCompletionProposal *proposal);

CTK_SOURCE_AVAILABLE_IN_ALL
void 		 ctk_source_completion_provider_update_info	(CtkSourceCompletionProvider *provider,
								 CtkSourceCompletionProposal *proposal,
								 CtkSourceCompletionInfo     *info);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	 ctk_source_completion_provider_get_start_iter	(CtkSourceCompletionProvider *provider,
								 CtkSourceCompletionContext  *context,
								 CtkSourceCompletionProposal *proposal,
								 CtkTextIter                 *iter);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	 ctk_source_completion_provider_activate_proposal (CtkSourceCompletionProvider *provider,
								   CtkSourceCompletionProposal *proposal,
								   CtkTextIter                 *iter);

CTK_SOURCE_AVAILABLE_IN_ALL
gint		 ctk_source_completion_provider_get_interactive_delay (CtkSourceCompletionProvider *provider);

CTK_SOURCE_AVAILABLE_IN_ALL
gint		 ctk_source_completion_provider_get_priority	(CtkSourceCompletionProvider *provider);

G_END_DECLS

#endif /* CTK_SOURCE_COMPLETION_PROVIDER_H */
