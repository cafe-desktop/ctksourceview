/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2003 - Gustavo Giráldez
 * Copyright (C) 2006, 2013 - Paolo Borelli
 * Copyright (C) 2013, 2016 - Sébastien Wilmet
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

#include <ctksourceview/ctksource.h>

static void
test_weak_ref (void)
{
	CtkTextBuffer *buffer;
	CtkSourceRegion *region;
	CtkSourceRegionIter region_iter;
	CtkTextIter start;
	CtkTextIter end;

	buffer = ctk_text_buffer_new (NULL);
	region = ctk_source_region_new (buffer);

	ctk_text_buffer_set_text (buffer, "test_weak_ref", -1);
	ctk_text_buffer_get_bounds (buffer, &start, &end);
	ctk_source_region_add_subregion (region, &start, &end);

	g_assert_false (ctk_source_region_is_empty (region));

	g_object_unref (buffer);

	g_assert_true (ctk_source_region_is_empty (region));
	g_assert_false (ctk_source_region_get_bounds (region, &start, &end));

	ctk_source_region_get_start_region_iter (region, &region_iter);
	g_assert_false (ctk_source_region_iter_get_subregion (&region_iter, &start, &end));

	g_object_unref (region);
}

static void
add_subregion (CtkSourceRegion *region,
	       gint             start_offset,
	       gint             end_offset)
{
	CtkTextBuffer *buffer;
	CtkTextIter start_iter;
	CtkTextIter end_iter;

	buffer = ctk_source_region_get_buffer (region);
	ctk_text_buffer_get_iter_at_offset (buffer, &start_iter, start_offset);
	ctk_text_buffer_get_iter_at_offset (buffer, &end_iter, end_offset);

	ctk_source_region_add_subregion (region, &start_iter, &end_iter);
}

static void
subtract_subregion (CtkSourceRegion *region,
		    gint             start_offset,
		    gint             end_offset)
{
	CtkTextBuffer *buffer;
	CtkTextIter start_iter;
	CtkTextIter end_iter;

	buffer = ctk_source_region_get_buffer (region);
	ctk_text_buffer_get_iter_at_offset (buffer, &start_iter, start_offset);
	ctk_text_buffer_get_iter_at_offset (buffer, &end_iter, end_offset);

	ctk_source_region_subtract_subregion (region, &start_iter, &end_iter);
}

static void
check_result (CtkSourceRegion *region,
	      const gchar     *expected_result)
{
	gchar *expected_region_str;
	gchar *region_str;

	if (expected_result == NULL)
	{
		g_assert_true (ctk_source_region_is_empty (region));
		return;
	}

	g_assert_false (ctk_source_region_is_empty (region));

	expected_region_str = g_strconcat ("Subregions: ", expected_result, NULL);
	region_str = ctk_source_region_to_string (region);
	g_assert_cmpstr (region_str, ==, expected_region_str);

	g_free (expected_region_str);
	g_free (region_str);
}

static void
test_add_subtract_subregion (void)
{
	CtkTextBuffer *buffer;
	CtkSourceRegion *region;

	buffer = ctk_text_buffer_new (NULL);
	region = ctk_source_region_new (buffer);

	ctk_text_buffer_set_text (buffer, "This is a test of CtkSourceRegion", -1);

	g_assert_true (ctk_source_region_is_empty (region));

	/* Add/remove 0-length subregions */
	add_subregion (region, 5, 5);
	g_assert_true (ctk_source_region_is_empty (region));
	subtract_subregion (region, 5, 5);
	g_assert_true (ctk_source_region_is_empty (region));

	/* Add subregion */
	add_subregion (region, 5, 10);
	check_result (region, "5-10");

	/* Add two adjacent subregions */
	add_subregion (region, 3, 5);
	check_result (region, "3-10");

	add_subregion (region, 10, 12);
	check_result (region, "3-12");

	/* Remove all */
	subtract_subregion (region, 1, 15);
	g_assert_true (ctk_source_region_is_empty (region));

	/* Add two separate subregions */
	add_subregion (region, 5, 10);
	add_subregion (region, 15, 20);
	check_result (region, "5-10 15-20");

	/* Join them */
	add_subregion (region, 7, 17);
	check_result (region, "5-20");

	/* Remove from the middle */
	subtract_subregion (region, 10, 15);
	check_result (region, "5-10 15-20");

	/* Exactly remove a subregion */
	subtract_subregion (region, 15, 20);
	check_result (region, "5-10");

	/* Try to remove an adjacent subregion */
	subtract_subregion (region, 10, 20);
	check_result (region, "5-10");

	subtract_subregion (region, 0, 5);
	check_result (region, "5-10");

	/* Add another separate subregion */
	add_subregion (region, 15, 20);
	check_result (region, "5-10 15-20");

	/* Join with excess */
	add_subregion (region, 0, 25);
	check_result (region, "0-25");

	/* Do two holes */
	subtract_subregion (region, 5, 10);
	check_result (region, "0-5 10-25");

	subtract_subregion (region, 15, 20);
	check_result (region, "0-5 10-15 20-25");

	/* Remove the middle subregion */
	subtract_subregion (region, 8, 22);
	check_result (region, "0-5 22-25");

	/* Add the subregion we just removed */
	add_subregion (region, 10, 15);
	check_result (region, "0-5 10-15 22-25");

	/* Remove the middle subregion */
	subtract_subregion (region, 3, 17);
	check_result (region, "0-3 22-25");

	/* Add the subregion we just removed */
	add_subregion (region, 10, 15);
	check_result (region, "0-3 10-15 22-25");

	/* Remove the middle subregion */
	subtract_subregion (region, 2, 23);
	check_result (region, "0-2 23-25");

	/* Add the subregion we just removed */
	add_subregion (region, 10, 15);
	check_result (region, "0-2 10-15 23-25");

	g_object_unref (buffer);
	g_object_unref (region);
}

