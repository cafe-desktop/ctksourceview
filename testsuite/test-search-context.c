/*
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2013, 2015, 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * GtkSourceView is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * GtkSourceView is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <ctk/ctk.h>
#include <ctksourceview/ctksource.h>

typedef struct
{
	gint match_start_offset;
	gint match_end_offset;
	guint found : 1;
	guint has_wrapped_around : 1;
} SearchResult;

typedef struct
{
	SearchResult *results;
	gint result_num;
	guint forward : 1;
} AsyncData;

static void check_async_search_results (GtkSourceSearchContext *context,
					SearchResult           *results,
					gboolean                forward,
					gboolean                start_check);

static gchar *
get_buffer_contents (GtkTextBuffer *buffer)
{
	GtkTextIter start;
	GtkTextIter end;

	ctk_text_buffer_get_bounds (buffer, &start, &end);
	return ctk_text_iter_get_visible_text (&start, &end);
}

/* If we are running from the source dir (e.g. during make check)
 * we override the path to read from the data dir.
 */
static void
init_style_scheme_manager (void)
{
	gchar *dir;

	dir = g_build_filename (TOP_SRCDIR, "data", "styles", NULL);

	if (g_file_test (dir, G_FILE_TEST_IS_DIR))
	{
		GtkSourceStyleSchemeManager *manager;
		gchar **dirs;

		manager = ctk_source_style_scheme_manager_get_default ();

		dirs = g_new0 (gchar *, 2);
		dirs[0] = dir;

		ctk_source_style_scheme_manager_set_search_path (manager, dirs);
		g_strfreev (dirs);
	}
	else
	{
		g_free (dir);
	}
}

static void
flush_queue (void)
{
	while (ctk_events_pending ())
	{
		ctk_main_iteration ();
	}
}

