/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2013 - Paolo Borelli
 * Copyright (C) 2013 - Sébastien Wilmet
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

static void
test_create (void)
{
	CtkSourceMark *m;

	m = ctk_source_mark_new ("Mark 1", "test");
	g_assert_cmpstr ("Mark 1", ==, ctk_text_mark_get_name (CTK_TEXT_MARK (m)));
	g_assert_cmpstr ("test", ==, ctk_source_mark_get_category (m));
	g_assert_null (ctk_text_mark_get_buffer (CTK_TEXT_MARK (m)));
	g_assert_null (ctk_source_mark_next (m, NULL));
	g_assert_null (ctk_source_mark_prev (m, NULL));
	g_object_unref (m);
}

static void
test_prev_next (void)
{
	CtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	CtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	CtkSourceMark *mark1, *mark2, *mark3;
	CtkTextIter iter;

	ctk_text_buffer_set_text (text_buffer, "text", -1);

	ctk_text_buffer_get_start_iter (text_buffer, &iter);
	mark1 = ctk_source_buffer_create_source_mark (source_buffer, NULL, "cat1", &iter);

	ctk_text_iter_forward_char (&iter);
	mark2 = ctk_source_buffer_create_source_mark (source_buffer, NULL, "cat2", &iter);

	ctk_text_iter_forward_char (&iter);
	mark3 = ctk_source_buffer_create_source_mark (source_buffer, NULL, "cat1", &iter);

	g_assert_true (mark2 == ctk_source_mark_next (mark1, NULL));
	g_assert_true (mark3 == ctk_source_mark_next (mark1, "cat1"));
	g_assert_null (ctk_source_mark_next (mark2, "cat2"));
	g_assert_null (ctk_source_mark_next (mark3, NULL));

	g_assert_true (mark1 == ctk_source_mark_prev (mark2, NULL));
	g_assert_true (mark1 == ctk_source_mark_prev (mark3, "cat1"));
	g_assert_null (ctk_source_mark_prev (mark2, "cat2"));
	g_assert_null (ctk_source_mark_prev (mark1, NULL));

	g_object_unref (source_buffer);
}

static void
test_forward_backward_iter (void)
{
	CtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	CtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	CtkTextIter iter;

	ctk_text_buffer_set_text (text_buffer, "text", -1);

	ctk_text_buffer_get_start_iter (text_buffer, &iter);
	ctk_source_buffer_create_source_mark (source_buffer, NULL, "cat1", &iter);

	ctk_text_iter_forward_char (&iter);
	ctk_source_buffer_create_source_mark (source_buffer, NULL, "cat2", &iter);

	ctk_text_iter_forward_char (&iter);
	ctk_source_buffer_create_source_mark (source_buffer, NULL, "cat1", &iter);

	ctk_text_buffer_get_start_iter (text_buffer, &iter);
	ctk_source_buffer_forward_iter_to_source_mark (source_buffer, &iter, NULL);
	g_assert_cmpint (1, ==, ctk_text_iter_get_offset (&iter));

	ctk_text_buffer_get_start_iter (text_buffer, &iter);
	ctk_source_buffer_forward_iter_to_source_mark (source_buffer, &iter, "cat1");
	g_assert_cmpint (2, ==, ctk_text_iter_get_offset (&iter));

	ctk_text_buffer_get_end_iter (text_buffer, &iter);
	ctk_source_buffer_backward_iter_to_source_mark (source_buffer, &iter, NULL);
	g_assert_cmpint (2, ==, ctk_text_iter_get_offset (&iter));

	ctk_text_buffer_get_end_iter (text_buffer, &iter);
	ctk_source_buffer_backward_iter_to_source_mark (source_buffer, &iter, "cat2");
	g_assert_cmpint (1, ==, ctk_text_iter_get_offset (&iter));

	g_object_unref (source_buffer);
}

static void
test_get_source_marks_at_iter (void)
{
	CtkSourceBuffer *source_buffer = ctk_source_buffer_new (NULL);
	CtkTextBuffer *text_buffer = CTK_TEXT_BUFFER (source_buffer);
	CtkSourceMark *mark1, *mark2, *mark3;
	CtkTextIter iter;
	GSList *list;

	ctk_text_buffer_set_text (text_buffer, "text", -1);

	ctk_text_buffer_get_start_iter (text_buffer, &iter);
	mark1 = ctk_source_buffer_create_source_mark (source_buffer, NULL, "cat1", &iter);
	mark2 = ctk_source_buffer_create_source_mark (source_buffer, NULL, "cat2", &iter);
	mark3 = ctk_source_buffer_create_source_mark (source_buffer, NULL, "cat1", &iter);

	list = ctk_source_buffer_get_source_marks_at_iter (source_buffer, &iter, "cat1");
	g_assert_cmpint (2, ==, g_slist_length (list));
	g_assert_nonnull (g_slist_find (list, mark1));
	g_assert_nonnull (g_slist_find (list, mark3));
	g_slist_free (list);

	list = ctk_source_buffer_get_source_marks_at_iter (source_buffer, &iter, NULL);
	g_assert_cmpint (3, ==, g_slist_length (list));
	g_assert_nonnull (g_slist_find (list, mark1));
	g_assert_nonnull (g_slist_find (list, mark2));
	g_assert_nonnull (g_slist_find (list, mark3));

	g_object_unref (source_buffer);
}

int
main (int argc, char** argv)
{
	ctk_test_init (&argc, &argv);

	g_test_add_func ("/Mark/create", test_create);
	g_test_add_func ("/Mark/prev-next", test_prev_next);
	g_test_add_func ("/Mark/forward-backward-iter", test_forward_backward_iter);
	g_test_add_func ("/Mark/get-source-marks-at-iter", test_get_source_marks_at_iter);

	return g_test_run();
}
