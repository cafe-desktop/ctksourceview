/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2010 - Jesse van den Kieboom
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

#ifndef CTK_SOURCE_GUTTER_RENDERER_MARKS_H
#define CTK_SOURCE_GUTTER_RENDERER_MARKS_H

#include <ctk/ctk.h>
#include "ctksourcetypes.h"
#include "ctksourcetypes-private.h"
#include "ctksourcegutterrendererpixbuf.h"

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_GUTTER_RENDERER_MARKS		(ctk_source_gutter_renderer_marks_get_type ())
#define CTK_SOURCE_GUTTER_RENDERER_MARKS(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER_MARKS, CtkSourceGutterRendererMarks))
#define CTK_SOURCE_GUTTER_RENDERER_MARKS_CONST(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER_MARKS, CtkSourceGutterRendererMarks const))
#define CTK_SOURCE_GUTTER_RENDERER_MARKS_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_GUTTER_RENDERER_MARKS, CtkSourceGutterRendererMarksClass))
#define CTK_SOURCE_IS_GUTTER_RENDERER_MARKS(obj)	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER_MARKS))
#define CTK_SOURCE_IS_GUTTER_RENDERER_MARKS_CLASS(klass)(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_GUTTER_RENDERER_MARKS))
#define CTK_SOURCE_GUTTER_RENDERER_MARKS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER_MARKS, CtkSourceGutterRendererMarksClass))

typedef struct _CtkSourceGutterRendererMarksClass	CtkSourceGutterRendererMarksClass;

struct _CtkSourceGutterRendererMarks
{
	CtkSourceGutterRendererPixbuf parent;
};

struct _CtkSourceGutterRendererMarksClass
{
	CtkSourceGutterRendererPixbufClass parent_class;
};

G_GNUC_INTERNAL
GType ctk_source_gutter_renderer_marks_get_type (void) G_GNUC_CONST;

G_GNUC_INTERNAL
CtkSourceGutterRenderer *ctk_source_gutter_renderer_marks_new (void);

G_END_DECLS

#endif /* CTK_SOURCE_GUTTER_RENDERER_MARKS_H */
