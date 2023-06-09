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

/* Interface for syntax highlighting engines. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourceengine.h"
#include "ctksourcestylescheme.h"

G_DEFINE_INTERFACE (CtkSourceEngine, _ctk_source_engine, G_TYPE_OBJECT)

static void
_ctk_source_engine_default_init (CtkSourceEngineInterface *interface)
{
}

void
_ctk_source_engine_attach_buffer (CtkSourceEngine *engine,
				  CtkTextBuffer   *buffer)
{
	g_return_if_fail (CTK_SOURCE_IS_ENGINE (engine));
	g_return_if_fail (CTK_SOURCE_ENGINE_GET_INTERFACE (engine)->attach_buffer != NULL);

	CTK_SOURCE_ENGINE_GET_INTERFACE (engine)->attach_buffer (engine, buffer);
}

void
_ctk_source_engine_text_inserted (CtkSourceEngine *engine,
				  gint             start_offset,
				  gint             end_offset)
{
	g_return_if_fail (CTK_SOURCE_IS_ENGINE (engine));
	g_return_if_fail (CTK_SOURCE_ENGINE_GET_INTERFACE (engine)->text_inserted != NULL);

	CTK_SOURCE_ENGINE_GET_INTERFACE (engine)->text_inserted (engine,
								 start_offset,
								 end_offset);
}

void
_ctk_source_engine_text_deleted (CtkSourceEngine *engine,
				 gint             offset,
				 gint             length)
{
	g_return_if_fail (CTK_SOURCE_IS_ENGINE (engine));
	g_return_if_fail (CTK_SOURCE_ENGINE_GET_INTERFACE (engine)->text_deleted != NULL);

	CTK_SOURCE_ENGINE_GET_INTERFACE (engine)->text_deleted (engine,
								offset,
								length);
}

void
_ctk_source_engine_update_highlight (CtkSourceEngine   *engine,
				     const CtkTextIter *start,
				     const CtkTextIter *end,
				     gboolean           synchronous)
{
	g_return_if_fail (CTK_SOURCE_IS_ENGINE (engine));
	g_return_if_fail (start != NULL && end != NULL);
	g_return_if_fail (CTK_SOURCE_ENGINE_GET_INTERFACE (engine)->update_highlight != NULL);

	CTK_SOURCE_ENGINE_GET_INTERFACE (engine)->update_highlight (engine,
								    start,
								    end,
								    synchronous);
}

void
_ctk_source_engine_set_style_scheme (CtkSourceEngine      *engine,
				     CtkSourceStyleScheme *scheme)
{
	g_return_if_fail (CTK_SOURCE_IS_ENGINE (engine));
	g_return_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme) || scheme == NULL);
	g_return_if_fail (CTK_SOURCE_ENGINE_GET_INTERFACE (engine)->set_style_scheme != NULL);

	CTK_SOURCE_ENGINE_GET_INTERFACE (engine)->set_style_scheme (engine, scheme);
}