static void
do_intersection_subregion (CtkSourceRegion *region,
			   gint             start_offset,
			   gint             end_offset,
			   const gchar     *expected_result)
{
	CtkTextBuffer *buffer;
	CtkTextIter start_iter;
	CtkTextIter end_iter;
	CtkSourceRegion *intersection;

	buffer = ctk_source_region_get_buffer (region);
	ctk_text_buffer_get_iter_at_offset (buffer, &start_iter, start_offset);
	ctk_text_buffer_get_iter_at_offset (buffer, &end_iter, end_offset);

	intersection = ctk_source_region_intersect_subregion (region, &start_iter, &end_iter);
	check_result (intersection, expected_result);
	g_clear_object (&intersection);
}

static void
test_intersect_subregion (void)
{
	CtkTextBuffer *buffer;
	CtkSourceRegion *region;

	buffer = ctk_text_buffer_new (NULL);
	region = ctk_source_region_new (buffer);

	ctk_text_buffer_set_text (buffer, "This is a test of CtkSourceRegion", -1);

	g_assert_true (ctk_source_region_is_empty (region));

	add_subregion (region, 0, 2);
	add_subregion (region, 10, 15);
	add_subregion (region, 23, 25);
	check_result (region, "0-2 10-15 23-25");

	do_intersection_subregion (region, 0, 25, "0-2 10-15 23-25");
	do_intersection_subregion (region, 10, 15, "10-15");
	do_intersection_subregion (region, 8, 17, "10-15");
	do_intersection_subregion (region, 1, 24, "1-2 10-15 23-24");
	do_intersection_subregion (region, 3, 7, NULL);

	g_object_unref (buffer);
	g_object_unref (region);
}

static void
test_add_subtract_intersect_region (void)
{
	CtkTextBuffer *buffer;
	CtkSourceRegion *main_region = NULL;
	CtkSourceRegion *region_to_add = NULL;
	CtkSourceRegion *region_to_subtract = NULL;
	CtkSourceRegion *intersection = NULL;

	buffer = ctk_text_buffer_new (NULL);
	main_region = ctk_source_region_new (buffer);

	ctk_text_buffer_set_text (buffer, "This is a test of CtkSourceRegion", -1);

	g_assert_true (ctk_source_region_is_empty (main_region));

	/* Basic tests */

	region_to_add = ctk_source_region_new (buffer);
	add_subregion (region_to_add, 0, 5);
	add_subregion (region_to_add, 10, 15);
	check_result (region_to_add, "0-5 10-15");
	ctk_source_region_add_region (main_region, region_to_add);
	check_result (main_region, "0-5 10-15");

	region_to_subtract = ctk_source_region_new (buffer);
	add_subregion (region_to_subtract, 2, 3);
	add_subregion (region_to_subtract, 10, 15);
	ctk_source_region_subtract_region (main_region, region_to_subtract);
	check_result (main_region, "0-2 3-5");

	add_subregion (main_region, 20, 25);
	check_result (main_region, "0-2 3-5 20-25");
	check_result (region_to_add, "0-5 10-15");
	intersection = ctk_source_region_intersect_region (main_region, region_to_add);
	check_result (intersection, "0-2 3-5");

	g_object_unref (buffer);
	g_clear_object (&main_region);
	g_clear_object (&region_to_add);
	g_clear_object (&region_to_subtract);
	g_clear_object (&intersection);
}

int
main (int argc, char **argv)
{
	ctk_test_init (&argc, &argv);

	g_test_add_func ("/Region/weak-ref", test_weak_ref);
	g_test_add_func ("/Region/add-subtract-subregion", test_add_subtract_subregion);
	g_test_add_func ("/Region/intersect-subregion", test_intersect_subregion);
	g_test_add_func ("/Region/add-subtract-intersect-region", test_add_subtract_intersect_region);

	return g_test_run();
}
