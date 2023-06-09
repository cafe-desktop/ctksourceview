/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2009 - Jesse van den Kieboom <jessevdk@gnome.org>
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

#ifndef CTK_SOURCE_COMPLETION_ITEM_H
#define CTK_SOURCE_COMPLETION_ITEM_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_COMPLETION_ITEM			(ctk_source_completion_item_get_type ())
#define CTK_SOURCE_COMPLETION_ITEM(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_ITEM, CtkSourceCompletionItem))
#define CTK_SOURCE_COMPLETION_ITEM_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_COMPLETION_ITEM, CtkSourceCompletionItemClass))
#define CTK_SOURCE_IS_COMPLETION_ITEM(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_COMPLETION_ITEM))
#define CTK_SOURCE_IS_COMPLETION_ITEM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_COMPLETION_ITEM))
#define CTK_SOURCE_COMPLETION_ITEM_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_COMPLETION_ITEM, CtkSourceCompletionItemClass))

typedef struct _CtkSourceCompletionItemClass	CtkSourceCompletionItemClass;
typedef struct _CtkSourceCompletionItemPrivate	CtkSourceCompletionItemPrivate;

struct _CtkSourceCompletionItem {
	GObject parent;

	CtkSourceCompletionItemPrivate *priv;
};

struct _CtkSourceCompletionItemClass {
	GObjectClass parent_class;

	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType 			 ctk_source_completion_item_get_type 		(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_4_0
CtkSourceCompletionItem *ctk_source_completion_item_new			(void);

CTK_SOURCE_AVAILABLE_IN_3_24
void			 ctk_source_completion_item_set_label		(CtkSourceCompletionItem *item,
									 const gchar             *label);

CTK_SOURCE_AVAILABLE_IN_3_24
void			 ctk_source_completion_item_set_markup		(CtkSourceCompletionItem *item,
									 const gchar             *markup);

CTK_SOURCE_AVAILABLE_IN_3_24
void			 ctk_source_completion_item_set_text		(CtkSourceCompletionItem *item,
									 const gchar             *text);

CTK_SOURCE_AVAILABLE_IN_3_24
void			 ctk_source_completion_item_set_icon		(CtkSourceCompletionItem *item,
									 GdkPixbuf               *icon);

CTK_SOURCE_AVAILABLE_IN_3_24
void			 ctk_source_completion_item_set_icon_name	(CtkSourceCompletionItem *item,
									 const gchar             *icon_name);

CTK_SOURCE_AVAILABLE_IN_3_24
void			 ctk_source_completion_item_set_gicon		(CtkSourceCompletionItem *item,
									 GIcon                   *gicon);

CTK_SOURCE_AVAILABLE_IN_3_24
void			 ctk_source_completion_item_set_info		(CtkSourceCompletionItem *item,
									 const gchar             *info);

G_END_DECLS

#endif /* CTK_SOURCE_COMPLETION_ITEM_H */
