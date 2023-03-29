/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- *
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2010 - Jesse van den Kieboom
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

#ifndef CTK_SOURCE_GUTTER_RENDERER_H
#define CTK_SOURCE_GUTTER_RENDERER_H

#if !defined (CTK_SOURCE_H_INSIDE) && !defined (CTK_SOURCE_COMPILATION)
#error "Only <ctksourceview/ctksource.h> can be included directly."
#endif

#include <ctk/ctk.h>
#include <ctksourceview/ctksourcetypes.h>

G_BEGIN_DECLS

#define CTK_SOURCE_TYPE_GUTTER_RENDERER			(ctk_source_gutter_renderer_get_type ())
#define CTK_SOURCE_GUTTER_RENDERER(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER, CtkSourceGutterRenderer))
#define CTK_SOURCE_GUTTER_RENDERER_CONST(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER, CtkSourceGutterRenderer const))
#define CTK_SOURCE_GUTTER_RENDERER_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), CTK_SOURCE_TYPE_GUTTER_RENDERER, CtkSourceGutterRendererClass))
#define CTK_SOURCE_IS_GUTTER_RENDERER(obj)		(G_TYPE_CHECK_INSTANCE_TYPE ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER))
#define CTK_SOURCE_IS_GUTTER_RENDERER_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), CTK_SOURCE_TYPE_GUTTER_RENDERER))
#define CTK_SOURCE_GUTTER_RENDERER_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), CTK_SOURCE_TYPE_GUTTER_RENDERER, CtkSourceGutterRendererClass))

typedef struct _CtkSourceGutterRendererClass	CtkSourceGutterRendererClass;
typedef struct _CtkSourceGutterRendererPrivate	CtkSourceGutterRendererPrivate;

/**
 * CtkSourceGutterRendererState:
 * @CTK_SOURCE_GUTTER_RENDERER_STATE_NORMAL: normal state
 * @CTK_SOURCE_GUTTER_RENDERER_STATE_CURSOR: area in the renderer represents the
 * line on which the insert cursor is currently positioned
 * @CTK_SOURCE_GUTTER_RENDERER_STATE_PRELIT: the mouse pointer is currently
 * over the activatable area of the renderer
 * @CTK_SOURCE_GUTTER_RENDERER_STATE_SELECTED: area in the renderer represents
 * a line in the buffer which contains part of the selection
 **/
typedef enum _CtkSourceGutterRendererState
{
	CTK_SOURCE_GUTTER_RENDERER_STATE_NORMAL = 0,
	CTK_SOURCE_GUTTER_RENDERER_STATE_CURSOR = 1 << 0,
	CTK_SOURCE_GUTTER_RENDERER_STATE_PRELIT = 1 << 1,
	CTK_SOURCE_GUTTER_RENDERER_STATE_SELECTED = 1 << 2
} CtkSourceGutterRendererState;

/**
 * CtkSourceGutterRendererAlignmentMode:
 * @CTK_SOURCE_GUTTER_RENDERER_ALIGNMENT_MODE_CELL: The full cell.
 * @CTK_SOURCE_GUTTER_RENDERER_ALIGNMENT_MODE_FIRST: The first line.
 * @CTK_SOURCE_GUTTER_RENDERER_ALIGNMENT_MODE_LAST: The last line.
 *
 * The alignment mode of the renderer, when a cell spans multiple lines (due to
 * text wrapping).
 **/
typedef enum _CtkSourceGutterRendererAlignmentMode
{
	CTK_SOURCE_GUTTER_RENDERER_ALIGNMENT_MODE_CELL,
	CTK_SOURCE_GUTTER_RENDERER_ALIGNMENT_MODE_FIRST,
	CTK_SOURCE_GUTTER_RENDERER_ALIGNMENT_MODE_LAST
} CtkSourceGutterRendererAlignmentMode;

struct _CtkSourceGutterRenderer
{
	GInitiallyUnowned parent;

	/*< private >*/
	CtkSourceGutterRendererPrivate *priv;
};

struct _CtkSourceGutterRendererClass
{
	GInitiallyUnownedClass parent_class;