/* Without insertion or deletion of text in the buffer afterwards. */
static void
test_occurrences_count_simple (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);
	GtkTextIter iter;
	gint occurrences_count;

	ctk_text_buffer_get_start_iter (text_buffer, &iter);
	ctk_text_buffer_insert (text_buffer, &iter, "Some foo\nSome bar\n", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 0);

	ctk_source_search_settings_set_search_text (settings, "world");
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 0);

	ctk_source_search_settings_set_search_text (settings, "Some");
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 2);

	ctk_source_search_settings_set_search_text (settings, "foo");
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	ctk_source_search_settings_set_search_text (settings, "world");
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 0);

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_occurrences_count_with_insert (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);
	GtkTextIter iter;
	gint occurrences_count;

	/* Contents: "foobar" */
	ctk_text_buffer_get_start_iter (text_buffer, &iter);
	ctk_text_buffer_insert (text_buffer, &iter, "foobar", -1);

	ctk_source_search_settings_set_search_text (settings, "foo");
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	/* Contents: "foobar " */
	ctk_text_buffer_get_end_iter (text_buffer, &iter);
	ctk_text_buffer_insert (text_buffer, &iter, " ", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	/* Contents: "foobar foobeer" */
	ctk_text_buffer_get_end_iter (text_buffer, &iter);
	ctk_text_buffer_insert (text_buffer, &iter, "foobeer", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 2);

	/* Contents: "foo bar foobeer" */
	ctk_text_buffer_get_iter_at_offset (text_buffer, &iter, 3);
	ctk_text_buffer_insert (text_buffer, &iter, " ", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 2);

	/* Contents: "foto bar foobeer" */
	ctk_text_buffer_get_iter_at_offset (text_buffer, &iter, 2);
	ctk_text_buffer_insert (text_buffer, &iter, "t", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	/* Contents: "footo bar foobeer" */
	ctk_text_buffer_get_iter_at_offset (text_buffer, &iter, 2);
	ctk_text_buffer_insert (text_buffer, &iter, "o", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 2);

	/* Contents: "foofooto bar foobeer" */
	ctk_text_buffer_get_start_iter (text_buffer, &iter);
	ctk_text_buffer_insert (text_buffer, &iter, "foo", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 3);

	/* Contents: "fooTfooto bar foobeer" */
	ctk_text_buffer_get_iter_at_offset (text_buffer, &iter, 3);
	ctk_text_buffer_insert (text_buffer, &iter, "T", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 3);

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_occurrences_count_with_delete (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);
	GtkTextIter start;
	GtkTextIter end;
	gint occurrences_count;

	ctk_source_search_settings_set_search_text (settings, "foo");

	/* Contents: "foo" -> "" */
	ctk_text_buffer_set_text (text_buffer, "foo", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	ctk_text_buffer_get_bounds (text_buffer, &start, &end);
	ctk_text_buffer_delete (text_buffer, &start, &end);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 0);

	/* Contents: "foo" -> "oo" */
	ctk_text_buffer_set_text (text_buffer, "foo", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	ctk_text_buffer_get_start_iter (text_buffer, &start);
	ctk_text_buffer_get_iter_at_offset (text_buffer, &end, 1);
	ctk_text_buffer_delete (text_buffer, &start, &end);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 0);

	/* Contents: "foobar foobeer" -> "foobar" */
	ctk_text_buffer_set_text (text_buffer, "foobar foobeer", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 2);

	ctk_text_buffer_get_iter_at_offset (text_buffer, &start, 6);
	ctk_text_buffer_get_end_iter (text_buffer, &end);
	ctk_text_buffer_delete (text_buffer, &start, &end);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	/* Contents: "foo[foo]foo" -> "foofoo" */
	ctk_text_buffer_set_text (text_buffer, "foofoofoo", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 3);

	ctk_text_buffer_get_iter_at_offset (text_buffer, &start, 3);
	ctk_text_buffer_get_iter_at_offset (text_buffer, &end, 6);
	ctk_text_buffer_delete (text_buffer, &start, &end);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 2);

	/* Contents: "fo[of]oo" -> "fooo" */
	ctk_text_buffer_get_iter_at_offset (text_buffer, &start, 2);
	ctk_text_buffer_get_iter_at_offset (text_buffer, &end, 4);
	ctk_text_buffer_delete (text_buffer, &start, &end);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	/* Contents: "foto" -> "foo" */
	ctk_text_buffer_set_text (text_buffer, "foto", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 0);

	ctk_text_buffer_get_iter_at_offset (text_buffer, &start, 2);
	ctk_text_buffer_get_iter_at_offset (text_buffer, &end, 3);
	ctk_text_buffer_delete (text_buffer, &start, &end);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_occurrences_count_multiple_lines (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);
	gint occurrences_count;

	ctk_source_search_settings_set_search_text (settings, "world\nhello");

	ctk_text_buffer_set_text (text_buffer, "hello world\nhello world\nhello world\n", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 2);

	ctk_source_search_settings_set_search_text (settings, "world\n");
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 3);

	ctk_source_search_settings_set_search_text (settings, "\nhello world\n");
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_case_sensitivity (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);
	gboolean case_sensitive;
	gint occurrences_count;

	ctk_text_buffer_set_text (text_buffer, "Case", -1);
	ctk_source_search_settings_set_search_text (settings, "case");

	ctk_source_search_settings_set_case_sensitive (settings, TRUE);
	case_sensitive = ctk_source_search_settings_get_case_sensitive (settings);
	g_assert_true (case_sensitive);

	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 0);

	ctk_source_search_settings_set_case_sensitive (settings, FALSE);
	case_sensitive = ctk_source_search_settings_get_case_sensitive (settings);
	g_assert_false (case_sensitive);

	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_search_at_word_boundaries (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);
	GtkTextIter iter;
	gboolean at_word_boundaries;
	gint occurrences_count;

	ctk_text_buffer_set_text (text_buffer, "AtWordBoundaries AtWord", -1);
	ctk_source_search_settings_set_search_text (settings, "AtWord");

	ctk_source_search_settings_set_at_word_boundaries (settings, TRUE);
	at_word_boundaries = ctk_source_search_settings_get_at_word_boundaries (settings);
	g_assert_true (at_word_boundaries);

	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	/* Contents: "AtWordBoundaries AtWord AtWord" */
	ctk_text_buffer_get_iter_at_offset (text_buffer, &iter, 16);
	ctk_text_buffer_insert (text_buffer, &iter, " AtWord", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 2);

	/* Contents: "AtWordBoundaries AtWordd AtWord" */
	ctk_text_buffer_get_iter_at_offset (text_buffer, &iter, 23);
	ctk_text_buffer_insert (text_buffer, &iter, "d", -1);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	ctk_source_search_settings_set_at_word_boundaries (settings, FALSE);
	at_word_boundaries = ctk_source_search_settings_get_at_word_boundaries (settings);
	g_assert_false (at_word_boundaries);

	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 3);

	/* Word with underscores */

	ctk_text_buffer_set_text (text_buffer, "_hello_world_ _hello_", -1);
	ctk_source_search_settings_set_search_text (settings, "_hello_");

	ctk_source_search_settings_set_at_word_boundaries (settings, TRUE);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	ctk_source_search_settings_set_at_word_boundaries (settings, FALSE);
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 2);

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
check_search_results (GtkSourceBuffer        *source_buffer,
		      GtkSourceSearchContext *context,
		      SearchResult           *results,
		      gboolean                forward)
{
	GtkTextIter iter;
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);

	ctk_text_buffer_get_start_iter (text_buffer, &iter);

	do
	{
		gint i;
		gboolean found;
		gboolean has_wrapped_around;
		GtkTextIter match_start;
		GtkTextIter match_end;

		i = ctk_text_iter_get_offset (&iter);

		if (forward)
		{
			found = ctk_source_search_context_forward (context,
								   &iter,
								   &match_start,
								   &match_end,
								   &has_wrapped_around);
		}
		else
		{
			found = ctk_source_search_context_backward (context,
								    &iter,
								    &match_start,
								    &match_end,
								    &has_wrapped_around);
		}

		g_assert_cmpint (found, ==, results[i].found);
		g_assert_cmpint (has_wrapped_around, ==, results[i].has_wrapped_around);

		if (found)
		{
			gint match_start_offset = ctk_text_iter_get_offset (&match_start);
			gint match_end_offset = ctk_text_iter_get_offset (&match_end);

			g_assert_cmpint (match_start_offset, ==, results[i].match_start_offset);
			g_assert_cmpint (match_end_offset, ==, results[i].match_end_offset);
		}
	}
	while (ctk_text_iter_forward_char (&iter));
}

