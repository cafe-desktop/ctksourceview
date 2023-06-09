/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2013 - Sébastien Wilmet <swilmet@gnome.org>
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

#include <ctk/ctk.h>
#include <ctksourceview/ctksource.h>
#include "ctksourceview/completion-providers/words/ctksourcecompletionwordslibrary.h"

static void
library_add_words (CtkSourceCompletionWordsLibrary *library)
{
	ctk_source_completion_words_library_add_word (library, "bb");
	ctk_source_completion_words_library_add_word (library, "bbc");
	ctk_source_completion_words_library_add_word (library, "bbd");
	ctk_source_completion_words_library_add_word (library, "dd");
	ctk_source_completion_words_library_add_word (library, "dde");
	ctk_source_completion_words_library_add_word (library, "ddf");
}

static void
test_library_find (void)
{
	CtkSourceCompletionWordsLibrary *library = ctk_source_completion_words_library_new ();
	CtkSourceCompletionWordsProposal *proposal;
	GSequenceIter *iter;
	const gchar *word;

	library_add_words (library);

	iter = ctk_source_completion_words_library_find_first (library, "a", -1);
	g_assert_null (iter);

	iter = ctk_source_completion_words_library_find_first (library, "bba", -1);
	g_assert_null (iter);

	iter = ctk_source_completion_words_library_find_first (library, "b", -1);
	g_assert_nonnull (iter);

	proposal = ctk_source_completion_words_library_get_proposal (iter);
	word = ctk_source_completion_words_proposal_get_word (proposal);
	g_assert_cmpstr (word, ==, "bb");

	iter = ctk_source_completion_words_library_find_first (library, "dd", -1);
	g_assert_nonnull (iter);

	proposal = ctk_source_completion_words_library_get_proposal (iter);
	word = ctk_source_completion_words_proposal_get_word (proposal);
	g_assert_cmpstr (word, ==, "dd");

	g_object_unref (library);
}

int
main (int argc, char **argv)
{
	ctk_test_init (&argc, &argv);

	g_test_add_func ("/CompletionWords/library/find",
			 test_library_find);

	return g_test_run ();
}