	/*< public >*/
	void (*begin)               (CtkSourceGutterRenderer     *renderer,
	                             cairo_t                     *cr,
	                             CdkRectangle                *background_area,
	                             CdkRectangle                *cell_area,
	                             CtkTextIter                 *start,
	                             CtkTextIter                 *end);

	void (*draw)                (CtkSourceGutterRenderer      *renderer,
	                             cairo_t                      *cr,
	                             CdkRectangle                 *background_area,
	                             CdkRectangle                 *cell_area,
	                             CtkTextIter                  *start,
	                             CtkTextIter                  *end,
	                             CtkSourceGutterRendererState  state);

	void (*end)                 (CtkSourceGutterRenderer      *renderer);

	/**
	 * CtkSourceGutterRendererClass::change_view:
	 * @renderer: a #CtkSourceGutterRenderer.
	 * @old_view: (nullable): the old #CtkTextView.
	 *
	 * This is called when the text view changes for @renderer.
	 */
	void (*change_view)         (CtkSourceGutterRenderer      *renderer,
	                             CtkTextView                  *old_view);

	/**
	 * CtkSourceGutterRendererClass::change_buffer:
	 * @renderer: a #CtkSourceGutterRenderer.
	 * @old_buffer: (nullable): the old #CtkTextBuffer.
	 *
	 * This is called when the text buffer changes for @renderer.
	 */
	void (*change_buffer)       (CtkSourceGutterRenderer      *renderer,
	                             CtkTextBuffer                *old_buffer);

	/* Signal handlers */
	gboolean (*query_activatable) (CtkSourceGutterRenderer      *renderer,
	                               CtkTextIter                  *iter,
	                               CdkRectangle                 *area,
	                               CdkEvent                     *event);

	void (*activate)            (CtkSourceGutterRenderer      *renderer,
	                             CtkTextIter                  *iter,
	                             CdkRectangle                 *area,
	                             CdkEvent                     *event);

	void (*queue_draw)          (CtkSourceGutterRenderer      *renderer);

	gboolean (*query_tooltip)   (CtkSourceGutterRenderer      *renderer,
	                             CtkTextIter                  *iter,
	                             CdkRectangle                 *area,
	                             gint                          x,
	                             gint                          y,
	                             CtkTooltip                   *tooltip);

	void (*query_data)          (CtkSourceGutterRenderer      *renderer,
	                             CtkTextIter                  *start,
	                             CtkTextIter                  *end,
	                             CtkSourceGutterRendererState  state);

	gpointer padding[20];
};

CTK_SOURCE_AVAILABLE_IN_ALL
GType    ctk_source_gutter_renderer_get_type (void) G_GNUC_CONST;

CTK_SOURCE_AVAILABLE_IN_ALL
void     ctk_source_gutter_renderer_begin           (CtkSourceGutterRenderer      *renderer,
                                                     cairo_t                      *cr,
                                                     CdkRectangle                 *background_area,
                                                     CdkRectangle                 *cell_area,
                                                     CtkTextIter                  *start,
                                                     CtkTextIter                  *end);

CTK_SOURCE_AVAILABLE_IN_ALL
void     ctk_source_gutter_renderer_draw            (CtkSourceGutterRenderer      *renderer,
                                                     cairo_t                      *cr,
                                                     CdkRectangle                 *background_area,
                                                     CdkRectangle                 *cell_area,
                                                     CtkTextIter                  *start,
                                                     CtkTextIter                  *end,
                                                     CtkSourceGutterRendererState  state);

CTK_SOURCE_AVAILABLE_IN_ALL
void     ctk_source_gutter_renderer_end             (CtkSourceGutterRenderer      *renderer);

CTK_SOURCE_AVAILABLE_IN_ALL
gint     ctk_source_gutter_renderer_get_size        (CtkSourceGutterRenderer      *renderer);

CTK_SOURCE_AVAILABLE_IN_ALL
void    ctk_source_gutter_renderer_set_size         (CtkSourceGutterRenderer      *renderer,
                                                     gint                          size);

