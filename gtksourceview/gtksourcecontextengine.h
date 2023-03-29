/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2003 - Gustavo Giráldez
 * Copyright (C) 2005 - Marco Barisione, Emanuele Aina
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

#ifndef CTK_SOURCE_CONTEXT_ENGINE_H
#define CTK_SOURCE_CONTEXT_ENGINE_H

#include "ctksourceengine.h"
#include "ctksourcetypes.h"
#include "ctksourcetypes-private.h"

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_CONTEXT_ENGINE            (_ctk_source_context_engine_get_type ())
#define CTK_SOURCE_CONTEXT_ENGINE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_CONTEXT_ENGINE, CtkSourceContextEngine))
#define CTK_SOURCE_CONTEXT_ENGINE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_CONTEXT_ENGINE, CtkSourceContextEngineClass))
#define CTK_SOURCE_IS_CONTEXT_ENGINE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_CONTEXT_ENGINE))
#define CTK_SOURCE_IS_CONTEXT_ENGINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_CONTEXT_ENGINE))
#define CTK_SOURCE_CONTEXT_ENGINE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_CONTEXT_ENGINE, CtkSourceContextEngineClass))

typedef struct _CtkSourceContextData          CtkSourceContextData;
typedef struct _CtkSourceContextReplace       CtkSourceContextReplace;
typedef struct _CtkSourceContextClass         CtkSourceContextClass;
typedef struct _CtkSourceContextEngineClass   CtkSourceContextEngineClass;
typedef struct _CtkSourceContextEnginePrivate CtkSourceContextEnginePrivate;

struct _CtkSourceContextEngine
{
	GObject parent_instance;

	/*< private >*/
	CtkSourceContextEnginePrivate *priv;
};

struct _CtkSourceContextEngineClass
{
	GObjectClass parent_class;
};

typedef enum _CtkSourceContextFlags {
	CTK_SOURCE_CONTEXT_EXTEND_PARENT	= 1 << 0,
	CTK_SOURCE_CONTEXT_END_PARENT		= 1 << 1,
	CTK_SOURCE_CONTEXT_END_AT_LINE_END	= 1 << 2,
	CTK_SOURCE_CONTEXT_FIRST_LINE_ONLY	= 1 << 3,
	CTK_SOURCE_CONTEXT_ONCE_ONLY		= 1 << 4,
	CTK_SOURCE_CONTEXT_STYLE_INSIDE		= 1 << 5
} CtkSourceContextFlags;

typedef enum _CtkSourceContextRefOptions {
	CTK_SOURCE_CONTEXT_IGNORE_STYLE		= 1 << 0,
	CTK_SOURCE_CONTEXT_OVERRIDE_STYLE	= 1 << 1,
	CTK_SOURCE_CONTEXT_REF_ORIGINAL		= 1 << 2
} CtkSourceContextRefOptions;

G_GNUC_INTERNAL
GType			 _ctk_source_context_engine_get_type		(void) G_GNUC_CONST;

G_GNUC_INTERNAL
CtkSourceContextData	*_ctk_source_context_data_new			(CtkSourceLanguage	 *lang);

G_GNUC_INTERNAL
CtkSourceContextData	*_ctk_source_context_data_ref			(CtkSourceContextData	 *data);

G_GNUC_INTERNAL
void			 _ctk_source_context_data_unref			(CtkSourceContextData	 *data);

G_GNUC_INTERNAL
CtkSourceContextClass	*ctk_source_context_class_new			(gchar const		 *name,
									 gboolean		  enabled);

G_GNUC_INTERNAL
void			 ctk_source_context_class_free			(CtkSourceContextClass	 *cclass);

G_GNUC_INTERNAL
CtkSourceContextEngine	*_ctk_source_context_engine_new			(CtkSourceContextData	 *data);

G_GNUC_INTERNAL
gboolean		 _ctk_source_context_data_define_context	(CtkSourceContextData	 *data,
									 const gchar		 *id,
									 const gchar		 *parent_id,
									 const gchar		 *match_regex,
									 const gchar		 *start_regex,
									 const gchar		 *end_regex,
									 const gchar		 *style,
									 GSList			 *context_classes,
									 CtkSourceContextFlags    flags,
									 GError			**error);

G_GNUC_INTERNAL
gboolean		 _ctk_source_context_data_add_sub_pattern	(CtkSourceContextData	 *data,
									 const gchar		 *id,
									 const gchar		 *parent_id,
									 const gchar		 *name,
									 const gchar		 *where,
									 const gchar		 *style,
									 GSList			 *context_classes,
									 GError			**error);

G_GNUC_INTERNAL
gboolean		 _ctk_source_context_data_add_ref		(CtkSourceContextData	 *data,
									 const gchar		 *parent_id,
									 const gchar		 *ref_id,
									 CtkSourceContextRefOptions options,
									 const gchar		 *style,
									 gboolean		  all,
									 GError			**error);

G_GNUC_INTERNAL
CtkSourceContextReplace	*_ctk_source_context_replace_new		(const gchar             *to_replace_id,
									 const gchar             *replace_with_id);

G_GNUC_INTERNAL
void			 _ctk_source_context_replace_free		(CtkSourceContextReplace *repl);

G_GNUC_INTERNAL
gboolean		 _ctk_source_context_data_finish_parse		(CtkSourceContextData	 *data,
									 GList                   *overrides,
									 GError			**error);

/* Only for lang files version 1, do not use it */
G_GNUC_INTERNAL
void			 _ctk_source_context_data_set_escape_char	(CtkSourceContextData	 *data,
									 gunichar		  esc_char);

G_END_DECLS

#endif /* CTK_SOURCE_CONTEXT_ENGINE_H */
