/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2009 - Jesse van den Kieboom
 * Copyright (C) 2013 - Sébastien Wilmet
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

#include "ctksourcecompletionwordslibrary.h"

#include <string.h>

enum
{
	LOCK,
	UNLOCK,
	N_SIGNALS
};

struct _CtkSourceCompletionWordsLibraryPrivate
{
	GSequence *store;
	gboolean locked;
};

static guint signals[N_SIGNALS];

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceCompletionWordsLibrary, ctk_source_completion_words_library, G_TYPE_OBJECT)

static void
ctk_source_completion_words_library_finalize (GObject *object)
{
	CtkSourceCompletionWordsLibrary *library = CTK_SOURCE_COMPLETION_WORDS_LIBRARY (object);

	g_sequence_free (library->priv->store);

	G_OBJECT_CLASS (ctk_source_completion_words_library_parent_class)->finalize (object);
}

static void
ctk_source_completion_words_library_class_init (CtkSourceCompletionWordsLibraryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = ctk_source_completion_words_library_finalize;

	signals[LOCK] =
		g_signal_new ("lock",
		              G_TYPE_FROM_CLASS (klass),
		              G_SIGNAL_RUN_LAST,
		              0,
		              NULL, NULL, NULL,
		              G_TYPE_NONE, 0);

	signals[UNLOCK] =
		g_signal_new ("unlock",
		              G_TYPE_FROM_CLASS (klass),
		              G_SIGNAL_RUN_LAST,
		              0,
		              NULL, NULL, NULL,
		              G_TYPE_NONE, 0);
}

static void
ctk_source_completion_words_library_init (CtkSourceCompletionWordsLibrary *self)
{
	self->priv = ctk_source_completion_words_library_get_instance_private (self);

	self->priv->store = g_sequence_new ((GDestroyNotify)g_object_unref);
}

CtkSourceCompletionWordsLibrary *
ctk_source_completion_words_library_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_COMPLETION_WORDS_LIBRARY, NULL);
}

static gint
compare_full (CtkSourceCompletionWordsProposal *a,
	      CtkSourceCompletionWordsProposal *b,
	      gpointer                          user_data)
{
	if (a == b)
	{
		return 0;
	}

	return strcmp (ctk_source_completion_words_proposal_get_word (a),
	               ctk_source_completion_words_proposal_get_word (b));
}

static gint
compare_prefix (CtkSourceCompletionWordsProposal *a,
		CtkSourceCompletionWordsProposal *b,
		gpointer                          len)
{
	gint prefix_length = GPOINTER_TO_INT (len);

	return strncmp (ctk_source_completion_words_proposal_get_word (a),
			ctk_source_completion_words_proposal_get_word (b),
			prefix_length);
}

static gboolean
iter_match_prefix (GSequenceIter *iter,
                   const gchar   *word,
                   gint           len)
{
	CtkSourceCompletionWordsProposal *item;
	const gchar *proposal_word;

	item = ctk_source_completion_words_library_get_proposal (iter);
	proposal_word = ctk_source_completion_words_proposal_get_word (item);

	if (len == -1)
	{
		len = strlen (word);
	}

	return strncmp (proposal_word, word, len) == 0;
}

CtkSourceCompletionWordsProposal *
ctk_source_completion_words_library_get_proposal (GSequenceIter *iter)
{
	if (iter == NULL)
	{
		return NULL;
	}

	return CTK_SOURCE_COMPLETION_WORDS_PROPOSAL (g_sequence_get (iter));
}

/* Find the first item in the library with the prefix equal to @word.
 * If no such items exist, returns %NULL.
 */
