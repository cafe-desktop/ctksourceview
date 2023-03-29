/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2010 - Krzesimir Nowak
 * Copyright (C) 2012-2015 - Sébastien Wilmet
 * Copyright (C) 2013, 2015 - Paolo Borelli
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

#include <stdlib.h>
#include <ctksourceview/ctksource.h>
#include "ctksourceview/ctksourcebuffer-private.h"

static const char *c_snippet =
	"#include <foo.h>\n"
	"\n"
	"/* this is a comment */\n"
	"int main() {\n"
	"}\n";

static void
flush_queue (void)
{
	while (ctk_events_pending ())
	{
		ctk_main_iteration ();
	}
}

/* If we are running from the source dir (e.g. during make check)
 * we override the path to read from the data dir
 */
static void
init_default_manager (void)
{
	gchar *dir;

	dir = g_build_filename (TOP_SRCDIR, "data", "language-specs", NULL);

	if (g_file_test (dir, G_FILE_TEST_IS_DIR))
	{
		CtkSourceLanguageManager *lm = ctk_source_language_manager_get_default ();
		gchar *lang_dirs[2] = {dir, NULL};

		ctk_source_language_manager_set_search_path (lm, lang_dirs);
	}

	g_free (dir);
}

static void
test_get_buffer (void)
{
	CtkWidget *view;
	CtkSourceBuffer *buffer;

	view = ctk_source_view_new ();

	buffer = CTK_SOURCE_BUFFER (ctk_text_view_get_buffer (CTK_TEXT_VIEW (view)));

	g_assert_nonnull (buffer);
	g_assert_true (CTK_SOURCE_IS_BUFFER (buffer));

	if (g_object_is_floating (view))
	{
		g_object_ref_sink (view);
	}

	/* Here we check if notify_buffer recreates the buffer while view is being
	 * destroyed, which causes assertion failure in CtkTextView's finalize ()
	 * function.
	 * Please see: https://bugzilla.gnome.org/show_bug.cgi?id=634510 */
	g_object_unref (view);
}

static void
test_get_context_classes (void)
{
	CtkSourceLanguageManager *lm;
	CtkSourceBuffer *buffer;
	CtkSourceLanguage *lang;
	CtkTextIter start, end, i;
	char **classes;

	/* test plain text */
	buffer = ctk_source_buffer_new (NULL);
	ctk_text_buffer_set_text (CTK_TEXT_BUFFER (buffer), "some text", -1);
	ctk_text_buffer_get_bounds (CTK_TEXT_BUFFER (buffer), &start, &end);
	ctk_source_buffer_ensure_highlight (buffer, &start, &end);

	ctk_text_buffer_get_start_iter (CTK_TEXT_BUFFER (buffer), &i);
	classes = ctk_source_buffer_get_context_classes_at_iter (buffer, &i);
	g_assert_cmpuint (g_strv_length (classes), ==, 0);
	g_strfreev (classes);

	g_object_unref (buffer);

	/* test C */
	lm = ctk_source_language_manager_get_default ();
	lang = ctk_source_language_manager_get_language (lm, "c");
	g_assert_true (CTK_SOURCE_IS_LANGUAGE (lang));
	buffer = ctk_source_buffer_new_with_language (lang);
	ctk_text_buffer_set_text (CTK_TEXT_BUFFER (buffer), c_snippet, -1);
	ctk_text_buffer_get_bounds (CTK_TEXT_BUFFER (buffer), &start, &end);
	ctk_source_buffer_ensure_highlight (buffer, &start, &end);

	ctk_text_buffer_get_start_iter (CTK_TEXT_BUFFER (buffer), &i);
	classes = ctk_source_buffer_get_context_classes_at_iter (buffer, &i);
	g_assert_cmpuint (g_strv_length (classes), ==, 1);
	g_assert_cmpstr (classes[0], ==, "no-spell-check");
	g_strfreev (classes);

	ctk_text_buffer_get_iter_at_line_offset (CTK_TEXT_BUFFER (buffer), &i, 2, 5);
	classes = ctk_source_buffer_get_context_classes_at_iter (buffer, &i);
	g_assert_cmpuint (g_strv_length (classes), ==, 1);
	g_assert_cmpstr (classes[0], ==, "comment");
	g_strfreev (classes);

	g_object_unref (buffer);
}

