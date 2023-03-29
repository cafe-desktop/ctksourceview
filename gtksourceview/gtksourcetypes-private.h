/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2012, 2013, 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef CTK_SOURCE_TYPES_PRIVATE_H
#define CTK_SOURCE_TYPES_PRIVATE_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct _CtkSourceBufferInputStream	CtkSourceBufferInputStream;
typedef struct _CtkSourceBufferOutputStream	CtkSourceBufferOutputStream;
typedef struct _CtkSourceCompletionContainer	CtkSourceCompletionContainer;
typedef struct _CtkSourceCompletionModel	CtkSourceCompletionModel;
typedef struct _CtkSourceContextEngine		CtkSourceContextEngine;
typedef struct _CtkSourceEngine			CtkSourceEngine;
typedef struct _CtkSourceGutterRendererLines	CtkSourceGutterRendererLines;
typedef struct _CtkSourceGutterRendererMarks	CtkSourceGutterRendererMarks;
typedef struct _CtkSourceMarksSequence		CtkSourceMarksSequence;
typedef struct _CtkSourcePixbufHelper		CtkSourcePixbufHelper;
typedef struct _CtkSourceRegex			CtkSourceRegex;
typedef struct _CtkSourceUndoManagerDefault	CtkSourceUndoManagerDefault;

#ifdef _MSC_VER
/* For Visual Studio, we need to export the symbols used by the unit tests */
#define CTK_SOURCE_INTERNAL __declspec(dllexport)
#else
#define CTK_SOURCE_INTERNAL G_GNUC_INTERNAL
#endif

G_END_DECLS

#endif /* CTK_SOURCE_TYPES_PRIVATE_H */
