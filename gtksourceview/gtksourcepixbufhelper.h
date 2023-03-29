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

#ifndef CTK_SOURCE_PIXBUF_HELPER_H
#define CTK_SOURCE_PIXBUF_HELPER_H

#include <ctk/ctk.h>
#include "ctksourcetypes-private.h"

G_GNUC_INTERNAL
CtkSourcePixbufHelper *ctk_source_pixbuf_helper_new (void);

G_GNUC_INTERNAL
void ctk_source_pixbuf_helper_free (CtkSourcePixbufHelper *helper);

G_GNUC_INTERNAL
void ctk_source_pixbuf_helper_set_pixbuf (CtkSourcePixbufHelper *helper,
                                          const GdkPixbuf       *pixbuf);

G_GNUC_INTERNAL
GdkPixbuf *ctk_source_pixbuf_helper_get_pixbuf (CtkSourcePixbufHelper *helper);

G_GNUC_INTERNAL
void ctk_source_pixbuf_helper_set_icon_name (CtkSourcePixbufHelper *helper,
                                             const gchar           *icon_name);

G_GNUC_INTERNAL
const gchar *ctk_source_pixbuf_helper_get_icon_name (CtkSourcePixbufHelper *helper);

G_GNUC_INTERNAL
void ctk_source_pixbuf_helper_set_gicon (CtkSourcePixbufHelper *helper,
                                         GIcon                 *gicon);

G_GNUC_INTERNAL
GIcon *ctk_source_pixbuf_helper_get_gicon (CtkSourcePixbufHelper *helper);

G_GNUC_INTERNAL
GdkPixbuf *ctk_source_pixbuf_helper_render (CtkSourcePixbufHelper *helper,
                                            CtkWidget             *widget,
                                            gint                   size);

#endif /* CTK_SOURCE_PIXBUF_HELPER_H */

