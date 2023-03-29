/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2001 - Mikael Hermansson <tyan@linux.se> and
 *                       Chris Phelps <chicane@reninet.com>
 * Copyright (C) 2003 - Gustavo Giráldez and Paolo Maggi
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

#ifndef CTK_SOURCE_VIEW_H
#define CTK_SOURCE_VIEW_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_VIEW             (ctk_source_view_get_type ())
#define CTK_SOURCE_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_VIEW, CtkSourceView))
#define CTK_SOURCE_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_VIEW, CtkSourceViewClass))
#define CTK_SOURCE_IS_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_VIEW))
#define CTK_SOURCE_IS_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_VIEW))
#define CTK_SOURCE_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_VIEW, CtkSourceViewClass))

typedef struct _CtkSourceViewClass CtkSourceViewClass;
typedef struct _CtkSourceViewPrivate CtkSourceViewPrivate;

/**
 * CtkSourceViewGutterPosition:
 * @CTK_SOURCE_VIEW_GUTTER_POSITION_LINES: the gutter position of the lines
 * renderer
 * @CTK_SOURCE_VIEW_GUTTER_POSITION_MARKS: the gutter position of the marks
 * renderer
 */
typedef enum _CtkSourceViewGutterPosition
{
	CTK_SOURCE_VIEW_GUTTER_POSITION_LINES = -30,
	CTK_SOURCE_VIEW_GUTTER_POSITION_MARKS = -20
} CtkSourceViewGutterPosition;

/**
 * CtkSourceSmartHomeEndType:
 * @CTK_SOURCE_SMART_HOME_END_DISABLED: smart-home-end disabled.
 * @CTK_SOURCE_SMART_HOME_END_BEFORE: move to the first/last
 * non-whitespace character on the first press of the HOME/END keys and
 * to the beginning/end of the line on the second press.
 * @CTK_SOURCE_SMART_HOME_END_AFTER: move to the beginning/end of the
 * line on the first press of the HOME/END keys and to the first/last
 * non-whitespace character on the second press.
 * @CTK_SOURCE_SMART_HOME_END_ALWAYS: always move to the first/last
 * non-whitespace character when the HOME/END keys are pressed.
 */
typedef enum _CtkSourceSmartHomeEndType
{
	CTK_SOURCE_SMART_HOME_END_DISABLED,
	CTK_SOURCE_SMART_HOME_END_BEFORE,
	CTK_SOURCE_SMART_HOME_END_AFTER,
	CTK_SOURCE_SMART_HOME_END_ALWAYS
} CtkSourceSmartHomeEndType;

/**
 * CtkSourceBackgroundPatternType:
 * @CTK_SOURCE_BACKGROUND_PATTERN_TYPE_NONE: no pattern
 * @CTK_SOURCE_BACKGROUND_PATTERN_TYPE_GRID: grid pattern
 *
 * Since: 3.16
 */
typedef enum _CtkSourceBackgroundPatternType
{
	CTK_SOURCE_BACKGROUND_PATTERN_TYPE_NONE,
	CTK_SOURCE_BACKGROUND_PATTERN_TYPE_GRID
} CtkSourceBackgroundPatternType;

struct _CtkSourceView
{
	CtkTextView parent;

	CtkSourceViewPrivate *priv;
};

struct _CtkSourceViewClass
{
	CtkTextViewClass parent_class;

	void (*undo) (CtkSourceView *view);
	void (*redo) (CtkSourceView *view);
	void (*line_mark_activated) (CtkSourceView *view,
	                             CtkTextIter   *iter,
	                             CdkEvent      *event);
	void (*show_completion) (CtkSourceView *view);
	void (*move_lines) (CtkSourceView *view,
			    gboolean       down);

	void (*move_words) (CtkSourceView *view,
	                    gint           step);