static void
do_test_change_case (CtkSourceBuffer         *buffer,
		     CtkSourceChangeCaseType  case_type,
		     const gchar             *text,
		     const gchar             *expected)
{
	CtkTextIter start;
	CtkTextIter end;
	gchar *changed;
	gchar *changed_normalized;
	gchar *expected_normalized;

	ctk_text_buffer_set_text (CTK_TEXT_BUFFER (buffer), text, -1);

	ctk_text_buffer_get_bounds (CTK_TEXT_BUFFER (buffer), &start, &end);
	ctk_source_buffer_change_case (buffer, case_type, &start, &end);

	ctk_text_buffer_get_bounds (CTK_TEXT_BUFFER (buffer), &start, &end);
	changed = ctk_text_buffer_get_text (CTK_TEXT_BUFFER (buffer), &start, &end, TRUE);

	changed_normalized = g_utf8_normalize (changed, -1, G_NORMALIZE_DEFAULT);
	expected_normalized = g_utf8_normalize (expected, -1, G_NORMALIZE_DEFAULT);

	g_assert_cmpstr (changed_normalized, ==, expected_normalized);

	g_free (changed);
	g_free (changed_normalized);
	g_free (expected_normalized);
}

static void
test_change_case (void)
{
	CtkSourceBuffer *buffer;

	buffer = ctk_source_buffer_new (NULL);

	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_LOWER, "some TEXT", "some text");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_UPPER, "some TEXT", "SOME TEXT");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_TOGGLE, "some TEXT", "SOME text");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_TITLE, "some TEXT", "Some Text");

	/* https://bugzilla.gnome.org/show_bug.cgi?id=416390 */
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_LOWER, "T̈OME", "ẗome");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_UPPER, "ẗome", "T̈OME");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_TOGGLE, "ẗome", "T̈OME");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_TOGGLE, "T̈OME", "ẗome");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_TITLE, "ẗome", "T̈ome");

	/* test g_unichar_totitle */
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_LOWER, "\307\261adzíki", "\307\263adzíki");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_LOWER, "\307\262adzíki", "\307\263adzíki");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_LOWER, "\307\263adzíki", "\307\263adzíki");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_UPPER, "\307\263adzíki", "\307\261ADZÍKI");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_UPPER, "\307\262adzíki", "\307\261ADZÍKI");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_TOGGLE, "\307\263adzíki", "\307\261ADZÍKI");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_TITLE, "\307\263adzíki", "\307\262adzíki");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_TITLE, "\307\261ADZÍKI", "\307\262adzíki");
	do_test_change_case (buffer, CTK_SOURCE_CHANGE_CASE_TITLE, "\307\262ADZÍKI", "\307\262adzíki");

	g_object_unref (buffer);
}

static void
do_test_join_lines (CtkSourceBuffer *buffer,
		    const gchar     *text,
		    const gchar     *expected,
		    gint             start_offset,
		    gint             end_offset)
{
	CtkTextIter start;
	CtkTextIter end;
	gchar *changed;

	ctk_text_buffer_set_text (CTK_TEXT_BUFFER (buffer), text, -1);

	ctk_text_buffer_get_iter_at_offset (CTK_TEXT_BUFFER (buffer), &start, start_offset);
	ctk_text_buffer_get_iter_at_offset (CTK_TEXT_BUFFER (buffer), &end, end_offset);

	ctk_source_buffer_join_lines (buffer, &start, &end);

	ctk_text_buffer_get_bounds (CTK_TEXT_BUFFER (buffer), &start, &end);
	changed = ctk_text_buffer_get_text (CTK_TEXT_BUFFER (buffer), &start, &end, TRUE);

	g_assert_cmpstr (changed, ==, expected);

	g_free (changed);
}

static void
test_join_lines (void)
{
	CtkSourceBuffer *buffer;

	buffer = ctk_source_buffer_new (NULL);

	do_test_join_lines (buffer, "some text", "some text", 0, -1);
	do_test_join_lines (buffer, "some\ntext", "some text", 0, -1);
	do_test_join_lines (buffer, "some\t  \n\t  text", "some text", 0, -1);
	do_test_join_lines (buffer, "some\n\n\ntext", "some text", 0, -1);
	do_test_join_lines (buffer, "some\ntext", "some\ntext", 0, 1);
	do_test_join_lines (buffer, "some\ntext", "some\ntext", 6, -1);
	do_test_join_lines (buffer, "some\ntext\nmore", "some text\nmore", 0, 6);
	do_test_join_lines (buffer, "some\ntext\nmore", "some\ntext more", 6, -1);
	do_test_join_lines (buffer, "some\n   text\nmore", "some text\nmore", 0, 5);
	do_test_join_lines (buffer, "some\ntext\n\n\nmore", "some text\n\nmore", 0, 10);

	g_object_unref (buffer);
}