GSequenceIter *
ctk_source_completion_words_library_find_first (CtkSourceCompletionWordsLibrary *library,
                                                const gchar                     *word,
                                                gint                             len)
{
	CtkSourceCompletionWordsProposal *proposal;
	GSequenceIter *iter;

	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_LIBRARY (library), NULL);
	g_return_val_if_fail (word != NULL, NULL);

	if (len == -1)
	{
		len = strlen (word);
	}

	proposal = ctk_source_completion_words_proposal_new (word);

	iter = g_sequence_lookup (library->priv->store,
				  proposal,
				  (GCompareDataFunc)compare_prefix,
				  GINT_TO_POINTER (len));

	g_clear_object (&proposal);

	if (iter == NULL)
	{
		return NULL;
	}

	while (!g_sequence_iter_is_begin (iter))
	{
		GSequenceIter *prev = g_sequence_iter_prev (iter);

		if (!iter_match_prefix (prev, word, len))
		{
			break;
		}

		iter = prev;
	}

	return iter;
}

GSequenceIter *
ctk_source_completion_words_library_find_next (GSequenceIter *iter,
                                               const gchar   *word,
                                               gint           len)
{
	g_return_val_if_fail (iter != NULL, NULL);
	g_return_val_if_fail (word != NULL, NULL);

	iter = g_sequence_iter_next (iter);

	if (!g_sequence_iter_is_end (iter) &&
	    iter_match_prefix (iter, word, len))
	{
		return iter;
	}

	return NULL;
}

GSequenceIter *
ctk_source_completion_words_library_find (CtkSourceCompletionWordsLibrary  *library,
					  CtkSourceCompletionWordsProposal *proposal)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_LIBRARY (library), NULL);
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_PROPOSAL (proposal), NULL);

	return g_sequence_lookup (library->priv->store,
				  proposal,
				  (GCompareDataFunc)compare_full,
				  NULL);
}

static void
on_proposal_unused (CtkSourceCompletionWordsProposal *proposal,
                    CtkSourceCompletionWordsLibrary  *library)
{
	GSequenceIter *iter = ctk_source_completion_words_library_find (library,
	                                                                proposal);

	if (iter != NULL)
	{
		g_sequence_remove (iter);
	}
}

CtkSourceCompletionWordsProposal *
ctk_source_completion_words_library_add_word (CtkSourceCompletionWordsLibrary *library,
                                              const gchar                     *word)
{
	CtkSourceCompletionWordsProposal *proposal;
	GSequenceIter *iter;

	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_LIBRARY (library), NULL);
	g_return_val_if_fail (word != NULL, NULL);

	/* Check if word already exists */
	proposal = ctk_source_completion_words_proposal_new (word);

	iter = ctk_source_completion_words_library_find (library, proposal);

	if (iter != NULL)
	{
		CtkSourceCompletionWordsProposal *iter_proposal;

		iter_proposal = ctk_source_completion_words_library_get_proposal (iter);

		/* Already exists, increase the use count */
		ctk_source_completion_words_proposal_use (iter_proposal);

		g_object_unref (proposal);
		return iter_proposal;
	}

	if (library->priv->locked)
	{
		g_object_unref (proposal);
		return NULL;
	}

	g_signal_connect (proposal,
	                  "unused",
	                  G_CALLBACK (on_proposal_unused),
	                  library);

	g_sequence_insert_sorted (library->priv->store,
	                          proposal,
	                          (GCompareDataFunc)compare_full,
	                          NULL);

	return proposal;
}

void
ctk_source_completion_words_library_remove_word (CtkSourceCompletionWordsLibrary  *library,
                                                 CtkSourceCompletionWordsProposal *proposal)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_LIBRARY (library));
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_PROPOSAL (proposal));

	ctk_source_completion_words_proposal_unuse (proposal);
}

void
ctk_source_completion_words_library_lock (CtkSourceCompletionWordsLibrary *library)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_LIBRARY (library));

	library->priv->locked = TRUE;
	g_signal_emit (library, signals[LOCK], 0);
}

void
ctk_source_completion_words_library_unlock (CtkSourceCompletionWordsLibrary *library)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_LIBRARY (library));

	library->priv->locked = FALSE;
	g_signal_emit (library, signals[UNLOCK], 0);
}

gboolean
ctk_source_completion_words_library_is_locked (CtkSourceCompletionWordsLibrary *library)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_LIBRARY (library), TRUE);

	return library->priv->locked;
}
