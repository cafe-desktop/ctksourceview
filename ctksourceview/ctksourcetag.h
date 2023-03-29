/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2015 - Université Catholique de Louvain
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
 *
 * Author: Sébastien Wilmet
 */

#ifndef CTK_SOURCE_TAG_H
#define CTK_SOURCE_TAG_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_TAG (ctk_source_tag_get_type ())

CTK_SOURCE_AVAILABLE_IN_3_20
G_DECLARE_DERIVABLE_TYPE (CtkSourceTag, ctk_source_tag,
			  CTK_SOURCE, TAG,
			  CtkTextTag)

struct _CtkSourceTagClass
{
	CtkTextTagClass parent_class;

	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_3_20
CtkTextTag *	ctk_source_tag_new		(const gchar *name);

G_END_DECLS

#endif /* CTK_SOURCE_TAG_H */
