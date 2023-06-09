/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#define NUM_LOCATIONS (3)
#define LEADING_INDEX (0)
#define INSIDE_TEXT_INDEX (1)
#define TRAILING_INDEX (2)

static gboolean
is_zero_matrix (CtkSourceSpaceTypeFlags *matrix)
{
	gint i;

	for (i = 0; i < NUM_LOCATIONS; i++)
	{
		if (matrix[i] != 0)
		{
			return FALSE;
		}
	}

	return TRUE;
}

static GVariant *
create_variant_from_matrix (CtkSourceSpaceTypeFlags *matrix)
{
	GVariantBuilder builder;
	gint i;

	if (is_zero_matrix (matrix))
	{
		return g_variant_new ("au", NULL);
	}

	g_variant_builder_init (&builder, G_VARIANT_TYPE ("au"));

	for (i = 0; i < NUM_LOCATIONS; i++)
	{
		GVariant *types;

		types = g_variant_new_uint32 (matrix[i]);

		g_variant_builder_add_value (&builder, types);
	}

	return g_variant_builder_end (&builder);
}

static void
check_equal_matrix (CtkSourceSpaceDrawer    *drawer,
		    CtkSourceSpaceTypeFlags *matrix)
{
	CtkSourceSpaceTypeFlags types;
	GVariant *my_variant;
	GVariant *drawer_variant;

	types = ctk_source_space_drawer_get_types_for_locations (drawer, CTK_SOURCE_SPACE_LOCATION_LEADING);
	g_assert_cmpint (types, ==, matrix[LEADING_INDEX]);

	types = ctk_source_space_drawer_get_types_for_locations (drawer, CTK_SOURCE_SPACE_LOCATION_INSIDE_TEXT);
	g_assert_cmpint (types, ==, matrix[INSIDE_TEXT_INDEX]);

	types = ctk_source_space_drawer_get_types_for_locations (drawer, CTK_SOURCE_SPACE_LOCATION_TRAILING);
	g_assert_cmpint (types, ==, matrix[TRAILING_INDEX]);

	/* Check variants */
	my_variant = create_variant_from_matrix (matrix);
	drawer_variant = ctk_source_space_drawer_get_matrix (drawer);
	g_assert_true (g_variant_equal (my_variant, drawer_variant));

	ctk_source_space_drawer_set_types_for_locations (drawer,
							 CTK_SOURCE_SPACE_LOCATION_ALL,
							 CTK_SOURCE_SPACE_TYPE_NONE);
	ctk_source_space_drawer_set_matrix (drawer, my_variant);
	g_variant_ref_sink (drawer_variant);
	g_variant_unref (drawer_variant);

	my_variant = create_variant_from_matrix (matrix);
	drawer_variant = ctk_source_space_drawer_get_matrix (drawer);
	g_assert_true (g_variant_equal (my_variant, drawer_variant));
	g_variant_ref_sink (my_variant);
	g_variant_unref (my_variant);
	g_variant_ref_sink (drawer_variant);
	g_variant_unref (drawer_variant);

	types = ctk_source_space_drawer_get_types_for_locations (drawer, CTK_SOURCE_SPACE_LOCATION_LEADING);
	g_assert_cmpint (types, ==, matrix[LEADING_INDEX]);

	types = ctk_source_space_drawer_get_types_for_locations (drawer, CTK_SOURCE_SPACE_LOCATION_INSIDE_TEXT);
	g_assert_cmpint (types, ==, matrix[INSIDE_TEXT_INDEX]);

	types = ctk_source_space_drawer_get_types_for_locations (drawer, CTK_SOURCE_SPACE_LOCATION_TRAILING);
	g_assert_cmpint (types, ==, matrix[TRAILING_INDEX]);
}

static void
set_matrix (CtkSourceSpaceDrawer    *drawer,
	    CtkSourceSpaceTypeFlags *matrix)
{
	CtkSourceSpaceTypeFlags types;

	/* Leading */
	ctk_source_space_drawer_set_types_for_locations (drawer,
							 CTK_SOURCE_SPACE_LOCATION_LEADING,
							 matrix[LEADING_INDEX]);

	types = ctk_source_space_drawer_get_types_for_locations (drawer, CTK_SOURCE_SPACE_LOCATION_LEADING);
	g_assert_cmpint (types, ==, matrix[LEADING_INDEX]);

	/* Inside text */
	ctk_source_space_drawer_set_types_for_locations (drawer,
							 CTK_SOURCE_SPACE_LOCATION_INSIDE_TEXT,
							 matrix[INSIDE_TEXT_INDEX]);

	types = ctk_source_space_drawer_get_types_for_locations (drawer, CTK_SOURCE_SPACE_LOCATION_INSIDE_TEXT);
	g_assert_cmpint (types, ==, matrix[INSIDE_TEXT_INDEX]);

	/* Trailing */
	ctk_source_space_drawer_set_types_for_locations (drawer,
							 CTK_SOURCE_SPACE_LOCATION_TRAILING,
							 matrix[TRAILING_INDEX]);

	types = ctk_source_space_drawer_get_types_for_locations (drawer, CTK_SOURCE_SPACE_LOCATION_TRAILING);
	g_assert_cmpint (types, ==, matrix[TRAILING_INDEX]);

	/* Check all */
	check_equal_matrix (drawer, matrix);
}