	/* Padding for future expansion */
	gpointer padding[20];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType		 ctk_source_view_get_type		(void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
CtkWidget	*ctk_source_view_new			(void);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkWidget 	*ctk_source_view_new_with_buffer	(CtkSourceBuffer *buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_view_set_show_line_numbers 	(CtkSourceView   *view,
							 gboolean         show);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean 	 ctk_source_view_get_show_line_numbers 	(CtkSourceView   *view);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_view_set_tab_width          (CtkSourceView   *view,
							 guint            width);

CTK_SOURCE_AVAILABLE_IN_ALL
guint            ctk_source_view_get_tab_width          (CtkSourceView   *view);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_view_set_indent_width 	(CtkSourceView   *view,
							 gint             width);

CTK_SOURCE_AVAILABLE_IN_ALL
gint		 ctk_source_view_get_indent_width	(CtkSourceView   *view);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_view_set_auto_indent 	(CtkSourceView   *view,
							 gboolean         enable);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	 ctk_source_view_get_auto_indent 	(CtkSourceView   *view);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_view_set_insert_spaces_instead_of_tabs
							(CtkSourceView   *view,
							 gboolean         enable);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	 ctk_source_view_get_insert_spaces_instead_of_tabs
							(CtkSourceView   *view);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_view_set_indent_on_tab 	(CtkSourceView   *view,
							 gboolean         enable);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	 ctk_source_view_get_indent_on_tab 	(CtkSourceView   *view);

CTK_SOURCE_AVAILABLE_IN_3_16
void		 ctk_source_view_indent_lines		(CtkSourceView   *view,
							 CtkTextIter     *start,
							 CtkTextIter     *end);

CTK_SOURCE_AVAILABLE_IN_3_16
void		 ctk_source_view_unindent_lines		(CtkSourceView   *view,
							 CtkTextIter     *start,
							 CtkTextIter     *end);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_view_set_highlight_current_line
							(CtkSourceView   *view,
							 gboolean         highlight);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean 	 ctk_source_view_get_highlight_current_line
							(CtkSourceView   *view);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_view_set_show_right_margin 	(CtkSourceView   *view,
							 gboolean         show);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean 	 ctk_source_view_get_show_right_margin 	(CtkSourceView   *view);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_view_set_right_margin_position
					 		(CtkSourceView   *view,
							 guint            pos);

CTK_SOURCE_AVAILABLE_IN_ALL
guint		 ctk_source_view_get_right_margin_position
					 		(CtkSourceView   *view);

CTK_SOURCE_AVAILABLE_IN_ALL
void 		 ctk_source_view_set_show_line_marks    (CtkSourceView   *view,
							 gboolean         show);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean	 ctk_source_view_get_show_line_marks    (CtkSourceView   *view);

CTK_SOURCE_AVAILABLE_IN_ALL
void             ctk_source_view_set_mark_attributes    (CtkSourceView           *view,
                                                         const gchar             *category,
                                                         CtkSourceMarkAttributes *attributes,
                                                         gint                     priority);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceMarkAttributes *
                 ctk_source_view_get_mark_attributes    (CtkSourceView           *view,
                                                         const gchar             *category,
                                                         gint                    *priority);

CTK_SOURCE_AVAILABLE_IN_3_18
void		 ctk_source_view_set_smart_backspace	(CtkSourceView   *view,
							 gboolean        smart_backspace);

CTK_SOURCE_AVAILABLE_IN_3_18
gboolean	 ctk_source_view_get_smart_backspace	(CtkSourceView   *view);

CTK_SOURCE_AVAILABLE_IN_ALL
void		 ctk_source_view_set_smart_home_end	(CtkSourceView             *view,
							 CtkSourceSmartHomeEndType  smart_home_end);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceSmartHomeEndType
		 ctk_source_view_get_smart_home_end	(CtkSourceView   *view);

CTK_SOURCE_AVAILABLE_IN_ALL
guint		 ctk_source_view_get_visual_column	(CtkSourceView     *view,
							 const CtkTextIter *iter);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceCompletion *
		 ctk_source_view_get_completion		(CtkSourceView   *view);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceGutter *ctk_source_view_get_gutter		(CtkSourceView     *view,
                                                         CtkTextWindowType  window_type);

CTK_SOURCE_AVAILABLE_IN_3_16
void		 ctk_source_view_set_background_pattern	(CtkSourceView                  *view,
                                                         CtkSourceBackgroundPatternType  background_pattern);

CTK_SOURCE_AVAILABLE_IN_3_16
CtkSourceBackgroundPatternType
		 ctk_source_view_get_background_pattern	(CtkSourceView   *view);

CTK_SOURCE_AVAILABLE_IN_3_24
CtkSourceSpaceDrawer *
		 ctk_source_view_get_space_drawer	(CtkSourceView   *view);

G_END_DECLS

#endif /* end of CTK_SOURCE_VIEW_H */
