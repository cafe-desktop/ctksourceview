/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
 * Copyright (C) 2014, 2015 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef CTK_SOURCE_ENCODING_H
#define CTK_SOURCE_ENCODING_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <glib.h>
#include <glib-object.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_ENCODING (ctk_source_encoding_get_type ())

CTK_SOURCE_AVAILABLE_IN_3_14
GType			 ctk_source_encoding_get_type			(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_3_14
const CtkSourceEncoding	*ctk_source_encoding_get_from_charset		(const gchar             *charset);

CTK_SOURCE_AVAILABLE_IN_3_14
gchar			*ctk_source_encoding_to_string			(const CtkSourceEncoding *enc);

CTK_SOURCE_AVAILABLE_IN_3_14
const gchar		*ctk_source_encoding_get_name			(const CtkSourceEncoding *enc);

CTK_SOURCE_AVAILABLE_IN_3_14
const gchar		*ctk_source_encoding_get_charset		(const CtkSourceEncoding *enc);

CTK_SOURCE_AVAILABLE_IN_3_14
const CtkSourceEncoding	*ctk_source_encoding_get_utf8			(void);

CTK_SOURCE_AVAILABLE_IN_3_14
const CtkSourceEncoding	*ctk_source_encoding_get_current		(void);

CTK_SOURCE_AVAILABLE_IN_3_14
GSList			*ctk_source_encoding_get_all			(void);

CTK_SOURCE_AVAILABLE_IN_3_18
GSList			*ctk_source_encoding_get_default_candidates	(void);

/* These should not be used, they are just to make python bindings happy */

CTK_SOURCE_AVAILABLE_IN_3_14
CtkSourceEncoding	*ctk_source_encoding_copy			(const CtkSourceEncoding *enc);

CTK_SOURCE_AVAILABLE_IN_3_14
void			 ctk_source_encoding_free			(CtkSourceEncoding       *enc);

G_END_DECLS

#endif  /* CTK_SOURCE_ENCODING_H */
