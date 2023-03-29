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

#ifndef CTK_SOURCE_COMPLETION_PROPOSAL_H
#define CTK_SOURCE_COMPLETION_PROPOSAL_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_COMPLETION_PROPOSAL			(ctk_source_completion_proposal_get_type ())
#define CTK_SOURCE_COMPLETION_PROPOSAL(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_PROPOSAL, CtkSourceCompletionProposal))
#define CTK_SOURCE_IS_COMPLETION_PROPOSAL(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_COMPLETION_PROPOSAL))
#define CTK_SOURCE_COMPLETION_PROPOSAL_GET_INTERFACE(obj)	(G_TYPE_INSTANCE_GET_INTERFACE ((obj), CTK_SOURCE_TYPE_COMPLETION_PROPOSAL, CtkSourceCompletionProposalIface))

typedef struct _CtkSourceCompletionProposalIface	CtkSourceCompletionProposalIface;

/**
 * CtkSourceCompletionProposalIface:
 * @parent: The parent interface.
 * @get_label: The virtual function pointer for ctk_source_completion_proposal_get_label().
 * By default, %NULL is returned.
 * @get_markup: The virtual function pointer for ctk_source_completion_proposal_get_markup().
 * By default, %NULL is returned.
 * @get_text: The virtual function pointer for ctk_source_completion_proposal_get_text().
 * By default, %NULL is returned.
 * @get_icon: The virtual function pointer for ctk_source_completion_proposal_get_icon().
 * By default, %NULL is returned.
 * @get_icon_name: The virtual function pointer for ctk_source_completion_proposal_get_icon_name().
 * By default, %NULL is returned.
 * @get_gicon: The virtual function pointer for ctk_source_completion_proposal_get_gicon().
 * By default, %NULL is returned.
 * @get_info: The virtual function pointer for ctk_source_completion_proposal_get_info().
 * By default, %NULL is returned.
 * @hash: The virtual function pointer for ctk_source_completion_proposal_hash().
 * By default, it uses a direct hash (g_direct_hash()).
 * @equal: The virtual function pointer for ctk_source_completion_proposal_equal().
 * By default, it uses direct equality (g_direct_equal()).
 * @changed: The function pointer for the #CtkSourceCompletionProposal::changed signal.
 *
 * The virtual function table for #CtkSourceCompletionProposal.
 */
struct _CtkSourceCompletionProposalIface
{
	GTypeInterface parent;

	/* Interface functions */
	gchar		*(*get_label)		(CtkSourceCompletionProposal *proposal);
	gchar		*(*get_markup)		(CtkSourceCompletionProposal *proposal);
	gchar		*(*get_text)		(CtkSourceCompletionProposal *proposal);

	GdkPixbuf	*(*get_icon)		(CtkSourceCompletionProposal *proposal);
	const gchar	*(*get_icon_name)	(CtkSourceCompletionProposal *proposal);
	GIcon		*(*get_gicon)		(CtkSourceCompletionProposal *proposal);

	gchar		*(*get_info)		(CtkSourceCompletionProposal *proposal);

	guint		 (*hash)		(CtkSourceCompletionProposal *proposal);
	gboolean	 (*equal)		(CtkSourceCompletionProposal *proposal,
						 CtkSourceCompletionProposal *other);

	/* Signals */
	void		 (*changed)		(CtkSourceCompletionProposal *proposal);
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType 			 ctk_source_completion_proposal_get_type 	(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
gchar			*ctk_source_completion_proposal_get_label	(CtkSourceCompletionProposal *proposal);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar			*ctk_source_completion_proposal_get_markup	(CtkSourceCompletionProposal *proposal);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar			*ctk_source_completion_proposal_get_text	(CtkSourceCompletionProposal *proposal);

CTK_SOURCE_AVAILABLE_IN_ALL
GdkPixbuf		*ctk_source_completion_proposal_get_icon	(CtkSourceCompletionProposal *proposal);

CTK_SOURCE_AVAILABLE_IN_3_18
const gchar		*ctk_source_completion_proposal_get_icon_name	(CtkSourceCompletionProposal *proposal);

CTK_SOURCE_AVAILABLE_IN_3_18
GIcon			*ctk_source_completion_proposal_get_gicon	(CtkSourceCompletionProposal *proposal);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar			*ctk_source_completion_proposal_get_info	(CtkSourceCompletionProposal *proposal);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_completion_proposal_changed		(CtkSourceCompletionProposal *proposal);

CTK_SOURCE_AVAILABLE_IN_ALL
guint			 ctk_source_completion_proposal_hash		(CtkSourceCompletionProposal *proposal);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		 ctk_source_completion_proposal_equal		(CtkSourceCompletionProposal *proposal,
									 CtkSourceCompletionProposal *other);

G_END_DECLS

#endif /* CTK_SOURCE_COMPLETION_PROPOSAL_H */
