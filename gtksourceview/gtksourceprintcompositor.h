/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2003  Gustavo Giráldez
 * Copyright (C) 2007-2008  Paolo Maggi, Paolo Borelli and Yevgen Muntyan
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

#ifndef CTK_SOURCE_PRINT_COMPOSITOR_H
#define CTK_SOURCE_PRINT_COMPOSITOR_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_PRINT_COMPOSITOR            (ctk_source_print_compositor_get_type ())
#define CTK_SOURCE_PRINT_COMPOSITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_PRINT_COMPOSITOR, CtkSourcePrintCompositor))
#define CTK_SOURCE_PRINT_COMPOSITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_PRINT_COMPOSITOR, CtkSourcePrintCompositorClass))
#define CTK_SOURCE_IS_PRINT_COMPOSITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_PRINT_COMPOSITOR))
#define CTK_SOURCE_IS_PRINT_COMPOSITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_PRINT_COMPOSITOR))
#define CTK_SOURCE_PRINT_COMPOSITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_PRINT_COMPOSITOR, CtkSourcePrintCompositorClass))

typedef struct _CtkSourcePrintCompositorClass    CtkSourcePrintCompositorClass;
typedef struct _CtkSourcePrintCompositorPrivate  CtkSourcePrintCompositorPrivate;

struct _CtkSourcePrintCompositor
{
	GObject parent_instance;

	CtkSourcePrintCompositorPrivate *priv;
};

struct _CtkSourcePrintCompositorClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	gpointer padding[10];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType			  ctk_source_print_compositor_get_type		(void) G_GNUC_CONST;


CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourcePrintCompositor *ctk_source_print_compositor_new		(CtkSourceBuffer          *buffer);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourcePrintCompositor *ctk_source_print_compositor_new_from_view	(CtkSourceView            *view);


CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceBuffer   	 *ctk_source_print_compositor_get_buffer	(CtkSourcePrintCompositor *compositor);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_tab_width	(CtkSourcePrintCompositor *compositor,
									 guint                     width);

CTK_SOURCE_AVAILABLE_IN_ALL
guint			  ctk_source_print_compositor_get_tab_width	(CtkSourcePrintCompositor *compositor);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_wrap_mode	(CtkSourcePrintCompositor *compositor,
									 CtkWrapMode               wrap_mode);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkWrapMode		  ctk_source_print_compositor_get_wrap_mode	(CtkSourcePrintCompositor *compositor);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_highlight_syntax
									(CtkSourcePrintCompositor *compositor,
									 gboolean                  highlight);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		  ctk_source_print_compositor_get_highlight_syntax
									(CtkSourcePrintCompositor *compositor);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_print_line_numbers
									(CtkSourcePrintCompositor *compositor,
									 guint                     interval);

CTK_SOURCE_AVAILABLE_IN_ALL
guint			  ctk_source_print_compositor_get_print_line_numbers
									(CtkSourcePrintCompositor *compositor);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_body_font_name
									(CtkSourcePrintCompositor *compositor,
									 const gchar              *font_name);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar			 *ctk_source_print_compositor_get_body_font_name
									(CtkSourcePrintCompositor *compositor);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_line_numbers_font_name
									(CtkSourcePrintCompositor *compositor,
									 const gchar              *font_name);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar			 *ctk_source_print_compositor_get_line_numbers_font_name
									(CtkSourcePrintCompositor *compositor);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_header_font_name
									(CtkSourcePrintCompositor *compositor,
									 const gchar              *font_name);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar			 *ctk_source_print_compositor_get_header_font_name
									(CtkSourcePrintCompositor *compositor);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_footer_font_name
									(CtkSourcePrintCompositor *compositor,
									 const gchar              *font_name);

CTK_SOURCE_AVAILABLE_IN_ALL
gchar			 *ctk_source_print_compositor_get_footer_font_name
									(CtkSourcePrintCompositor *compositor);

CTK_SOURCE_AVAILABLE_IN_ALL
gdouble			  ctk_source_print_compositor_get_top_margin	(CtkSourcePrintCompositor *compositor,
									 CtkUnit                   unit);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_top_margin	(CtkSourcePrintCompositor *compositor,
									 gdouble                   margin,
									 CtkUnit                   unit);

CTK_SOURCE_AVAILABLE_IN_ALL
gdouble			  ctk_source_print_compositor_get_bottom_margin	(CtkSourcePrintCompositor *compositor,
									 CtkUnit                   unit);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_bottom_margin	(CtkSourcePrintCompositor *compositor,
									 gdouble                   margin,
									 CtkUnit                   unit);

CTK_SOURCE_AVAILABLE_IN_ALL
gdouble			  ctk_source_print_compositor_get_left_margin	(CtkSourcePrintCompositor *compositor,
									 CtkUnit                   unit);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_left_margin	(CtkSourcePrintCompositor *compositor,
									 gdouble                   margin,
									 CtkUnit                   unit);

CTK_SOURCE_AVAILABLE_IN_ALL
gdouble			  ctk_source_print_compositor_get_right_margin	(CtkSourcePrintCompositor *compositor,
									 CtkUnit                   unit);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_right_margin	(CtkSourcePrintCompositor *compositor,
									 gdouble                   margin,
									 CtkUnit                   unit);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_print_header	(CtkSourcePrintCompositor *compositor,
									 gboolean                  print);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		  ctk_source_print_compositor_get_print_header	(CtkSourcePrintCompositor *compositor);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_print_footer	(CtkSourcePrintCompositor *compositor,
									 gboolean                  print);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		  ctk_source_print_compositor_get_print_footer	(CtkSourcePrintCompositor *compositor);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_header_format	(CtkSourcePrintCompositor *compositor,
									 gboolean                  separator,
									 const gchar              *left,
									 const gchar              *center,
									 const gchar              *right);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_set_footer_format	(CtkSourcePrintCompositor *compositor,
									 gboolean                  separator,
									 const gchar              *left,
									 const gchar              *center,
									 const gchar              *right);

CTK_SOURCE_AVAILABLE_IN_ALL
gint			  ctk_source_print_compositor_get_n_pages	(CtkSourcePrintCompositor *compositor);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean		  ctk_source_print_compositor_paginate		(CtkSourcePrintCompositor *compositor,
									 CtkPrintContext          *context);

CTK_SOURCE_AVAILABLE_IN_ALL
gdouble			  ctk_source_print_compositor_get_pagination_progress
									(CtkSourcePrintCompositor *compositor);

CTK_SOURCE_AVAILABLE_IN_ALL
void			  ctk_source_print_compositor_draw_page		(CtkSourcePrintCompositor *compositor,
									 CtkPrintContext          *context,
									 gint                      page_nr);

G_END_DECLS

#endif /* CTK_SOURCE_PRINT_COMPOSITOR_H */