static void
finish_check_result (GtkSourceSearchContext *context,
		     GAsyncResult           *result,
		     AsyncData              *data)
{
	GtkTextIter match_start;
	GtkTextIter match_end;
	gboolean found;
	gboolean has_wrapped_around;
	SearchResult search_result = data->results[data->result_num];

	if (data->forward)
	{
		found = ctk_source_search_context_forward_finish (context,
								  result,
								  &match_start,
								  &match_end,
								  &has_wrapped_around,
								  NULL);
	}
	else
	{
		found = ctk_source_search_context_backward_finish (context,
								   result,
								   &match_start,
								   &match_end,
								   &has_wrapped_around,
								   NULL);
	}

	g_assert_cmpint (found, ==, search_result.found);
	g_assert_cmpint (has_wrapped_around, ==, search_result.has_wrapped_around);

	if (found)
	{
		gint match_start_offset = ctk_text_iter_get_offset (&match_start);
		gint match_end_offset = ctk_text_iter_get_offset (&match_end);

		g_assert_cmpint (match_start_offset, ==, search_result.match_start_offset);
		g_assert_cmpint (match_end_offset, ==, search_result.match_end_offset);
	}

	check_async_search_results (context,
				    data->results,
				    data->forward,
				    FALSE);

	g_slice_free (AsyncData, data);
}

static void
check_async_search_results (GtkSourceSearchContext *context,
			    SearchResult           *results,
			    gboolean                forward,
			    gboolean                start_check)
{
	GtkSourceBuffer *source_buffer = ctk_source_search_context_get_buffer (context);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	static GtkTextIter iter;
	AsyncData *data;

	if (start_check)
	{
		ctk_text_buffer_get_start_iter (text_buffer, &iter);
	}
	else if (!ctk_text_iter_forward_char (&iter))
	{
		ctk_main_quit ();
		return;
	}

	data = g_slice_new (AsyncData);
	data->results = results;
	data->result_num = ctk_text_iter_get_offset (&iter);
	data->forward = forward;

	if (forward)
	{
		ctk_source_search_context_forward_async (context,
							 &iter,
							 NULL,
							 (GAsyncReadyCallback)finish_check_result,
							 data);
	}
	else
	{
		ctk_source_search_context_backward_async (context,
							  &iter,
							  NULL,
							  (GAsyncReadyCallback)finish_check_result,
							  data);
	}
}

