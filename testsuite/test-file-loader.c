/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2010 - Jesse van den Kieboom
 * Copyright (C) 2014 - Sébastien Wilmet
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

#include <ctksourceview/ctksource.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
	const gchar *expected_buffer_contents;
	gint newline_type;
} LoaderTestData;

static void
delete_file (GFile *location)
{
	if (g_file_query_exists (location, NULL))
	{
		GError *error = NULL;

		g_file_delete (location, NULL, &error);
		g_assert_no_error (error);
	}
}

static void
load_file_cb (GtkSourceFileLoader *loader,
	      GAsyncResult        *result,
	      LoaderTestData      *data)
{
	GError *error = NULL;

	ctk_source_file_loader_load_finish (loader, result, &error);
	g_assert_no_error (error);

	if (data->expected_buffer_contents != NULL)
	{
		GtkSourceBuffer *buffer;
		GtkTextIter start;
		GtkTextIter end;
		gchar *buffer_contents;

		buffer = ctk_source_file_loader_get_buffer (loader);

		ctk_text_buffer_get_bounds (CTK_TEXT_BUFFER (buffer), &start, &end);
		buffer_contents = ctk_text_iter_get_slice (&start, &end);

		g_assert_cmpstr (buffer_contents, ==, data->expected_buffer_contents);

		g_free (buffer_contents);
	}

	if (data->newline_type != -1)
	{
		g_assert_cmpint (ctk_source_file_loader_get_newline_type (loader),
		                 ==,
		                 data->newline_type);
	}

	/* finished */
	ctk_main_quit ();
}

static void
test_loader (const gchar *filename,
             const gchar *contents,
             const gchar *expected_buffer_contents,
             gint         newline_type)
{
	GFile *location;
	GtkSourceBuffer *buffer;
	GtkSourceFile *file;
	GtkSourceFileLoader *loader;
	GSList *candidate_encodings;
	LoaderTestData *data;
	GError *error = NULL;

	g_file_set_contents (filename, contents, -1, &error);
	g_assert_no_error (error);

	location = g_file_new_for_path (filename);
	buffer = ctk_source_buffer_new (NULL);
	file = ctk_source_file_new ();
	ctk_source_file_set_location (file, location);
	loader = ctk_source_file_loader_new (buffer, file);

	candidate_encodings = g_slist_prepend (NULL, (gpointer) ctk_source_encoding_get_utf8 ());
	ctk_source_file_loader_set_candidate_encodings (loader, candidate_encodings);

	data = g_slice_new (LoaderTestData);
	data->expected_buffer_contents = expected_buffer_contents;
	data->newline_type = newline_type;

	ctk_source_file_loader_load_async (loader,
					   G_PRIORITY_DEFAULT,
					   NULL, NULL, NULL, NULL,
					   (GAsyncReadyCallback) load_file_cb,
					   data);

	ctk_main ();

	g_slice_free (LoaderTestData, data);
	delete_file (location);
	g_object_unref (location);
	g_object_unref (buffer);
	g_object_unref (file);
	g_object_unref (loader);
	g_slist_free (candidate_encodings);
}

static void
test_end_line_stripping (void)
{
	test_loader ("file-loader.txt",
	             "hello world\n",
	             "hello world",
	             -1);

	test_loader ("file-loader.txt",
	             "hello world",
	             "hello world",
	             -1);

	test_loader ("file-loader.txt",
	             "\nhello world",
	             "\nhello world",
	             -1);

	test_loader ("file-loader.txt",
	             "\nhello world\n",
	             "\nhello world",
	             -1);

	test_loader ("file-loader.txt",
	             "hello world\n\n",
	             "hello world\n",
	             -1);

	test_loader ("file-loader.txt",
	             "hello world\r\n",
	             "hello world",
	             -1);

	test_loader ("file-loader.txt",
	             "hello world\r\n\r\n",
	             "hello world\r\n",
	             -1);

	test_loader ("file-loader.txt",
	             "\n",
	             "",
	             -1);

	test_loader ("file-loader.txt",
	             "\r\n",
	             "",
	             -1);

	test_loader ("file-loader.txt",
	             "\n\n",
	             "\n",
	             -1);

	test_loader ("file-loader.txt",
	             "\r\n\r\n",
	             "\r\n",
	             -1);
}

static void
test_end_new_line_detection (void)
{
	test_loader ("file-loader.txt",
	             "hello world\n",
	             NULL,
	             CTK_SOURCE_NEWLINE_TYPE_LF);

	test_loader ("file-loader.txt",
	             "hello world\r\n",
	             NULL,
	             CTK_SOURCE_NEWLINE_TYPE_CR_LF);

	test_loader ("file-loader.txt",
	             "hello world\r",
	             NULL,
	             CTK_SOURCE_NEWLINE_TYPE_CR);
}

static void
test_begin_new_line_detection (void)
{
	test_loader ("file-loader.txt",
	             "\nhello world",
	             NULL,
	             CTK_SOURCE_NEWLINE_TYPE_LF);

	test_loader ("file-loader.txt",
	             "\r\nhello world",
	             NULL,
	             CTK_SOURCE_NEWLINE_TYPE_CR_LF);

	test_loader ("file-loader.txt",
	             "\rhello world",
	             NULL,
	             CTK_SOURCE_NEWLINE_TYPE_CR);
}

gint
main (gint   argc,
      gchar *argv[])
{
	g_test_init (&argc, &argv, NULL);

	g_test_add_func ("/file-loader/end-line-stripping", test_end_line_stripping);
	g_test_add_func ("/file-loader/end-new-line-detection", test_end_new_line_detection);
	g_test_add_func ("/file-loader/begin-new-line-detection", test_begin_new_line_detection);

	return g_test_run ();
}
