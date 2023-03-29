/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourcecompletionwordsproposal.h"

struct _GtkSourceCompletionWordsProposalPrivate
{
	gchar *word;
	gint use_count;
};

enum
{
	UNUSED,
	N_SIGNALS
};

static guint signals[N_SIGNALS];

static void ctk_source_completion_proposal_iface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (GtkSourceCompletionWordsProposal,
			 ctk_source_completion_words_proposal,
			 G_TYPE_OBJECT,
			 G_ADD_PRIVATE (GtkSourceCompletionWordsProposal)
			 G_IMPLEMENT_INTERFACE (CTK_SOURCE_TYPE_COMPLETION_PROPOSAL,
			 			ctk_source_completion_proposal_iface_init))

static gchar *
ctk_source_completion_words_proposal_get_text (GtkSourceCompletionProposal *proposal)
{
	return g_strdup (CTK_SOURCE_COMPLETION_WORDS_PROPOSAL(proposal)->priv->word);
}

static void
ctk_source_completion_proposal_iface_init (gpointer g_iface,
                                           gpointer iface_data)
{
	GtkSourceCompletionProposalIface *iface = (GtkSourceCompletionProposalIface *)g_iface;

	/* Interface data getter implementations */
	iface->get_label = ctk_source_completion_words_proposal_get_text;
	iface->get_text = ctk_source_completion_words_proposal_get_text;
}

static void
ctk_source_completion_words_proposal_finalize (GObject *object)
{
	GtkSourceCompletionWordsProposal *proposal;

	proposal = CTK_SOURCE_COMPLETION_WORDS_PROPOSAL (object);
	g_free (proposal->priv->word);

	G_OBJECT_CLASS (ctk_source_completion_words_proposal_parent_class)->finalize (object);
}

static void
ctk_source_completion_words_proposal_class_init (GtkSourceCompletionWordsProposalClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = ctk_source_completion_words_proposal_finalize;

	signals[UNUSED] =
		g_signal_new ("unused",
		              G_TYPE_FROM_CLASS (klass),
		              G_SIGNAL_RUN_LAST,
		              0,
		              NULL, NULL, NULL,
		              G_TYPE_NONE, 0);
}

static void
ctk_source_completion_words_proposal_init (GtkSourceCompletionWordsProposal *self)
{
	self->priv = ctk_source_completion_words_proposal_get_instance_private (self);
	self->priv->use_count = 1;
}

GtkSourceCompletionWordsProposal *
ctk_source_completion_words_proposal_new (const gchar *word)
{
	GtkSourceCompletionWordsProposal *proposal =
		g_object_new (CTK_SOURCE_TYPE_COMPLETION_WORDS_PROPOSAL, NULL);

	proposal->priv->word = g_strdup (word);
	return proposal;
}

void
ctk_source_completion_words_proposal_use (GtkSourceCompletionWordsProposal *proposal)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_PROPOSAL (proposal));

	g_atomic_int_inc (&proposal->priv->use_count);
}

void
ctk_source_completion_words_proposal_unuse (GtkSourceCompletionWordsProposal *proposal)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_PROPOSAL (proposal));

	if (g_atomic_int_dec_and_test (&proposal->priv->use_count))
	{
		g_signal_emit (proposal, signals[UNUSED], 0);
	}
}

const gchar *
ctk_source_completion_words_proposal_get_word (GtkSourceCompletionWordsProposal *proposal)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_PROPOSAL (proposal), NULL);
	return proposal->priv->word;
}

