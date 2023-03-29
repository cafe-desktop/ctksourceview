/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2012-2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef CTK_SOURCE_TYPES_H
#define CTK_SOURCE_TYPES_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <glib.h>
#include <ctksourceview/ctksourceversion.h>

G_BEGIN_DECLS

/* This header exists to avoid cycles in header inclusions, when header A needs
 * the type B and header B needs the type A. For an alternative way to solve
 * this problem (in C11), see:
 * https://bugzilla.gnome.org/show_bug.cgi?id=679424#c20
 */

typedef struct _CtkSourceBuffer			CtkSourceBuffer;
typedef struct _CtkSourceCompletionContext	CtkSourceCompletionContext;
typedef struct _CtkSourceCompletion		CtkSourceCompletion;
typedef struct _CtkSourceCompletionInfo		CtkSourceCompletionInfo;
typedef struct _CtkSourceCompletionItem		CtkSourceCompletionItem;
typedef struct _CtkSourceCompletionProposal	CtkSourceCompletionProposal;
typedef struct _CtkSourceCompletionProvider	CtkSourceCompletionProvider;
typedef struct _CtkSourceEncoding		CtkSourceEncoding;
typedef struct _CtkSourceFile			CtkSourceFile;
typedef struct _CtkSourceFileLoader		CtkSourceFileLoader;
typedef struct _CtkSourceFileSaver		CtkSourceFileSaver;
typedef struct _CtkSourceGutter			CtkSourceGutter;
typedef struct _CtkSourceGutterRenderer		CtkSourceGutterRenderer;
typedef struct _CtkSourceGutterRendererPixbuf	CtkSourceGutterRendererPixbuf;
typedef struct _CtkSourceGutterRendererText	CtkSourceGutterRendererText;
typedef struct _CtkSourceLanguage		CtkSourceLanguage;
typedef struct _CtkSourceLanguageManager	CtkSourceLanguageManager;
typedef struct _CtkSourceMap			CtkSourceMap;
typedef struct _CtkSourceMarkAttributes		CtkSourceMarkAttributes;
typedef struct _CtkSourceMark			CtkSourceMark;
typedef struct _CtkSourcePrintCompositor	CtkSourcePrintCompositor;
typedef struct _CtkSourceSearchContext		CtkSourceSearchContext;
typedef struct _CtkSourceSearchSettings		CtkSourceSearchSettings;
typedef struct _CtkSourceSpaceDrawer		CtkSourceSpaceDrawer;
typedef struct _CtkSourceStyle			CtkSourceStyle;
typedef struct _CtkSourceStyleScheme		CtkSourceStyleScheme;
typedef struct _CtkSourceStyleSchemeChooser	CtkSourceStyleSchemeChooser;
typedef struct _CtkSourceStyleSchemeChooserButton CtkSourceStyleSchemeChooserButton;
typedef struct _CtkSourceStyleSchemeChooserWidget CtkSourceStyleSchemeChooserWidget;
typedef struct _CtkSourceStyleSchemeManager	CtkSourceStyleSchemeManager;
typedef struct _CtkSourceUndoManager		CtkSourceUndoManager;
typedef struct _CtkSourceView			CtkSourceView;

G_END_DECLS

#endif /* CTK_SOURCE_TYPES_H */
