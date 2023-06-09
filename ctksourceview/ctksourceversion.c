/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2015 - Paolo Borelli <pborelli@gnome.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourceversion.h"

/**
 * SECTION:version
 * @Short_description: Macros and functions to check the CtkSourceView version
 * @Title: Version Information
 */

/**
 * ctk_source_get_major_version:
 *
 * Returns the major version number of the CtkSourceView library.
 * (e.g. in CtkSourceView version 3.20.0 this is 3.)
 *
 * This function is in the library, so it represents the CtkSourceView library
 * your code is running against. Contrast with the #CTK_SOURCE_MAJOR_VERSION
 * macro, which represents the major version of the CtkSourceView headers you
 * have included when compiling your code.
 *
 * Returns: the major version number of the CtkSourceView library
 *
 * Since: 3.20
 */
guint
ctk_source_get_major_version (void)
{
	return CTK_SOURCE_MAJOR_VERSION;
}

/**
 * ctk_source_get_minor_version:
 *
 * Returns the minor version number of the CtkSourceView library.
 * (e.g. in CtkSourceView version 3.20.0 this is 20.)
 *
 * This function is in the library, so it represents the CtkSourceView library
 * your code is running against. Contrast with the #CTK_SOURCE_MINOR_VERSION
 * macro, which represents the minor version of the CtkSourceView headers you
 * have included when compiling your code.
 *
 * Returns: the minor version number of the CtkSourceView library
 *
 * Since: 3.20
 */
guint
ctk_source_get_minor_version (void)
{
	return CTK_SOURCE_MINOR_VERSION;
}

/**
 * ctk_source_get_micro_version:
 *
 * Returns the micro version number of the CtkSourceView library.
 * (e.g. in CtkSourceView version 3.20.0 this is 0.)
 *
 * This function is in the library, so it represents the CtkSourceView library
 * your code is running against. Contrast with the #CTK_SOURCE_MICRO_VERSION
 * macro, which represents the micro version of the CtkSourceView headers you
 * have included when compiling your code.
 *
 * Returns: the micro version number of the CtkSourceView library
 *
 * Since: 3.20
 */
guint
ctk_source_get_micro_version (void)
{
	return CTK_SOURCE_MICRO_VERSION;
}

/**
 * ctk_source_check_version:
 * @major: the major version to check
 * @minor: the minor version to check
 * @micro: the micro version to check
 *
 * Like CTK_SOURCE_CHECK_VERSION, but the check for ctk_source_check_version is
 * at runtime instead of compile time. This is useful for compiling
 * against older versions of CtkSourceView, but using features from newer
 * versions.
 *
 * Returns: %TRUE if the version of the CtkSourceView currently loaded
 * is the same as or newer than the passed-in version.
 *
 * Since: 3.20
 */
gboolean
ctk_source_check_version (guint major,
                          guint minor,
                          guint micro)
{
	return CTK_SOURCE_CHECK_VERSION (major, (int)minor, micro);
}
