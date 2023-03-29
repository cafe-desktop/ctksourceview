/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2009 - Jesse van den Kieboom
 *
 * ctksourceview is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * ctksourceview is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CTK_SOURCE_COMPLETION_WORDS_H
#define CTK_SOURCE_COMPLETION_WORDS_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctksourceview/ctksourcecompletionprovider.h>
#include <ctk/ctk.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_COMPLETION_WORDS		(ctk_source_completion_words_get_type ())
#define CTK_SOURCE_COMPLETION_WORDS(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS, CtkSourceCompletionWords))
#define CTK_SOURCE_COMPLETION_WORDS_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_COMPLETION_WORDS, CtkSourceCompletionWordsClass))
#define CTK_SOURCE_IS_COMPLETION_WORDS(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS))
#define CTK_SOURCE_IS_COMPLETION_WORDS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_COMPLETION_WORDS))
#define CTK_SOURCE_COMPLETION_WORDS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS, CtkSourceCompletionWordsClass))

typedef struct _CtkSourceCompletionWords		CtkSourceCompletionWords;
typedef struct _CtkSourceCompletionWordsClass		CtkSourceCompletionWordsClass;
typedef struct _CtkSourceCompletionWordsPrivate		CtkSourceCompletionWordsPrivate;

struct _CtkSourceCompletionWords {
	GObject parent;

	CtkSourceCompletionWordsPrivate *priv;
};

struct _CtkSourceCompletionWordsClass {
	GObjectClass parent_class;
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType		ctk_source_completion_words_get_type	(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceCompletionWords *
		ctk_source_completion_words_new 	(const gchar              *name,
		                                         GdkPixbuf                *icon);

CTK_SOURCE_AVAILABLE_IN_ALL
void 		ctk_source_completion_words_register 	(CtkSourceCompletionWords *words,
                                                         CtkTextBuffer            *buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
void 		ctk_source_completion_words_unregister 	(CtkSourceCompletionWords *words,
                                                         CtkTextBuffer            *buffer);

G_END_DECLS

#endif /* CTK_SOURCE_COMPLETION_WORDS_H */
