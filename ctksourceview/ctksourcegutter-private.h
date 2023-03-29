/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- *
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2009 - Jesse van den Kieboom
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

#ifndef CTK_SOURCE_GUTTER_PRIVATE_H
#define CTK_SOURCE_GUTTER_PRIVATE_H

#include <ctk/ctk.h>
#include "ctksourcetypes.h"

G_BEGIN_DECLS

G_GNUC_INTERNAL
CtkSourceGutter *	_ctk_source_gutter_new		(CtkSourceView     *view,
							 CtkTextWindowType  type);

G_GNUC_INTERNAL
void			_ctk_source_gutter_draw		(CtkSourceGutter *gutter,
							 CtkSourceView   *view,
							 cairo_t         *cr);


G_END_DECLS

#endif /* CTK_SOURCE_GUTTER_PRIVATE_H */
