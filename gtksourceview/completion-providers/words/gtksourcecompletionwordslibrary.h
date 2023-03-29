/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of GtkSourceView
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

#ifndef CTK_SOURCE_COMPLETION_WORDS_LIBRARY_H
#define CTK_SOURCE_COMPLETION_WORDS_LIBRARY_H

#include <glib-object.h>
#include "ctksourcecompletionwordsproposal.h"

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_COMPLETION_WORDS_LIBRARY			(ctk_source_completion_words_library_get_type ())
#define CTK_SOURCE_COMPLETION_WORDS_LIBRARY(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_LIBRARY, GtkSourceCompletionWordsLibrary))
#define CTK_SOURCE_COMPLETION_WORDS_LIBRARY_CONST(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_LIBRARY, GtkSourceCompletionWordsLibrary const))
#define CTK_SOURCE_COMPLETION_WORDS_LIBRARY_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_COMPLETION_WORDS_LIBRARY, GtkSourceCompletionWordsLibraryClass))
#define CTK_SOURCE_IS_COMPLETION_WORDS_LIBRARY(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_LIBRARY))
#define CTK_SOURCE_IS_COMPLETION_WORDS_LIBRARY_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_COMPLETION_WORDS_LIBRARY))
#define CTK_SOURCE_COMPLETION_WORDS_LIBRARY_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_COMPLETION_WORDS_LIBRARY, GtkSourceCompletionWordsLibraryClass))

typedef struct _GtkSourceCompletionWordsLibrary			GtkSourceCompletionWordsLibrary;
typedef struct _GtkSourceCompletionWordsLibraryClass		GtkSourceCompletionWordsLibraryClass;
typedef struct _GtkSourceCompletionWordsLibraryPrivate		GtkSourceCompletionWordsLibraryPrivate;

struct _GtkSourceCompletionWordsLibrary {
	GObject parent;

	GtkSourceCompletionWordsLibraryPrivate *priv;
};

struct _GtkSourceCompletionWordsLibraryClass {
	GObjectClass parent_class;
};

CTK_SOURCE_INTERNAL
GType		 ctk_source_completion_words_library_get_type		(void) G_GNUC_CONST;

CTK_SOURCE_INTERNAL
GtkSourceCompletionWordsLibrary *
		 ctk_source_completion_words_library_new		(void);

/* Finding */
CTK_SOURCE_INTERNAL
GSequenceIter	*ctk_source_completion_words_library_find 		(GtkSourceCompletionWordsLibrary  *library,
									 GtkSourceCompletionWordsProposal *proposal);

CTK_SOURCE_INTERNAL
GSequenceIter	*ctk_source_completion_words_library_find_first		(GtkSourceCompletionWordsLibrary  *library,
									 const gchar                      *word,
									 gint                              len);

CTK_SOURCE_INTERNAL
GSequenceIter	*ctk_source_completion_words_library_find_next		(GSequenceIter                    *iter,
									 const gchar                      *word,
									 gint                              len);

/* Getting */
CTK_SOURCE_INTERNAL
GtkSourceCompletionWordsProposal *
		 ctk_source_completion_words_library_get_proposal 	(GSequenceIter                    *iter);

/* Adding/removing */
CTK_SOURCE_INTERNAL
GtkSourceCompletionWordsProposal *
		 ctk_source_completion_words_library_add_word 		(GtkSourceCompletionWordsLibrary  *library,
                                              				 const gchar                      *word);

CTK_SOURCE_INTERNAL
void		 ctk_source_completion_words_library_remove_word 	(GtkSourceCompletionWordsLibrary  *library,
                                                 			 GtkSourceCompletionWordsProposal *proposal);

CTK_SOURCE_INTERNAL
gboolean	 ctk_source_completion_words_library_is_locked 		(GtkSourceCompletionWordsLibrary  *library);

CTK_SOURCE_INTERNAL
void		 ctk_source_completion_words_library_lock 		(GtkSourceCompletionWordsLibrary  *library);

CTK_SOURCE_INTERNAL
void		 ctk_source_completion_words_library_unlock 		(GtkSourceCompletionWordsLibrary  *library);

G_END_DECLS

#endif /* CTK_SOURCE_COMPLETION_WORDS_LIBRARY_H */
