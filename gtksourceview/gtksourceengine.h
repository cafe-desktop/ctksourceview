/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2003 - Gustavo Giráldez
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

#ifndef CTK_SOURCE_ENGINE_H
#define CTK_SOURCE_ENGINE_H

#include <ctk/ctk.h>
#include "ctksourcetypes.h"
#include "ctksourcetypes-private.h"

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_ENGINE               (_ctk_source_engine_get_type ())
#define CTK_SOURCE_ENGINE(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_ENGINE, CtkSourceEngine))
#define CTK_SOURCE_IS_ENGINE(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_ENGINE))
#define CTK_SOURCE_ENGINE_GET_INTERFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), CTK_SOURCE_TYPE_ENGINE, CtkSourceEngineInterface))

typedef struct _CtkSourceEngineInterface CtkSourceEngineInterface;

struct _CtkSourceEngineInterface
{
	GTypeInterface parent_interface;

	void     (* attach_buffer)    (CtkSourceEngine      *engine,
				       CtkTextBuffer        *buffer);

	void     (* text_inserted)    (CtkSourceEngine      *engine,
				       gint                  start_offset,
				       gint                  end_offset);
	void     (* text_deleted)     (CtkSourceEngine      *engine,
				       gint                  offset,
				       gint                  length);

	void     (* update_highlight) (CtkSourceEngine      *engine,
				       const CtkTextIter    *start,
				       const CtkTextIter    *end,
				       gboolean              synchronous);

	void     (* set_style_scheme) (CtkSourceEngine      *engine,
				       CtkSourceStyleScheme *scheme);
};

G_GNUC_INTERNAL
GType       _ctk_source_engine_get_type		(void) G_GNUC_CONST;

G_GNUC_INTERNAL
void        _ctk_source_engine_attach_buffer	(CtkSourceEngine      *engine,
						 CtkTextBuffer        *buffer);

G_GNUC_INTERNAL
void        _ctk_source_engine_text_inserted	(CtkSourceEngine      *engine,
						 gint                  start_offset,
						 gint                  end_offset);

G_GNUC_INTERNAL
void        _ctk_source_engine_text_deleted	(CtkSourceEngine      *engine,
						 gint                  offset,
						 gint                  length);

G_GNUC_INTERNAL
void        _ctk_source_engine_update_highlight	(CtkSourceEngine      *engine,
						 const CtkTextIter    *start,
						 const CtkTextIter    *end,
						 gboolean              synchronous);

G_GNUC_INTERNAL
void        _ctk_source_engine_set_style_scheme	(CtkSourceEngine      *engine,
						 CtkSourceStyleScheme *scheme);

G_END_DECLS

#endif /* CTK_SOURCE_ENGINE_H */
