/*
 * This file is part of ctksourceview
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

#ifndef CTK_SOURCE_AUTOCLEANUPS_H
#define CTK_SOURCE_AUTOCLEANUPS_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <glib.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#ifndef __GI_SCANNER__

G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceBuffer, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceCompletion, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceCompletionContext, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceCompletionInfo, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceCompletionItem, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceFile, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceFileLoader, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceFileSaver, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceGutter, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceGutterRenderer, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceGutterRendererPixbuf, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceGutterRendererText, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceLanguage, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceLanguageManager, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceMark, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceMap, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourcePrintCompositor, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceSearchContext, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceSearchSettings, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceSpaceDrawer, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceStyleScheme, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceStyleSchemeChooserButton, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceStyleSchemeChooserWidget, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceStyleSchemeManager, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceUndoManager, g_object_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC(CtkSourceView, g_object_unref)

#endif /* __GI_SCANNER__ */

G_END_DECLS

#endif /* CTK_SOURCE_AUTOCLEANUPS_H */
