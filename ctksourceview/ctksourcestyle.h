/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2003 - Paolo Maggi <paolo.maggi@polito.it>
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

#ifndef CTK_SOURCE_STYLE_H
#define CTK_SOURCE_STYLE_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_STYLE             (ctk_source_style_get_type ())
#define CTK_SOURCE_STYLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_STYLE, CtkSourceStyle))
#define CTK_SOURCE_IS_STYLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_STYLE))
#define CTK_SOURCE_STYLE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_STYLE, CtkSourceStyleClass))
#define CTK_SOURCE_IS_STYLE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_STYLE))
#define CTK_SOURCE_STYLE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_STYLE, CtkSourceStyleClass))

typedef struct _CtkSourceStyleClass CtkSourceStyleClass;

CTK_SOURCE_AVAILABLE_IN_ALL
GType		 ctk_source_style_get_type	(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceStyle	*ctk_source_style_copy		(const CtkSourceStyle *style);

CTK_SOURCE_AVAILABLE_IN_3_22
void		 ctk_source_style_apply		(const CtkSourceStyle *style,
						 CtkTextTag           *tag);

G_END_DECLS

#endif  /* CTK_SOURCE_STYLE_H */
