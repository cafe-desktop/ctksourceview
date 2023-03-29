/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef CTK_SOURCE_BUFFER_INTERNAL_H
#define CTK_SOURCE_BUFFER_INTERNAL_H

#include <glib-object.h>
#include "ctksourcetypes.h"
#include "ctksourcetypes-private.h"

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_BUFFER_INTERNAL (_ctk_source_buffer_internal_get_type ())

G_GNUC_INTERNAL
G_DECLARE_FINAL_TYPE (CtkSourceBufferInternal, _ctk_source_buffer_internal,
		      CTK_SOURCE, BUFFER_INTERNAL,
		      GObject)

G_GNUC_INTERNAL
CtkSourceBufferInternal *
		_ctk_source_buffer_internal_get_from_buffer		(CtkSourceBuffer *buffer);

G_GNUC_INTERNAL
void		_ctk_source_buffer_internal_emit_search_start		(CtkSourceBufferInternal *buffer_internal,
									 CtkSourceSearchContext  *search_context);

G_END_DECLS

#endif /* CTK_SOURCE_BUFFER_INTERNAL_H */