static void
do_test_sort_lines (CtkSourceBuffer    *buffer,
		    const gchar        *text,
		    const gchar        *expected,
		    gint                start_offset,
		    gint                end_offset,
		    CtkSourceSortFlags  flags,
		    gint                column)
{
	CtkTextIter start;
	CtkTextIter end;
	gchar *changed;

	ctk_text_buffer_set_text (CTK_TEXT_BUFFER (buffer), text, -1);

	ctk_text_buffer_get_iter_at_offset (CTK_TEXT_BUFFER (buffer), &start, start_offset);
	ctk_text_buffer_get_iter_at_offset (CTK_TEXT_BUFFER (buffer), &end, end_offset);

	ctk_source_buffer_sort_lines (buffer, &start, &end, flags, column);

	ctk_text_buffer_get_bounds (CTK_TEXT_BUFFER (buffer), &start, &end);
	changed = ctk_text_buffer_get_text (CTK_TEXT_BUFFER (buffer), &start, &end, TRUE);

	g_assert_cmpstr (changed, ==, expected);

	g_free (changed);
}

static void
test_sort_lines (void)
{
	CtkSourceBuffer *buffer;

	buffer = ctk_source_buffer_new (NULL);

	do_test_sort_lines (buffer, "aaa\nbbb\n", "aaa\nbbb\n", 0, -1, 0, 0);
	do_test_sort_lines (buffer, "bbb\naaa\n", "aaa\nbbb\n", 0, -1, 0, 0);
	do_test_sort_lines (buffer, "bbb\naaa\n", "aaa\nbbb\n", 1, -1, 0, 0);
	do_test_sort_lines (buffer, "bbb\naaa\n", "aaa\nbbb\n", 0, 5, 0, 0);
	do_test_sort_lines (buffer, "ccc\nbbb\naaa\n", "bbb\nccc\naaa\n", 0, 7, 0, 0);
	do_test_sort_lines (buffer, "ccc\nbbb\naaa\n", "bbb\nccc\naaa\n", 0, 8, 0, 0);
	do_test_sort_lines (buffer, "ccc\nbbb\naaa\n", "aaa\nbbb\nccc\n", 0, 9, 0, 0);
	do_test_sort_lines (buffer, "aaa\nbbb\n", "bbb\naaa\n", 0, -1, CTK_SOURCE_SORT_FLAGS_REVERSE_ORDER, 0);
	do_test_sort_lines (buffer, "aaa\nbbb\naaa\n", "aaa\nbbb\n", 0, -1, CTK_SOURCE_SORT_FLAGS_REMOVE_DUPLICATES, 0);
	do_test_sort_lines (buffer, "bbb\naaa\nCCC\n", "CCC\naaa\nbbb\n", 0, -1, CTK_SOURCE_SORT_FLAGS_CASE_SENSITIVE, 0);
	do_test_sort_lines (buffer, "aaabbb\nbbbaaa\n", "bbbaaa\naaabbb\n", 0, -1, 0, 3);
	do_test_sort_lines (buffer, "abcdefghijk\n", "abcdefghijk\n", 2, 6, 0, 0);

	g_object_unref (buffer);
}

static void
do_test_move_words (CtkSourceView      *view,
                    CtkSourceBuffer    *buffer,
                    const gchar        *text,
                    const gchar        *expected,
                    gint                start_offset,
                    gint                end_offset,
                    gint                step)
{
	CtkTextIter start;
	CtkTextIter end;
	gchar *changed;

	ctk_text_buffer_set_text (CTK_TEXT_BUFFER (buffer), text, -1);

	ctk_text_buffer_get_iter_at_offset (CTK_TEXT_BUFFER (buffer), &start, start_offset);
	ctk_text_buffer_get_iter_at_offset (CTK_TEXT_BUFFER (buffer), &end, end_offset);
	ctk_text_buffer_select_range (CTK_TEXT_BUFFER (buffer), &start, &end);

	g_signal_emit_by_name (view, "move-words", step);

	ctk_text_buffer_get_bounds (CTK_TEXT_BUFFER (buffer), &start, &end);
	changed = ctk_text_buffer_get_text (CTK_TEXT_BUFFER (buffer), &start, &end, TRUE);

	g_assert_cmpstr (changed, ==, expected);

	g_free (changed);
}