static void
test_forward_search (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);
	gboolean wrap_around;

	static SearchResult results1[] =
	{
		{ 0, 2, TRUE, FALSE },
		{ 2, 4, TRUE, FALSE },
		{ 2, 4, TRUE, FALSE },
		{ 0, 2, TRUE, TRUE },
		{ 0, 2, TRUE, TRUE }
	};

	static SearchResult results2[] =
	{
		{ 0, 2, TRUE, FALSE },
		{ 2, 4, TRUE, FALSE },
		{ 2, 4, TRUE, FALSE },
		{ 0, 0, FALSE, FALSE },
		{ 0, 0, FALSE, FALSE }
	};

	ctk_text_buffer_set_text (text_buffer, "aaaa", -1);
	ctk_source_search_settings_set_search_text (settings, "aa");

	/* Wrap around: TRUE */

	ctk_source_search_settings_set_wrap_around (settings, TRUE);
	wrap_around = ctk_source_search_settings_get_wrap_around (settings);
	g_assert_true (wrap_around);

	check_search_results (source_buffer, context, results1, TRUE);

	ctk_source_search_settings_set_regex_enabled (settings, TRUE);
	check_search_results (source_buffer, context, results1, TRUE);
	ctk_source_search_settings_set_regex_enabled (settings, FALSE);

	g_test_trap_subprocess ("/Search/forward/subprocess/async-wrap-around",
				0,
				G_TEST_SUBPROCESS_INHERIT_STDERR);

	g_test_trap_assert_passed ();

	/* Wrap around: FALSE */

	ctk_source_search_settings_set_wrap_around (settings, FALSE);
	wrap_around = ctk_source_search_settings_get_wrap_around (settings);
	g_assert_false (wrap_around);

	check_search_results (source_buffer, context, results2, TRUE);

	ctk_source_search_settings_set_regex_enabled (settings, TRUE);
	check_search_results (source_buffer, context, results2, TRUE);
	ctk_source_search_settings_set_regex_enabled (settings, FALSE);

	g_test_trap_subprocess ("/Search/forward/subprocess/async-normal",
				0,
				G_TEST_SUBPROCESS_INHERIT_STDERR);

	g_test_trap_assert_passed ();

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_async_forward_search_normal (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);

	static SearchResult results[] =
	{
		{ 0, 2, TRUE, FALSE },
		{ 2, 4, TRUE, FALSE },
		{ 2, 4, TRUE, FALSE },
		{ 0, 0, FALSE, FALSE },
		{ 0, 0, FALSE, FALSE }
	};

	ctk_text_buffer_set_text (text_buffer, "aaaa", -1);
	ctk_source_search_settings_set_search_text (settings, "aa");

	check_async_search_results (context, results, TRUE, TRUE);

	ctk_main ();
	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_async_forward_search_wrap_around (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);

	static SearchResult results[] =
	{
		{ 0, 2, TRUE, FALSE },
		{ 2, 4, TRUE, FALSE },
		{ 2, 4, TRUE, FALSE },
		{ 0, 2, TRUE, TRUE },
		{ 0, 2, TRUE, TRUE }
	};

	ctk_text_buffer_set_text (text_buffer, "aaaa", -1);
	ctk_source_search_settings_set_search_text (settings, "aa");
	ctk_source_search_settings_set_wrap_around (settings, TRUE);

	check_async_search_results (context, results, TRUE, TRUE);

	ctk_main ();
	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_backward_search (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);

	static SearchResult results1[] =
	{
		{ 2, 4, TRUE, TRUE },
		{ 2, 4, TRUE, TRUE },
		{ 0, 2, TRUE, FALSE },
		{ 0, 2, TRUE, FALSE },
		{ 2, 4, TRUE, FALSE }
	};

	static SearchResult results2[] =
	{
		{ 0, 0, FALSE, FALSE },
		{ 0, 0, FALSE, FALSE },
		{ 0, 2, TRUE, FALSE },
		{ 0, 2, TRUE, FALSE },
		{ 2, 4, TRUE, FALSE }
	};

	ctk_text_buffer_set_text (text_buffer, "aaaa", -1);
	ctk_source_search_settings_set_search_text (settings, "aa");

	/* Wrap around: TRUE */

	ctk_source_search_settings_set_wrap_around (settings, TRUE);
	check_search_results (source_buffer, context, results1, FALSE);

	ctk_source_search_settings_set_regex_enabled (settings, TRUE);
	check_search_results (source_buffer, context, results1, FALSE);
	ctk_source_search_settings_set_regex_enabled (settings, FALSE);

	g_test_trap_subprocess ("/Search/backward/subprocess/async-wrap-around",
				0,
				G_TEST_SUBPROCESS_INHERIT_STDERR);

	g_test_trap_assert_passed ();

	/* Wrap around: FALSE */

	ctk_source_search_settings_set_wrap_around (settings, FALSE);
	check_search_results (source_buffer, context, results2, FALSE);

	ctk_source_search_settings_set_regex_enabled (settings, TRUE);
	check_search_results (source_buffer, context, results2, FALSE);
	ctk_source_search_settings_set_regex_enabled (settings, FALSE);

	g_test_trap_subprocess ("/Search/backward/subprocess/async-normal",
				0,
				G_TEST_SUBPROCESS_INHERIT_STDERR);

	g_test_trap_assert_passed ();

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_async_backward_search_normal (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);

	static SearchResult results[] =
	{
		{ 0, 0, FALSE, FALSE },
		{ 0, 0, FALSE, FALSE },
		{ 0, 2, TRUE, FALSE },
		{ 0, 2, TRUE, FALSE },
		{ 2, 4, TRUE, FALSE }
	};

	ctk_text_buffer_set_text (text_buffer, "aaaa", -1);
	ctk_source_search_settings_set_search_text (settings, "aa");

	check_async_search_results (context, results, FALSE, TRUE);

	ctk_main ();
	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_async_backward_search_wrap_around (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);

	static SearchResult results[] =
	{
		{ 2, 4, TRUE, TRUE },
		{ 2, 4, TRUE, TRUE },
		{ 0, 2, TRUE, FALSE },
		{ 0, 2, TRUE, FALSE },
		{ 2, 4, TRUE, FALSE }
	};

	ctk_text_buffer_set_text (text_buffer, "aaaa", -1);
	ctk_source_search_settings_set_search_text (settings, "aa");
	ctk_source_search_settings_set_wrap_around (settings, TRUE);

	check_async_search_results (context, results, FALSE, TRUE);

	ctk_main ();
	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_highlight (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkSourceSearchContext *context1 = ctk_source_search_context_new (source_buffer, NULL);
	GtkSourceSearchContext *context2 = ctk_source_search_context_new (source_buffer, NULL);
	gboolean highlight;

	ctk_source_search_context_set_highlight (context1, TRUE);
	highlight = ctk_source_search_context_get_highlight (context1);
	g_assert_true (highlight);

	ctk_source_search_context_set_highlight (context2, FALSE);
	highlight = ctk_source_search_context_get_highlight (context2);
	g_assert_false (highlight);

	g_object_unref (source_buffer);
	g_object_unref (context1);
	g_object_unref (context2);
}

