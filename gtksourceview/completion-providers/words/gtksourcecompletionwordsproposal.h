/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of GtkSourceView
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
#define CTK_SOURCE_COMPLETION_WORDS_PROPOSAL(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_PROPOSAL, GtkSourceCompletionWordsProposal))
#define CTK_SOURCE_COMPLETION_WORDS_PROPOSAL_CONST(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_PROPOSAL, GtkSourceCompletionWordsProposal const))
#define CTK_SOURCE_COMPLETION_WORDS_PROPOSAL_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_COMPLETION_WORDS_PROPOSAL, GtkSourceCompletionWordsProposalClass))
#define CTK_SOURCE_IS_COMPLETION_WORDS_PROPOSAL(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_PROPOSAL))
#define CTK_SOURCE_IS_COMPLETION_WORDS_PROPOSAL_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_COMPLETION_WORDS_PROPOSAL))
#define CTK_SOURCE_COMPLETION_WORDS_PROPOSAL_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_PROPOSAL, GtkSourceCompletionWordsProposalClass))

typedef struct _GtkSourceCompletionWordsProposal		GtkSourceCompletionWordsProposal;
typedef struct _GtkSourceCompletionWordsProposalClass		GtkSourceCompletionWordsProposalClass;
typedef struct _GtkSourceCompletionWordsProposalPrivate		GtkSourceCompletionWordsProposalPrivate;

struct _GtkSourceCompletionWordsProposal {
	GObject parent;

	GtkSourceCompletionWordsProposalPrivate *priv;
};

struct _GtkSourceCompletionWordsProposalClass {
	GObjectClass parent_class;
};

CTK_SOURCE_INTERNAL
GType		 ctk_source_completion_words_proposal_get_type	(void) G_GNUC_CONST;

CTK_SOURCE_INTERNAL
GtkSourceCompletionWordsProposal *
		 ctk_source_completion_words_proposal_new 	(const gchar                      *word);

CTK_SOURCE_INTERNAL
const gchar 	*ctk_source_completion_words_proposal_get_word 	(GtkSourceCompletionWordsProposal *proposal);

CTK_SOURCE_INTERNAL
void		 ctk_source_completion_words_proposal_use 	(GtkSourceCompletionWordsProposal *proposal);

CTK_SOURCE_INTERNAL
void		 ctk_source_completion_words_proposal_unuse 	(GtkSourceCompletionWordsProposal *proposal);

G_END_DECLS

#endif /* CTK_SOURCE_COMPLETION_WORDS_PROPOSAL_H */
