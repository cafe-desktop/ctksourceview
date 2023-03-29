/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2003-2007 - Paolo Maggi <paolo.maggi@polito.it>
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

#ifndef CTK_SOURCE_STYLE_SCHEME_MANAGER_H
#define CTK_SOURCE_STYLE_SCHEME_MANAGER_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <glib-object.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_STYLE_SCHEME_MANAGER		(ctk_source_style_scheme_manager_get_type ())
#define CTK_SOURCE_STYLE_SCHEME_MANAGER(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), CTK_SOURCE_TYPE_STYLE_SCHEME_MANAGER, CtkSourceStyleSchemeManager))
#define CTK_SOURCE_STYLE_SCHEME_MANAGER_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), CTK_SOURCE_TYPE_STYLE_SCHEME_MANAGER, CtkSourceStyleSchemeManagerClass))
#define CTK_SOURCE_IS_STYLE_SCHEME_MANAGER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), CTK_SOURCE_TYPE_STYLE_SCHEME_MANAGER))
#define CTK_SOURCE_IS_STYLE_SCHEME_MANAGER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_STYLE_SCHEME_MANAGER))
#define CTK_SOURCE_STYLE_SCHEME_MANAGER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj), CTK_SOURCE_TYPE_STYLE_SCHEME_MANAGER, CtkSourceStyleSchemeManagerClass))

typedef struct _CtkSourceStyleSchemeManagerClass	CtkSourceStyleSchemeManagerClass;
typedef struct _CtkSourceStyleSchemeManagerPrivate	CtkSourceStyleSchemeManagerPrivate;

struct _CtkSourceStyleSchemeManager
{
	GObject parent;

	CtkSourceStyleSchemeManagerPrivate *priv;
};

struct _CtkSourceStyleSchemeManagerClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType			 ctk_source_style_scheme_manager_get_type		(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceStyleSchemeManager *
			 ctk_source_style_scheme_manager_new			(void);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceStyleSchemeManager *
			 ctk_source_style_scheme_manager_get_default		(void);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_style_scheme_manager_set_search_path	(CtkSourceStyleSchemeManager	*manager,
						    				 gchar			       **path);

CTK_SOURCE_AVAILABLE_IN_ALL
void 			 ctk_source_style_scheme_manager_append_search_path	(CtkSourceStyleSchemeManager	*manager,
						    				 const gchar			*path);

CTK_SOURCE_AVAILABLE_IN_ALL
void 			 ctk_source_style_scheme_manager_prepend_search_path	(CtkSourceStyleSchemeManager	*manager,
						    				 const gchar			*path);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar * const *	 ctk_source_style_scheme_manager_get_search_path	(CtkSourceStyleSchemeManager	*manager);

CTK_SOURCE_AVAILABLE_IN_ALL
void			 ctk_source_style_scheme_manager_force_rescan		(CtkSourceStyleSchemeManager	*manager);

CTK_SOURCE_AVAILABLE_IN_ALL
const gchar * const *	 ctk_source_style_scheme_manager_get_scheme_ids		(CtkSourceStyleSchemeManager	*manager);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceStyleScheme	*ctk_source_style_scheme_manager_get_scheme		(CtkSourceStyleSchemeManager	*manager,
										 const gchar			*scheme_id);

G_GNUC_INTERNAL
CtkSourceStyleSchemeManager *
			 _ctk_source_style_scheme_manager_peek_default		(void);

G_END_DECLS

#endif /* CTK_SOURCE_STYLE_SCHEME_MANAGER_H */
