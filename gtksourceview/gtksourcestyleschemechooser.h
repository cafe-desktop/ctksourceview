/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2014 - Ignacio Casal Quinteiro
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
 * along with GtkSourceView. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CTK_SOURCE_STYLE_SCHEME_CHOOSER_H
#define CTK_SOURCE_STYLE_SCHEME_CHOOSER_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <glib-object.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_STYLE_SCHEME_CHOOSER                  (ctk_source_style_scheme_chooser_get_type ())
#define CTK_SOURCE_STYLE_SCHEME_CHOOSER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_STYLE_SCHEME_CHOOSER, GtkSourceStyleSchemeChooser))
#define CTK_SOURCE_IS_STYLE_SCHEME_CHOOSER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_STYLE_SCHEME_CHOOSER))
#define CTK_SOURCE_STYLE_SCHEME_CHOOSER_GET_IFACE(inst)       (G_TYPE_INSTANCE_GET_INTERFACE ((inst), CTK_SOURCE_TYPE_STYLE_SCHEME_CHOOSER, GtkSourceStyleSchemeChooserInterface))

typedef struct _GtkSourceStyleSchemeChooserInterface GtkSourceStyleSchemeChooserInterface;

struct _GtkSourceStyleSchemeChooserInterface
{
	GTypeInterface base_interface;

	/* Methods */
	GtkSourceStyleScheme * (* get_style_scheme)       (GtkSourceStyleSchemeChooser *chooser);

	void                   (* set_style_scheme)       (GtkSourceStyleSchemeChooser *chooser,
	                                                   GtkSourceStyleScheme        *scheme);

	/* Padding */
	gpointer padding[12];
};

CTK_SOURCE_AVAILABLE_IN_3_16
GType                     ctk_source_style_scheme_chooser_get_type               (void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_3_16
GtkSourceStyleScheme     *ctk_source_style_scheme_chooser_get_style_scheme       (GtkSourceStyleSchemeChooser *chooser);

CTK_SOURCE_AVAILABLE_IN_3_16
void                      ctk_source_style_scheme_chooser_set_style_scheme       (GtkSourceStyleSchemeChooser *chooser,
                                                                                  GtkSourceStyleScheme        *scheme);

G_END_DECLS

#endif /* CTK_SOURCE_STYLE_SCHEME_CHOOSER_H */
