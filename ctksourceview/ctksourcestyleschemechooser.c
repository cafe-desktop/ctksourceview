/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2014 - Ignacio Casal Quinteiro
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
 * along with CtkSourceView. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourcestyleschemechooser.h"
#include "ctksourcestylescheme.h"

/**
 * SECTION:styleschemechooser
 * @Short_description: Interface implemented by widgets for choosing style schemes
 * @Title: CtkSourceStyleSchemeChooser
 * @See_also: #CtkSourceStyleSchemeChooserWidget, #CtkSourceStyleSchemeChooserButton
 *
 * #CtkSourceStyleSchemeChooser is an interface that is implemented by widgets
 * for choosing style schemes.
 *
 * In CtkSourceView, the main widgets that implement this interface are
 * #CtkSourceStyleSchemeChooserWidget and #CtkSourceStyleSchemeChooserButton.
 *
 * Since: 3.16
 */

G_DEFINE_INTERFACE (CtkSourceStyleSchemeChooser, ctk_source_style_scheme_chooser, G_TYPE_OBJECT);

static void
ctk_source_style_scheme_chooser_default_init (CtkSourceStyleSchemeChooserInterface *iface)
{
	/**
	 * CtkSourceStyleSchemeChooser:style-scheme:
	 *
	 * The :style-scheme property contains the currently selected style
	 * scheme. The property can be set to change
	 * the current selection programmatically.
	 *
	 * Since: 3.16
	 */
	g_object_interface_install_property (iface,
		g_param_spec_object ("style-scheme",
		                     "Style Scheme",
		                     "Current style scheme",
		                     CTK_SOURCE_TYPE_STYLE_SCHEME,
		                     G_PARAM_READWRITE));
}

/**
 * ctk_source_style_scheme_chooser_get_style_scheme:
 * @chooser: a #CtkSourceStyleSchemeChooser
 *
 * Gets the currently-selected scheme.
 *
 * Returns: (transfer none): the currently-selected scheme.
 *
 * Since: 3.16
 */
CtkSourceStyleScheme *
ctk_source_style_scheme_chooser_get_style_scheme (CtkSourceStyleSchemeChooser *chooser)
{
	g_return_val_if_fail (CTK_SOURCE_IS_STYLE_SCHEME_CHOOSER (chooser), NULL);

	return CTK_SOURCE_STYLE_SCHEME_CHOOSER_GET_IFACE (chooser)->get_style_scheme (chooser);
}

/**
 * ctk_source_style_scheme_chooser_set_style_scheme:
 * @chooser: a #CtkSourceStyleSchemeChooser
 * @scheme: a #CtkSourceStyleScheme
 *
 * Sets the scheme.
 *
 * Since: 3.16
 */
void
ctk_source_style_scheme_chooser_set_style_scheme (CtkSourceStyleSchemeChooser *chooser,
                                                  CtkSourceStyleScheme        *scheme)
{
	g_return_if_fail (CTK_SOURCE_IS_STYLE_SCHEME_CHOOSER (chooser));
	g_return_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme));

	CTK_SOURCE_STYLE_SCHEME_CHOOSER_GET_IFACE (chooser)->set_style_scheme (chooser, scheme);
}
