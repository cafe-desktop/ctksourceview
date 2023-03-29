/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2007 - Johannes Schmid <jhs@gnome.org>
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

#ifndef CTKSOURCEMARK_H
#define CTKSOURCEMARK_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_MARK             (ctk_source_mark_get_type ())
#define CTK_SOURCE_MARK(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_MARK, GtkSourceMark))
#define CTK_SOURCE_MARK_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_MARK, GtkSourceMarkClass))
#define CTK_SOURCE_IS_MARK(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_MARK))
#define CTK_SOURCE_IS_MARK_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_MARK))
#define CTK_SOURCE_MARK_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_MARK, GtkSourceMarkClass))

typedef struct _GtkSourceMarkClass GtkSourceMarkClass;

typedef struct _GtkSourceMarkPrivate GtkSourceMarkPrivate;

struct _GtkSourceMark
{
	GtkTextMark parent_instance;

	GtkSourceMarkPrivate *priv;
};

struct _GtkSourceMarkClass
{
	GtkTextMarkClass parent_class;

	/* Padding for future expansion */
	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType		 ctk_source_mark_get_type (void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
GtkSourceMark   *ctk_source_mark_new		(const gchar	*name,
						 const gchar	*category);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar	*ctk_source_mark_get_category	(GtkSourceMark	*mark);

CTK_SOURCE_AVAILABLE_IN_ALL
GtkSourceMark	*ctk_source_mark_next		(GtkSourceMark	*mark,
						 const gchar	*category);

CTK_SOURCE_AVAILABLE_IN_ALL
GtkSourceMark	*ctk_source_mark_prev		(GtkSourceMark	*mark,
						 const gchar	*category);

G_END_DECLS

#endif /* CTKSOURCEMARK_H */
