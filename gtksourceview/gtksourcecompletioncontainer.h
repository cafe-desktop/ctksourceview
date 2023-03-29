/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2013 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef CTK_SOURCE_COMPLETION_CONTAINER_H
#define CTK_SOURCE_COMPLETION_CONTAINER_H

#include <ctk/ctk.h>
#include "ctksourcetypes-private.h"

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_COMPLETION_CONTAINER             (_ctk_source_completion_container_get_type ())
#define CTK_SOURCE_COMPLETION_CONTAINER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_CONTAINER, CtkSourceCompletionContainer))
#define CTK_SOURCE_COMPLETION_CONTAINER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_COMPLETION_CONTAINER, CtkSourceCompletionContainerClass)
#define CTK_SOURCE_IS_COMPLETION_CONTAINER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_COMPLETION_CONTAINER))
#define CTK_SOURCE_IS_COMPLETION_CONTAINER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_COMPLETION_CONTAINER))
#define CTK_SOURCE_COMPLETION_CONTAINER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_COMPLETION_CONTAINER, CtkSourceCompletionContainerClass))

typedef struct _CtkSourceCompletionContainerClass	CtkSourceCompletionContainerClass;

struct _CtkSourceCompletionContainer
{
	CtkScrolledWindow parent;
};

struct _CtkSourceCompletionContainerClass
{
	CtkScrolledWindowClass parent_class;
};

G_GNUC_INTERNAL
GType		 _ctk_source_completion_container_get_type		(void) G_GNUC_CONST;

G_GNUC_INTERNAL
CtkSourceCompletionContainer *
		 _ctk_source_completion_container_new			(void);

G_END_DECLS

#endif /* CTK_SOURCE_COMPLETION_CONTAINER_H */
