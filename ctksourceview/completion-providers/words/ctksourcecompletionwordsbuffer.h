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

#ifndef CTK_SOURCE_COMPLETION_WORDS_BUFFER_H
#define CTK_SOURCE_COMPLETION_WORDS_BUFFER_H

#include <ctk/ctk.h>

#include "ctksourcecompletionwordslibrary.h"

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_COMPLETION_WORDS_BUFFER			(ctk_source_completion_words_buffer_get_type ())
#define CTK_SOURCE_COMPLETION_WORDS_BUFFER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_BUFFER, CtkSourceCompletionWordsBuffer))
#define CTK_SOURCE_COMPLETION_WORDS_BUFFER_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_BUFFER, CtkSourceCompletionWordsBuffer const))
#define CTK_SOURCE_COMPLETION_WORDS_BUFFER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_COMPLETION_WORDS_BUFFER, CtkSourceCompletionWordsBufferClass))
#define CTK_SOURCE_IS_COMPLETION_WORDS_BUFFER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_BUFFER))
#define CTK_SOURCE_IS_COMPLETION_WORDS_BUFFER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_COMPLETION_WORDS_BUFFER))
#define CTK_SOURCE_COMPLETION_WORDS_BUFFER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_BUFFER, CtkSourceCompletionWordsBufferClass))

typedef struct _CtkSourceCompletionWordsBuffer			CtkSourceCompletionWordsBuffer;
typedef struct _CtkSourceCompletionWordsBufferClass		CtkSourceCompletionWordsBufferClass;
typedef struct _CtkSourceCompletionWordsBufferPrivate		CtkSourceCompletionWordsBufferPrivate;

struct _CtkSourceCompletionWordsBuffer {
	GObject parent;

	CtkSourceCompletionWordsBufferPrivate *priv;
};

struct _CtkSourceCompletionWordsBufferClass {
	GObjectClass parent_class;
};

G_GNUC_INTERNAL
GType		 ctk_source_completion_words_buffer_get_type			(void) G_GNUC_CONST;

G_GNUC_INTERNAL
CtkSourceCompletionWordsBuffer *
		 ctk_source_completion_words_buffer_new				(CtkSourceCompletionWordsLibrary *library,
										 CtkTextBuffer                   *buffer);

G_GNUC_INTERNAL
CtkTextBuffer 	*ctk_source_completion_words_buffer_get_buffer			(CtkSourceCompletionWordsBuffer  *buffer);

G_GNUC_INTERNAL
void		 ctk_source_completion_words_buffer_set_scan_batch_size		(CtkSourceCompletionWordsBuffer  *buffer,
										 guint                            size);

G_GNUC_INTERNAL
void		 ctk_source_completion_words_buffer_set_minimum_word_size	(CtkSourceCompletionWordsBuffer  *buffer,
										 guint                            size);

G_END_DECLS

#endif /* CTK_SOURCE_COMPLETION_WORDS_BUFFER_H */