static void
test_move_words (void)
{
	CtkSourceView *view;
	CtkSourceBuffer *buffer;

	buffer = ctk_source_buffer_new (NULL);
	view = g_object_ref_sink (CTK_SOURCE_VIEW (ctk_source_view_new ()));

	ctk_text_view_set_buffer (CTK_TEXT_VIEW (view), CTK_TEXT_BUFFER (buffer));

	do_test_move_words (view, buffer, "a > b", "a b >", 2, 3, 1);
	do_test_move_words (view, buffer, "a>b", "ab>", 1, 2, 1);
	do_test_move_words (view, buffer, "a>b", ">ab", 1, 2, -1);
	do_test_move_words (view, buffer, "what is this word.", "what word this is.", 13, 17, -2);
	do_test_move_words (view, buffer, "what word this is.", "what is this word.", 5, 9, 2);

	g_object_unref (buffer);
	g_object_unref (view);
}

static void
do_test_bracket_matching (CtkSourceBuffer           *source_buffer,
			  const gchar               *text,
			  gint                       offset,
			  gint                       expected_offset_bracket,
			  gint                       expected_offset_match,
			  CtkSourceBracketMatchType  expected_result)
{
	CtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	CtkTextIter iter;
	CtkTextIter bracket;
	CtkTextIter bracket_match;
	CtkSourceBracketMatchType result;

	ctk_text_buffer_set_text (text_buffer, text, -1);

	/* Ensure that the syntax highlighting engine has finished, and that
	 * context classes are correctly defined.
	 */
	flush_queue ();

	ctk_text_buffer_get_iter_at_offset (text_buffer, &iter, offset);

	result = _ctk_source_buffer_find_bracket_match (source_buffer,
							&iter,
							&bracket,
							&bracket_match);
	g_assert_cmpint (result, ==, expected_result);

	if (result == CTK_SOURCE_BRACKET_MATCH_FOUND)
	{
		gint offset_bracket;
		gint offset_match;

		offset_bracket = ctk_text_iter_get_offset (&bracket);
		offset_match = ctk_text_iter_get_offset (&bracket_match);

		g_assert_cmpint (offset_bracket, ==, expected_offset_bracket);
		g_assert_cmpint (offset_match, ==, expected_offset_match);
	}
}

