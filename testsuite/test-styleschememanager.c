/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2013 - Paolo Borelli
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
test_get_default (void)
{
	CtkSourceStyleSchemeManager *sm1, *sm2;

	sm1 = ctk_source_style_scheme_manager_get_default ();
	sm2 = ctk_source_style_scheme_manager_get_default ();
	g_assert_true (sm1 == sm2);
}

static void
test_prepend_search_path (void)
{
	CtkSourceStyleSchemeManager *sm;
	gchar *style_dir;
	CtkSourceStyleScheme *scheme;
	const gchar *fname;
	gchar *expected;

	sm = ctk_source_style_scheme_manager_get_default ();

	style_dir = g_test_build_filename (G_TEST_DIST, "styles", NULL);
	ctk_source_style_scheme_manager_prepend_search_path (sm, style_dir);

	scheme = ctk_source_style_scheme_manager_get_scheme (sm, "classic");
	fname = ctk_source_style_scheme_get_filename (scheme);
	expected = g_build_filename (style_dir, "classic.xml", NULL);
	g_assert_cmpstr (fname, ==, expected);

	g_free (expected);
	g_free (style_dir);
}

int
main (int argc, char** argv)
{
	ctk_test_init (&argc, &argv);

	g_test_add_func ("/StyleSchemeManager/get-default", test_get_default);
	g_test_add_func ("/StyleSchemeManager/prepend-search-path", test_prepend_search_path);

	return g_test_run();
}