static void
test_get_search_text (void)
{
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	const gchar *search_text;

	search_text = ctk_source_search_settings_get_search_text (settings);
	g_assert_null (search_text);

	ctk_source_search_settings_set_search_text (settings, "");
	search_text = ctk_source_search_settings_get_search_text (settings);
	g_assert_null (search_text);

	ctk_source_search_settings_set_search_text (settings, "search-text");
	search_text = ctk_source_search_settings_get_search_text (settings);
	g_assert_cmpstr (search_text, ==, "search-text");

	g_object_unref (settings);
}

static void
test_occurrence_position (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);
	GtkTextIter start;
	GtkTextIter end;
	gint pos;

	ctk_text_buffer_set_text (text_buffer, "aaaa", -1);
	ctk_source_search_settings_set_search_text (settings, "aa");
	flush_queue ();

	ctk_text_buffer_get_start_iter (text_buffer, &start);
	end = start;
	ctk_text_iter_forward_chars (&end, 2);

	pos = ctk_source_search_context_get_occurrence_position (context, &start, &end);
	g_assert_cmpint (pos, ==, 1);

	ctk_text_iter_forward_char (&start);
	ctk_text_iter_forward_char (&end);
	pos = ctk_source_search_context_get_occurrence_position (context, &start, &end);
	g_assert_cmpint (pos, ==, 0);

	ctk_text_iter_forward_char (&start);
	ctk_text_iter_forward_char (&end);
	pos = ctk_source_search_context_get_occurrence_position (context, &start, &end);
	g_assert_cmpint (pos, ==, 2);

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_replace (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);
	GtkTextIter start;
	GtkTextIter end;
	gint offset;
	gboolean replaced;
	gchar *contents;

	ctk_text_buffer_set_text (text_buffer, "aaaa", -1);
	ctk_source_search_settings_set_search_text (settings, "aa");
	flush_queue ();

	ctk_text_buffer_get_iter_at_offset (text_buffer, &start, 1);
	ctk_text_buffer_get_iter_at_offset (text_buffer, &end, 3);

	replaced = ctk_source_search_context_replace (context, &start, &end, "bbb", -1, NULL);
	g_assert_false (replaced);

	ctk_text_buffer_get_iter_at_offset (text_buffer, &start, 2);
	ctk_text_buffer_get_iter_at_offset (text_buffer, &end, 4);

	replaced = ctk_source_search_context_replace (context, &start, &end, "bbb", -1, NULL);
	g_assert_true (replaced);
	offset = ctk_text_iter_get_offset (&start);
	g_assert_cmpint (offset, ==, 2);
	offset = ctk_text_iter_get_offset (&end);
	g_assert_cmpint (offset, ==, 5);

	contents = get_buffer_contents (text_buffer);
	g_assert_cmpstr (contents, ==, "aabbb");
	g_free (contents);

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_replace_all (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);
	gint nb_replacements;
	gchar *contents;

	ctk_text_buffer_set_text (text_buffer, "aaaa", -1);
	ctk_source_search_settings_set_search_text (settings, "aa");
	flush_queue ();

	nb_replacements = ctk_source_search_context_replace_all (context, "bb", 2, NULL);
	g_assert_cmpint (nb_replacements, ==, 2);

	contents = get_buffer_contents (text_buffer);
	g_assert_cmpstr (contents, ==, "bbbb");
	g_free (contents);

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_regex_basics (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);
	gboolean regex_enabled;
	gint occurrences_count;
	GtkTextIter start;
	GtkTextIter end;
	gchar *contents;

	ctk_text_buffer_set_text (text_buffer, "hello\nworld\n", -1);
	ctk_source_search_settings_set_regex_enabled (settings, TRUE);
	regex_enabled = ctk_source_search_settings_get_regex_enabled (settings);
	g_assert_true (regex_enabled);

	/* Simple regex */

	ctk_source_search_settings_set_search_text (settings, "\\w+");
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 2);

	/* Test partial matching */

	ctk_source_search_settings_set_search_text (settings, "(.*\n)*");
	flush_queue ();
	occurrences_count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (occurrences_count, ==, 1);

	/* Test replace */

	ctk_text_buffer_set_text (text_buffer, "aa#bb", -1);
	ctk_source_search_settings_set_search_text (settings, "(\\w+)#(\\w+)");
	flush_queue ();

	ctk_text_buffer_get_start_iter (text_buffer, &start);
	ctk_text_buffer_get_end_iter (text_buffer, &end);
	ctk_source_search_context_replace (context, &start, &end, "\\2#\\1", -1, NULL);
	g_assert_true (ctk_text_iter_is_start (&start));
	g_assert_true (ctk_text_iter_is_end (&end));

	contents = get_buffer_contents (text_buffer);
	g_assert_cmpstr (contents, ==, "bb#aa");
	g_free (contents);

	/* Test replace all */

	ctk_text_buffer_set_text (text_buffer, "aa#bb cc#dd", -1);
	ctk_source_search_context_replace_all (context, "\\2#\\1", -1, NULL);

	contents = get_buffer_contents (text_buffer);
	g_assert_cmpstr (contents, ==, "bb#aa dd#cc");
	g_free (contents);

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_regex_at_word_boundaries (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);
	GtkTextIter iter;
	GtkTextIter match_start;
	GtkTextIter match_end;
	gboolean has_wrapped_around;
	gboolean found;
	gint offset;
	gchar *content;

	ctk_text_buffer_set_text (text_buffer, "1234\n12345\n1234", -1);

	ctk_source_search_settings_set_regex_enabled (settings, TRUE);
	ctk_source_search_settings_set_at_word_boundaries (settings, TRUE);
	ctk_source_search_settings_set_search_text (settings, "\\d{4}");

	ctk_text_buffer_get_start_iter (text_buffer, &iter);

	found = ctk_source_search_context_forward (context,
						   &iter,
						   &match_start,
						   &match_end,
						   &has_wrapped_around);
	g_assert_true (found);
	g_assert_false (has_wrapped_around);

	offset = ctk_text_iter_get_offset (&match_start);
	g_assert_cmpint (offset, ==, 0);
	offset = ctk_text_iter_get_offset (&match_end);
	g_assert_cmpint (offset, ==, 4);

	iter = match_end;
	found = ctk_source_search_context_forward (context,
						   &iter,
						   &match_start,
						   &match_end,
						   &has_wrapped_around);
	g_assert_true (found);
	g_assert_false (has_wrapped_around);

	offset = ctk_text_iter_get_offset (&match_start);
	g_assert_cmpint (offset, ==, 11);
	offset = ctk_text_iter_get_offset (&match_end);
	g_assert_cmpint (offset, ==, 15);

	/* Test replace, see https://bugzilla.gnome.org/show_bug.cgi?id=740810 */

	ctk_text_buffer_set_text (text_buffer, "&aa", -1);
	ctk_source_search_settings_set_search_text (settings, "aa");
	flush_queue ();

	ctk_text_buffer_get_iter_at_offset (text_buffer, &match_start, 1);
	ctk_text_buffer_get_end_iter (text_buffer, &match_end);
	ctk_source_search_context_replace (context, &match_start, &match_end, "bbb", -1, NULL);
	offset = ctk_text_iter_get_offset (&match_start);
	g_assert_cmpint (offset, ==, 1);
	g_assert_true (ctk_text_iter_is_end (&match_end));

	content = get_buffer_contents (text_buffer);
	g_assert_cmpstr (content, ==, "&bbb");
	g_free (content);

	/* Test replace multi-byte character */

	ctk_text_buffer_set_text (text_buffer, "–aa", -1);
	ctk_source_search_settings_set_search_text (settings, "aa");
	flush_queue ();

	ctk_text_buffer_get_iter_at_offset (text_buffer, &match_start, 1);
	ctk_text_buffer_get_end_iter (text_buffer, &match_end);
	ctk_source_search_context_replace (context, &match_start, &match_end, "bbb", -1, NULL);
	offset = ctk_text_iter_get_offset (&match_start);
	g_assert_cmpint (offset, ==, 1);
	g_assert_true (ctk_text_iter_is_end (&match_end));

	content = get_buffer_contents (text_buffer);
	g_assert_cmpstr (content, ==, "–bbb");
	g_free (content);

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_regex_look_behind (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);
	GtkTextIter iter;
	GtkTextIter match_start;
	GtkTextIter match_end;
	gint count;
	gint pos;
	gint offset;
	gboolean has_wrapped_around;
	gboolean found;
	gchar *contents;
	GError *error = NULL;

	ctk_text_buffer_set_text (text_buffer, "12\n23\n123\n23\n12", -1);

	ctk_source_search_settings_set_regex_enabled (settings, TRUE);
	ctk_source_search_settings_set_search_text (settings, "(?<=1)23");
	flush_queue ();

	/* Occurrences count */
	count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (count, ==, 1);

	/* Forward search */
	ctk_text_buffer_get_start_iter (text_buffer, &iter);
	found = ctk_source_search_context_forward (context,
						   &iter,
						   &match_start,
						   &match_end,
						   &has_wrapped_around);
	g_assert_true (found);
	g_assert_false (has_wrapped_around);

	offset = ctk_text_iter_get_offset (&match_start);
	g_assert_cmpint (offset, ==, 7);
	offset = ctk_text_iter_get_offset (&match_end);
	g_assert_cmpint (offset, ==, 9);

	/* Backward search */
	ctk_text_buffer_get_end_iter (text_buffer, &iter);
	found = ctk_source_search_context_backward (context,
						    &iter,
						    &match_start,
						    &match_end,
						    &has_wrapped_around);
	g_assert_true (found);
	g_assert_false (has_wrapped_around);

	offset = ctk_text_iter_get_offset (&match_start);
	g_assert_cmpint (offset, ==, 7);
	offset = ctk_text_iter_get_offset (&match_end);
	g_assert_cmpint (offset, ==, 9);

	/* Occurrence position */
	pos = ctk_source_search_context_get_occurrence_position (context, &match_start, &match_end);
	g_assert_cmpint (pos, ==, 1);

	/* Replace */
	ctk_source_search_context_replace (context, &match_start, &match_end, "R", -1, &error);
	g_assert_no_error (error);

	contents = get_buffer_contents (text_buffer);
	g_assert_cmpstr (contents, ==, "12\n23\n1R\n23\n12");
	g_free (contents);

	/* Replace all */
	ctk_text_buffer_set_text (text_buffer, "12\n23\n123 123\n23\n12", -1);
	flush_queue ();

	ctk_source_search_context_replace_all (context, "R", -1, &error);
	g_assert_no_error (error);

	contents = get_buffer_contents (text_buffer);
	g_assert_cmpstr (contents, ==, "12\n23\n1R 1R\n23\n12");
	g_free (contents);

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_regex_look_ahead (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);
	GtkTextIter iter;
	GtkTextIter match_start;
	GtkTextIter match_end;
	gint count;
	gint pos;
	gint offset;
	gboolean has_wrapped_around;
	gboolean found;
	gchar *contents;
	GError *error = NULL;

	ctk_text_buffer_set_text (text_buffer, "12\n23\n123\n23\n12", -1);

	ctk_source_search_settings_set_regex_enabled (settings, TRUE);
	ctk_source_search_settings_set_search_text (settings, "12(?=3)");
	flush_queue ();

	/* Occurrences count */
	count = ctk_source_search_context_get_occurrences_count (context);
	g_assert_cmpint (count, ==, 1);

	/* Forward search */
	ctk_text_buffer_get_start_iter (text_buffer, &iter);
	found = ctk_source_search_context_forward (context,
						   &iter,
						   &match_start,
						   &match_end,
						   &has_wrapped_around);
	g_assert_true (found);
	g_assert_false (has_wrapped_around);

	offset = ctk_text_iter_get_offset (&match_start);
	g_assert_cmpint (offset, ==, 6);
	offset = ctk_text_iter_get_offset (&match_end);
	g_assert_cmpint (offset, ==, 8);

	/* Backward search */
	ctk_text_buffer_get_end_iter (text_buffer, &iter);
	found = ctk_source_search_context_backward (context,
						    &iter,
						    &match_start,
						    &match_end,
						    &has_wrapped_around);
	g_assert_true (found);
	g_assert_false (has_wrapped_around);

	offset = ctk_text_iter_get_offset (&match_start);
	g_assert_cmpint (offset, ==, 6);
	offset = ctk_text_iter_get_offset (&match_end);
	g_assert_cmpint (offset, ==, 8);

	/* Occurrence position */
	pos = ctk_source_search_context_get_occurrence_position (context, &match_start, &match_end);
	g_assert_cmpint (pos, ==, 1);

	/* Replace */
	ctk_source_search_context_replace (context, &match_start, &match_end, "R", -1, &error);
	g_assert_no_error (error);

	contents = get_buffer_contents (text_buffer);
	g_assert_cmpstr (contents, ==, "12\n23\nR3\n23\n12");
	g_free (contents);

	/* Replace all */
	ctk_text_buffer_set_text (text_buffer, "12\n23\n123 123\n23\n12", -1);
	flush_queue ();

	ctk_source_search_context_replace_all (context, "R", -1, &error);
	g_assert_no_error (error);

	contents = get_buffer_contents (text_buffer);
	g_assert_cmpstr (contents, ==, "12\n23\nR3 R3\n23\n12");
	g_free (contents);

	g_object_unref (source_buffer);
	g_object_unref (settings);
	g_object_unref (context);
}