static void
test_bracket_matching (void)
{
	CtkSourceBuffer *buffer;
	CtkSourceLanguageManager *language_manager;
	CtkSourceLanguage *c_language;
	CtkTextTagTable *table;

	buffer = ctk_source_buffer_new (NULL);

	language_manager = ctk_source_language_manager_get_default ();
	c_language = ctk_source_language_manager_get_language (language_manager, "c");
	g_assert_nonnull (c_language);
	ctk_source_buffer_set_language (buffer, c_language);

	/* Basics */

	do_test_bracket_matching (buffer, "(ab)", 0, 0, 3, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "(ab)", 1, 0, 3, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "(ab)", 2, -1, -1, CTK_SOURCE_BRACKET_MATCH_NONE);
	do_test_bracket_matching (buffer, "(ab)", 3, 3, 0, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "(ab)", 4, 3, 0, CTK_SOURCE_BRACKET_MATCH_FOUND);

	do_test_bracket_matching (buffer, "(ab))", 0, 0, 3, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "(ab))", 1, 0, 3, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "(ab))", 2, -1, -1, CTK_SOURCE_BRACKET_MATCH_NONE);
	do_test_bracket_matching (buffer, "(ab))", 3, 3, 0, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "(ab))", 4, 3, 0, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "(ab))", 5, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);

	do_test_bracket_matching (buffer, "((ab)", 0, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);
	do_test_bracket_matching (buffer, "((ab)", 1, 1, 4, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "((ab)", 2, 1, 4, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "((ab)", 3, -1, -1, CTK_SOURCE_BRACKET_MATCH_NONE);
	do_test_bracket_matching (buffer, "((ab)", 4, 4, 1, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "((ab)", 5, 4, 1, CTK_SOURCE_BRACKET_MATCH_FOUND);

	/* String context class */

	do_test_bracket_matching (buffer, "(\"(ab))\")", 0, 0, 8, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "(\"(ab))\")", 1, 0, 8, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "(\"(ab))\")", 2, 2, 5, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "(\"(ab))\")", 3, 2, 5, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "(\"(ab))\")", 4, -1, -1, CTK_SOURCE_BRACKET_MATCH_NONE);
	do_test_bracket_matching (buffer, "(\"(ab))\")", 5, 5, 2, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "(\"(ab))\")", 6, 5, 2, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "(\"(ab))\")", 7, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);
	do_test_bracket_matching (buffer, "(\"(ab))\")", 8, 8, 0, CTK_SOURCE_BRACKET_MATCH_FOUND);
	do_test_bracket_matching (buffer, "(\"(ab))\")", 9, 8, 0, CTK_SOURCE_BRACKET_MATCH_FOUND);

	do_test_bracket_matching (buffer, "((\"(ab))\")", 0, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);

	do_test_bracket_matching (buffer, "\"(\"a\")\"", 0, -1, -1, CTK_SOURCE_BRACKET_MATCH_NONE);
	do_test_bracket_matching (buffer, "\"(\"a\")\"", 1, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);
	do_test_bracket_matching (buffer, "\"(\"a\")\"", 2, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);
	do_test_bracket_matching (buffer, "\"(\"a\")\"", 3, -1, -1, CTK_SOURCE_BRACKET_MATCH_NONE);
	do_test_bracket_matching (buffer, "\"(\"a\")\"", 4, -1, -1, CTK_SOURCE_BRACKET_MATCH_NONE);
	do_test_bracket_matching (buffer, "\"(\"a\")\"", 5, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);
	do_test_bracket_matching (buffer, "\"(\"a\")\"", 6, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);
	do_test_bracket_matching (buffer, "\"(\"a\")\"", 7, -1, -1, CTK_SOURCE_BRACKET_MATCH_NONE);

	/* Comment context class */

	do_test_bracket_matching (buffer, "/*(*/ /*)*/", 2, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);
	do_test_bracket_matching (buffer, "/*(*/ /*)*/", 8, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);

	/* Direct changes: string -> comment -> string */
	do_test_bracket_matching (buffer, "\"(\"/*a*/\")\"", 1, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);
	do_test_bracket_matching (buffer, "\"(\"/*a*/\")\"", 9, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);

	/* Direct changes: comment -> string -> comment */
	do_test_bracket_matching (buffer, "/*(*/\"a\"/*)*/", 2, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);
	do_test_bracket_matching (buffer, "/*(*/\"a\"/*)*/", 10, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);

	/* Single char in C */
	do_test_bracket_matching (buffer, "'(' ')'", 1, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);
	do_test_bracket_matching (buffer, "'(' ')'", 5, -1, -1, CTK_SOURCE_BRACKET_MATCH_NOT_FOUND);

	g_object_unref (buffer);

	/* Test setting the property and a specific tag table. There was a
	 * hack in the implementation to avoid trying to match brackets before
	 * the tag-table property is set. But now the hack is no longer needed.
	 */
	table = ctk_text_tag_table_new ();

	buffer = g_object_new (CTK_SOURCE_TYPE_BUFFER,
			       "highlight-matching-brackets", FALSE,
			       "tag-table", table,
			       NULL);
	g_object_unref (buffer);

	buffer = g_object_new (CTK_SOURCE_TYPE_BUFFER,
			       "highlight-matching-brackets", TRUE,
			       "tag-table", table,
			       NULL);
	g_object_unref (buffer);

	g_object_unref (table);
}

int
main (int argc, char** argv)
{
	ctk_test_init (&argc, &argv);

	init_default_manager ();

	g_test_add_func ("/Buffer/bug-634510", test_get_buffer);
	g_test_add_func ("/Buffer/get-context-classes", test_get_context_classes);
	g_test_add_func ("/Buffer/change-case", test_change_case);
	g_test_add_func ("/Buffer/join-lines", test_join_lines);
	g_test_add_func ("/Buffer/sort-lines", test_sort_lines);
	g_test_add_func ("/Buffer/move-words", test_move_words);
	g_test_add_func ("/Buffer/bracket-matching", test_bracket_matching);

	return g_test_run();
}