CTK_SOURCE_AVAILABLE_IN_ALL
void     ctk_source_gutter_renderer_set_visible     (CtkSourceGutterRenderer      *renderer,
                                                     gboolean                      visible);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean ctk_source_gutter_renderer_get_visible     (CtkSourceGutterRenderer      *renderer);

CTK_SOURCE_AVAILABLE_IN_ALL
void     ctk_source_gutter_renderer_get_padding     (CtkSourceGutterRenderer      *renderer,
                                                     gint                         *xpad,
                                                     gint                         *ypad);

CTK_SOURCE_AVAILABLE_IN_ALL
void     ctk_source_gutter_renderer_set_padding     (CtkSourceGutterRenderer      *renderer,
                                                     gint                          xpad,
                                                     gint                          ypad);

CTK_SOURCE_AVAILABLE_IN_ALL
void     ctk_source_gutter_renderer_get_alignment   (CtkSourceGutterRenderer      *renderer,
                                                     gfloat                       *xalign,
                                                     gfloat                       *yalign);

CTK_SOURCE_AVAILABLE_IN_ALL
void     ctk_source_gutter_renderer_set_alignment   (CtkSourceGutterRenderer      *renderer,
                                                     gfloat                        xalign,
                                                     gfloat                        yalign);

CTK_SOURCE_AVAILABLE_IN_ALL
void     ctk_source_gutter_renderer_set_alignment_mode (CtkSourceGutterRenderer              *renderer,
                                                        CtkSourceGutterRendererAlignmentMode  mode);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkTextWindowType
	ctk_source_gutter_renderer_get_window_type  (CtkSourceGutterRenderer      *renderer);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkTextView *ctk_source_gutter_renderer_get_view    (CtkSourceGutterRenderer      *renderer);

CTK_SOURCE_AVAILABLE_IN_ALL
CtkSourceGutterRendererAlignmentMode
	ctk_source_gutter_renderer_get_alignment_mode (CtkSourceGutterRenderer    *renderer);

CTK_SOURCE_AVAILABLE_IN_ALL
gboolean ctk_source_gutter_renderer_get_background  (CtkSourceGutterRenderer      *renderer,
                                                     CdkRGBA                      *color);

CTK_SOURCE_AVAILABLE_IN_ALL
void     ctk_source_gutter_renderer_set_background  (CtkSourceGutterRenderer      *renderer,
                                                     const CdkRGBA                *color);

/* Emits the 'activate' signal */
CTK_SOURCE_AVAILABLE_IN_ALL
void     ctk_source_gutter_renderer_activate        (CtkSourceGutterRenderer      *renderer,
                                                     CtkTextIter                  *iter,
                                                     CdkRectangle                 *area,
                                                     CdkEvent                     *event);

/* Emits the 'query-activatable' signal */
CTK_SOURCE_AVAILABLE_IN_ALL
gboolean ctk_source_gutter_renderer_query_activatable (CtkSourceGutterRenderer      *renderer,
                                                       CtkTextIter                  *iter,
                                                       CdkRectangle                 *area,
                                                       CdkEvent                     *event);

/* Emits the 'queue-draw' signal */
CTK_SOURCE_AVAILABLE_IN_ALL
void     ctk_source_gutter_renderer_queue_draw      (CtkSourceGutterRenderer      *renderer);

/* Emits the 'query-tooltip' signal */
CTK_SOURCE_AVAILABLE_IN_ALL
gboolean ctk_source_gutter_renderer_query_tooltip   (CtkSourceGutterRenderer      *renderer,
                                                     CtkTextIter                  *iter,
                                                     CdkRectangle                 *area,
                                                     gint                          x,
                                                     gint                          y,
                                                     CtkTooltip                   *tooltip);

/* Emits the 'query-data' signal */
CTK_SOURCE_AVAILABLE_IN_ALL
void     ctk_source_gutter_renderer_query_data      (CtkSourceGutterRenderer      *renderer,
                                                     CtkTextIter                  *start,
                                                     CtkTextIter                  *end,
                                                     CtkSourceGutterRendererState  state);

G_END_DECLS

#endif /* CTK_SOURCE_GUTTER_RENDERER_H */
