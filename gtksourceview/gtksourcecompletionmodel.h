/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2009 - Jesse van den Kieboom <jessevdk@gnome.org>
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

#ifndef CTK_SOURCE_COMPLETION_MODEL_H
#define CTK_SOURCE_COMPLETION_MODEL_H

#include <ctk/ctk.h>
#include "ctksourcetypes.h"
#include "ctksourcetypes-private.h"

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_COMPLETION_MODEL		(ctk_source_completion_model_get_type ())
#define CTK_SOURCE_COMPLETION_MODEL(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_MODEL, CtkSourceCompletionModel))
#define CTK_SOURCE_COMPLETION_MODEL_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_MODEL, CtkSourceCompletionModel const))
#define CTK_SOURCE_COMPLETION_MODEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_COMPLETION_MODEL, CtkSourceCompletionModelClass))
#define CTK_SOURCE_IS_COMPLETION_MODEL(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_COMPLETION_MODEL))
#define CTK_SOURCE_IS_COMPLETION_MODEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_COMPLETION_MODEL))
#define CTK_SOURCE_COMPLETION_MODEL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_COMPLETION_MODEL, CtkSourceCompletionModelClass))

typedef struct _CtkSourceCompletionModelClass	CtkSourceCompletionModelClass;
typedef struct _CtkSourceCompletionModelPrivate	CtkSourceCompletionModelPrivate;

struct _CtkSourceCompletionModel {
	GObject parent;

	CtkSourceCompletionModelPrivate *priv;
};

struct _CtkSourceCompletionModelClass {
	GObjectClass parent_class;

	void (*providers_changed) 	(CtkSourceCompletionModel *model);
};

enum
{
	CTK_SOURCE_COMPLETION_MODEL_COLUMN_MARKUP,
	CTK_SOURCE_COMPLETION_MODEL_COLUMN_ICON,
	CTK_SOURCE_COMPLETION_MODEL_COLUMN_ICON_NAME,
	CTK_SOURCE_COMPLETION_MODEL_COLUMN_GICON,
	CTK_SOURCE_COMPLETION_MODEL_COLUMN_PROPOSAL,
	CTK_SOURCE_COMPLETION_MODEL_COLUMN_PROVIDER,
	CTK_SOURCE_COMPLETION_MODEL_COLUMN_IS_HEADER,
	CTK_SOURCE_COMPLETION_MODEL_N_COLUMNS
};

CTK_SOURCE_INTERNAL
GType    ctk_source_completion_model_get_type			(void) G_GNUC_CONST;

CTK_SOURCE_INTERNAL
CtkSourceCompletionModel *
         ctk_source_completion_model_new			(void);

CTK_SOURCE_INTERNAL
void     ctk_source_completion_model_add_proposals              (CtkSourceCompletionModel    *model,
								 CtkSourceCompletionProvider *provider,
								 GList                       *proposals);

CTK_SOURCE_INTERNAL
gboolean ctk_source_completion_model_is_empty			(CtkSourceCompletionModel    *model,
								 gboolean                     only_visible);

CTK_SOURCE_INTERNAL
void     ctk_source_completion_model_set_visible_providers	(CtkSourceCompletionModel    *model,
								 GList                       *providers);

CTK_SOURCE_INTERNAL
GList   *ctk_source_completion_model_get_visible_providers	(CtkSourceCompletionModel    *model);

CTK_SOURCE_INTERNAL
GList   *ctk_source_completion_model_get_providers		(CtkSourceCompletionModel    *model);

CTK_SOURCE_INTERNAL
void     ctk_source_completion_model_set_show_headers		(CtkSourceCompletionModel    *model,
								 gboolean                     show_headers);

CTK_SOURCE_INTERNAL
gboolean ctk_source_completion_model_iter_is_header		(CtkSourceCompletionModel    *model,
								 CtkTreeIter                 *iter);

CTK_SOURCE_INTERNAL
gboolean ctk_source_completion_model_iter_previous		(CtkSourceCompletionModel    *model,
								 CtkTreeIter                 *iter);

CTK_SOURCE_INTERNAL
gboolean ctk_source_completion_model_first_proposal             (CtkSourceCompletionModel    *model,
								 CtkTreeIter                 *iter);

CTK_SOURCE_INTERNAL
gboolean ctk_source_completion_model_last_proposal              (CtkSourceCompletionModel    *model,
								 CtkTreeIter                 *iter);

CTK_SOURCE_INTERNAL
gboolean ctk_source_completion_model_next_proposal              (CtkSourceCompletionModel    *model,
								 CtkTreeIter                 *iter);

CTK_SOURCE_INTERNAL
gboolean ctk_source_completion_model_previous_proposal          (CtkSourceCompletionModel    *model,
								 CtkTreeIter                 *iter);

CTK_SOURCE_INTERNAL
gboolean ctk_source_completion_model_has_info                   (CtkSourceCompletionModel    *model);

CTK_SOURCE_INTERNAL
gboolean ctk_source_completion_model_iter_equal			(CtkSourceCompletionModel    *model,
								 CtkTreeIter                 *iter1,
								 CtkTreeIter                 *iter2);

G_END_DECLS

#endif /* CTK_SOURCE_COMPLETION_MODEL_H */