/* For a matrix, the getters and setters are less trivial so it's better to test
 * them.
 */
static void
test_matrix_getters_setters (void)
{
	CtkSourceView *view;
	CtkSourceSpaceDrawer *drawer;
	CtkSourceSpaceTypeFlags matrix[NUM_LOCATIONS];
	CtkSourceSpaceTypeFlags types;

	view = CTK_SOURCE_VIEW (ctk_source_view_new ());
	g_object_ref_sink (view);

	drawer = ctk_source_view_get_space_drawer (view);

	/* Default value */
	matrix[LEADING_INDEX] = CTK_SOURCE_SPACE_TYPE_ALL;
	matrix[INSIDE_TEXT_INDEX] = CTK_SOURCE_SPACE_TYPE_ALL;
	matrix[TRAILING_INDEX] = CTK_SOURCE_SPACE_TYPE_ALL;
	check_equal_matrix (drawer, matrix);

	/* Set each location separately */
	set_matrix (drawer, matrix);

	matrix[INSIDE_TEXT_INDEX] = 0;
	set_matrix (drawer, matrix);

	matrix[TRAILING_INDEX] = CTK_SOURCE_SPACE_TYPE_NBSP;
	set_matrix (drawer, matrix);

	/* Reset to 0 all at once */
	ctk_source_space_drawer_set_types_for_locations (drawer,
							 CTK_SOURCE_SPACE_LOCATION_ALL,
							 CTK_SOURCE_SPACE_TYPE_NONE);

	matrix[LEADING_INDEX] = 0;
	matrix[INSIDE_TEXT_INDEX] = 0;
	matrix[TRAILING_INDEX] = 0;
	check_equal_matrix (drawer, matrix);

	/* Set leading and trailing at once */
	ctk_source_space_drawer_set_types_for_locations (drawer,
							 CTK_SOURCE_SPACE_LOCATION_LEADING |
							 CTK_SOURCE_SPACE_LOCATION_TRAILING,
							 CTK_SOURCE_SPACE_TYPE_TAB);

	matrix[LEADING_INDEX] = CTK_SOURCE_SPACE_TYPE_TAB;
	matrix[TRAILING_INDEX] = CTK_SOURCE_SPACE_TYPE_TAB;
	check_equal_matrix (drawer, matrix);

	/* Enable all at once */
	ctk_source_space_drawer_set_types_for_locations (drawer,
							 CTK_SOURCE_SPACE_LOCATION_ALL,
							 CTK_SOURCE_SPACE_TYPE_ALL);

	matrix[LEADING_INDEX] = CTK_SOURCE_SPACE_TYPE_ALL;
	matrix[INSIDE_TEXT_INDEX] = CTK_SOURCE_SPACE_TYPE_ALL;
	matrix[TRAILING_INDEX] = CTK_SOURCE_SPACE_TYPE_ALL;
	check_equal_matrix (drawer, matrix);

	/* Get several locations at once */
	matrix[LEADING_INDEX] = CTK_SOURCE_SPACE_TYPE_NBSP | CTK_SOURCE_SPACE_TYPE_TAB;
	matrix[INSIDE_TEXT_INDEX] = CTK_SOURCE_SPACE_TYPE_NBSP;
	matrix[TRAILING_INDEX] = CTK_SOURCE_SPACE_TYPE_ALL;
	set_matrix (drawer, matrix);

	types = ctk_source_space_drawer_get_types_for_locations (drawer, CTK_SOURCE_SPACE_LOCATION_ALL);
	g_assert_cmpint (types, ==, CTK_SOURCE_SPACE_TYPE_NBSP);

	types = ctk_source_space_drawer_get_types_for_locations (drawer,
								 CTK_SOURCE_SPACE_LOCATION_LEADING |
								 CTK_SOURCE_SPACE_LOCATION_TRAILING);
	g_assert_cmpint (types, ==, CTK_SOURCE_SPACE_TYPE_NBSP | CTK_SOURCE_SPACE_TYPE_TAB);

	/* Set at none locations */
	ctk_source_space_drawer_set_types_for_locations (drawer,
							 CTK_SOURCE_SPACE_LOCATION_NONE,
							 CTK_SOURCE_SPACE_TYPE_ALL);
	check_equal_matrix (drawer, matrix);

	/* Get at none locations */
	types = ctk_source_space_drawer_get_types_for_locations (drawer, CTK_SOURCE_SPACE_LOCATION_NONE);
	g_assert_cmpint (types, ==, 0);

	g_object_unref (view);
}

int
main (int argc, char **argv)
{
	ctk_test_init (&argc, &argv);

	g_test_add_func ("/SpaceDrawer/matrix-getters-setters", test_matrix_getters_setters);

	return g_test_run();
}
