/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
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

#ifndef CTK_SOURCE_MARKS_SEQUENCE_H
#define CTK_SOURCE_MARKS_SEQUENCE_H

#include <ctk/ctk.h>
#include "ctksourcetypes-private.h"

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_MARKS_SEQUENCE             (_ctk_source_marks_sequence_get_type ())
#define CTK_SOURCE_MARKS_SEQUENCE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_MARKS_SEQUENCE, CtkSourceMarksSequence))
#define CTK_SOURCE_MARKS_SEQUENCE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_MARKS_SEQUENCE, CtkSourceMarksSequenceClass))
#define CTK_SOURCE_IS_MARKS_SEQUENCE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_MARKS_SEQUENCE))
#define CTK_SOURCE_IS_MARKS_SEQUENCE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_MARKS_SEQUENCE))
#define CTK_SOURCE_MARKS_SEQUENCE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_MARKS_SEQUENCE, CtkSourceMarksSequenceClass))

typedef struct _CtkSourceMarksSequenceClass    CtkSourceMarksSequenceClass;
typedef struct _CtkSourceMarksSequencePrivate  CtkSourceMarksSequencePrivate;

struct _CtkSourceMarksSequence
{
	GObject parent;

	CtkSourceMarksSequencePrivate *priv;
};

struct _CtkSourceMarksSequenceClass
{
	GObjectClass parent_class;
};

G_GNUC_INTERNAL
GType			 _ctk_source_marks_sequence_get_type		(void) G_GNUC_CONST;

G_GNUC_INTERNAL
CtkSourceMarksSequence	*_ctk_source_marks_sequence_new			(CtkTextBuffer          *buffer);

G_GNUC_INTERNAL
gboolean		 _ctk_source_marks_sequence_is_empty		(CtkSourceMarksSequence *seq);

G_GNUC_INTERNAL
void			 _ctk_source_marks_sequence_add			(CtkSourceMarksSequence *seq,
									 CtkTextMark            *mark);

G_GNUC_INTERNAL
void			 _ctk_source_marks_sequence_remove		(CtkSourceMarksSequence *seq,
									 CtkTextMark            *mark);

G_GNUC_INTERNAL
CtkTextMark		*_ctk_source_marks_sequence_next		(CtkSourceMarksSequence *seq,
									 CtkTextMark            *mark);

G_GNUC_INTERNAL
CtkTextMark		*_ctk_source_marks_sequence_prev		(CtkSourceMarksSequence *seq,
									 CtkTextMark            *mark);

G_GNUC_INTERNAL
gboolean		 _ctk_source_marks_sequence_forward_iter	(CtkSourceMarksSequence *seq,
									 CtkTextIter            *iter);

G_GNUC_INTERNAL
gboolean		 _ctk_source_marks_sequence_backward_iter	(CtkSourceMarksSequence *seq,
									 CtkTextIter            *iter);

G_GNUC_INTERNAL
GSList			*_ctk_source_marks_sequence_get_marks_at_iter	(CtkSourceMarksSequence *seq,
									 const CtkTextIter      *iter);

G_GNUC_INTERNAL
GSList			*_ctk_source_marks_sequence_get_marks_in_range	(CtkSourceMarksSequence *seq,
									 const CtkTextIter      *iter1,
									 const CtkTextIter      *iter2);

G_END_DECLS

#endif /* CTK_SOURCE_MARKS_SEQUENCE_H */