static void
test_destroy_buffer_during_search (void)
{
	GtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	GtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	GtkSourceSearchSettings *settings = ctk_source_search_settings_new ();
	GtkSourceSearchContext *context = ctk_source_search_context_new (source_buffer, settings);

	ctk_text_buffer_set_text (text_buffer, "y", -1);
	ctk_source_search_settings_set_search_text (settings, "y");

	/* Destroy buffer during search. */
	g_object_unref (source_buffer);
	flush_queue ();

	/* Test also a new search when buffer already destroyed. */
	ctk_source_search_settings_set_search_text (settings, "x");
	flush_queue ();

	g_object_unref (settings);
	g_object_unref (context);
}

int
main (int argc, char **argv)
{
	ctk_test_init (&argc, &argv);

	init_style_scheme_manager ();

	g_test_add_func ("/Search/occurrences-count/simple", test_occurrences_count_simple);
	g_test_add_func ("/Search/occurrences-count/with-insert", test_occurrences_count_with_insert);
	g_test_add_func ("/Search/occurrences-count/with-delete", test_occurrences_count_with_delete);
	g_test_add_func ("/Search/occurrences-count/multiple-lines", test_occurrences_count_multiple_lines);
	g_test_add_func ("/Search/case-sensitivity", test_case_sensitivity);
	g_test_add_func ("/Search/at-word-boundaries", test_search_at_word_boundaries);
	g_test_add_func ("/Search/forward", test_forward_search);
	g_test_add_func ("/Search/forward/subprocess/async-normal", test_async_forward_search_normal);
	g_test_add_func ("/Search/forward/subprocess/async-wrap-around", test_async_forward_search_wrap_around);
	g_test_add_func ("/Search/backward", test_backward_search);
	g_test_add_func ("/Search/backward/subprocess/async-normal", test_async_backward_search_normal);
	g_test_add_func ("/Search/backward/subprocess/async-wrap-around", test_async_backward_search_wrap_around);
	g_test_add_func ("/Search/highlight", test_highlight);
	g_test_add_func ("/Search/get-search-text", test_get_search_text);
	g_test_add_func ("/Search/occurrence-position", test_occurrence_position);
	g_test_add_func ("/Search/replace", test_replace);
	g_test_add_func ("/Search/replace_all", test_replace_all);
	g_test_add_func ("/Search/regex/basics", test_regex_basics);
	g_test_add_func ("/Search/regex/at-word-boundaries", test_regex_at_word_boundaries);
	g_test_add_func ("/Search/regex/look-behind", test_regex_look_behind);
	g_test_add_func ("/Search/regex/look-ahead", test_regex_look_ahead);
	g_test_add_func ("/Search/destroy-buffer-during-search", test_destroy_buffer_during_search);

	return g_test_run ();
}
