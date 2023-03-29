/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2009 - Jesse van den Kieboom
 *
 * ctksourceview is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ctksourceview is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CTK_SOURCE_COMPLETION_WORDS_PROPOSAL_H
#define CTK_SOURCE_COMPLETION_WORDS_PROPOSAL_H

#include <glib-object.h>
#include <ctksourceview/ctksourcecompletionproposal.h>

#include "ctksourceview/ctksourcetypes-private.h"

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_COMPLETION_WORDS_PROPOSAL			(ctk_source_completion_words_proposal_get_type ())
#define CTK_SOURCE_COMPLETION_WORDS_PROPOSAL(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_PROPOSAL, CtkSourceCompletionWordsProposal))
#define CTK_SOURCE_COMPLETION_WORDS_PROPOSAL_CONST(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_PROPOSAL, CtkSourceCompletionWordsProposal const))
#define CTK_SOURCE_COMPLETION_WORDS_PROPOSAL_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_COMPLETION_WORDS_PROPOSAL, CtkSourceCompletionWordsProposalClass))
#define CTK_SOURCE_IS_COMPLETION_WORDS_PROPOSAL(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_PROPOSAL))
#define CTK_SOURCE_IS_COMPLETION_WORDS_PROPOSAL_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_COMPLETION_WORDS_PROPOSAL))
#define CTK_SOURCE_COMPLETION_WORDS_PROPOSAL_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_PROPOSAL, CtkSourceCompletionWordsProposalClass))

typedef struct _CtkSourceCompletionWordsProposal		CtkSourceCompletionWordsProposal;
typedef struct _CtkSourceCompletionWordsProposalClass		CtkSourceCompletionWordsProposalClass;
typedef struct _CtkSourceCompletionWordsProposalPrivate		CtkSourceCompletionWordsProposalPrivate;

struct _CtkSourceCompletionWordsProposal {
	GObject parent;

	CtkSourceCompletionWordsProposalPrivate *priv;
};

struct _CtkSourceCompletionWordsProposalClass {
	GObjectClass parent_class;
};

CTK_SOURCE_INTERNAL
GType		 ctk_source_completion_words_proposal_get_type	(void) G_GNUC_CONST;

CTK_SOURCE_INTERNAL
CtkSourceCompletionWordsProposal *
		 ctk_source_completion_words_proposal_new 	(const gchar                      *word);

CTK_SOURCE_INTERNAL
const gchar 	*ctk_source_completion_words_proposal_get_word 	(CtkSourceCompletionWordsProposal *proposal);

CTK_SOURCE_INTERNAL
void		 ctk_source_completion_words_proposal_use 	(CtkSourceCompletionWordsProposal *proposal);

CTK_SOURCE_INTERNAL
void		 ctk_source_completion_words_proposal_unuse 	(CtkSourceCompletionWordsProposal *proposal);

G_END_DECLS

#endif /* CTK_SOURCE_COMPLETION_WORDS_PROPOSAL_H */
