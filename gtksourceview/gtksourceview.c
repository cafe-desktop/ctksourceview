/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2001 - Mikael Hermansson <tyan@linux.se> and
 *                      Chris Phelps <chicane@reninet.com>
 * Copyright (C) 2002 - Jeroen Zwartepoorte
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourceview.h"

#include <string.h> /* For strlen */
#include <fribidi.h>
#include <ctk/ctk.h>
#include <cdk/cdkkeysyms.h>
#include <pango/pango-tabs.h>
#include <glib/gi18n-lib.h>

#include "ctksourcebuffer.h"
#include "ctksourcebuffer-private.h"
#include "ctksourcebufferinternal.h"
#include "ctksource-enumtypes.h"
#include "ctksourcemark.h"
#include "ctksourcemarkattributes.h"
#include "ctksource-marshal.h"
#include "ctksourcestylescheme.h"
#include "ctksourcecompletion.h"
#include "ctksourcecompletion-private.h"
#include "ctksourcecompletionprovider.h"
#include "ctksourcegutter.h"
#include "ctksourcegutter-private.h"
#include "ctksourcegutterrendererlines.h"
#include "ctksourcegutterrenderermarks.h"
#include "ctksourceiter.h"
#include "ctksourcesearchcontext.h"
#include "ctksourcespacedrawer.h"
#include "ctksourcespacedrawer-private.h"

/**
 * SECTION:view
 * @Short_description: Subclass of #CtkTextView
 * @Title: CtkSourceView
 * @See_also: #CtkTextView, #CtkSourceBuffer
 *
 * #CtkSourceView is the main class of the CtkSourceView library.
 * Use a #CtkSourceBuffer to display text with a #CtkSourceView.
 *
 * This class provides:
 *  - Show the line numbers;
 *  - Show a right margin;
 *  - Highlight the current line;
 *  - Indentation settings;
 *  - Configuration for the Home and End keyboard keys;
 *  - Configure and show line marks;
 *  - And a few other things.
 *
 * An easy way to test all these features is to use the test-widget mini-program
 * provided in the CtkSourceView repository, in the tests/ directory.
 *
 * # CtkSourceView as CtkBuildable
 *
 * The CtkSourceView implementation of the #CtkBuildable interface exposes the
 * #CtkSourceView:completion object with the internal-child "completion".
 *
 * An example of a UI definition fragment with CtkSourceView:
 * |[
 * <object class="CtkSourceView" id="source_view">
 *   <property name="tab_width">4</property>
 *   <property name="auto_indent">True</property>
 *   <child internal-child="completion">
 *     <object class="CtkSourceCompletion">
 *       <property name="select_on_show">False</property>
 *     </object>
 *   </child>
 * </object>
 * ]|
 *
 * # Changing the Font
 *
 * Ctk CSS provides the best way to change the font for a #CtkSourceView in a
 * manner that allows for components like #CtkSourceMap to scale the desired
 * font.
 *
 * |[
 *   CtkCssProvider *provider = ctk_css_provider_new ();
 *   ctk_css_provider_load_from_data (provider,
 *                                    "textview { font-family: Monospace; font-size: 8pt; }",
 *                                    -1,
 *                                    NULL);
 *   ctk_style_context_add_provider (ctk_widget_get_style_context (view),
 *                                   CTK_STYLE_PROVIDER (provider),
 *                                   CTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
 *   g_object_unref (provider);
 * ]|
 *
 * If you need to adjust the font or size of font within a portion of the
 * document only, you should use a #CtkTextTag with the #CtkTextTag:family or
 * #CtkTextTag:scale set so that the font size may be scaled relative to
 * the default font set in CSS.
 */

/*
#define ENABLE_DEBUG
*/
#undef ENABLE_DEBUG

/*
#define ENABLE_PROFILE
*/
#undef ENABLE_PROFILE

#ifdef ENABLE_DEBUG
#define DEBUG(x) (x)
#else
#define DEBUG(x)
#endif

#ifdef ENABLE_PROFILE
#define PROFILE(x) (x)
#else
#define PROFILE(x)
#endif

#define GUTTER_PIXMAP 			16
#define DEFAULT_TAB_WIDTH 		8
#define MAX_TAB_WIDTH			32
#define MAX_INDENT_WIDTH		32

#define DEFAULT_RIGHT_MARGIN_POSITION	80
#define MAX_RIGHT_MARGIN_POSITION	1000

#define RIGHT_MARGIN_LINE_ALPHA		40
#define RIGHT_MARGIN_OVERLAY_ALPHA	15

enum
{
	UNDO,
	REDO,
	SHOW_COMPLETION,
	LINE_MARK_ACTIVATED,
	MOVE_LINES,
	MOVE_WORDS,
	SMART_HOME_END,
	MOVE_TO_MATCHING_BRACKET,
	CHANGE_NUMBER,
	CHANGE_CASE,
	JOIN_LINES,
	N_SIGNALS
};

enum
{
	PROP_0,
	PROP_COMPLETION,
	PROP_SHOW_LINE_NUMBERS,
	PROP_SHOW_LINE_MARKS,
	PROP_TAB_WIDTH,
	PROP_INDENT_WIDTH,
	PROP_AUTO_INDENT,
	PROP_INSERT_SPACES,
	PROP_SHOW_RIGHT_MARGIN,
	PROP_RIGHT_MARGIN_POSITION,
	PROP_SMART_HOME_END,
	PROP_HIGHLIGHT_CURRENT_LINE,
	PROP_INDENT_ON_TAB,
	PROP_BACKGROUND_PATTERN,
	PROP_SMART_BACKSPACE,
	PROP_SPACE_DRAWER
};

struct _CtkSourceViewPrivate
{
	CtkSourceStyleScheme *style_scheme;
	CdkRGBA *right_margin_line_color;
	CdkRGBA *right_margin_overlay_color;

	CtkSourceSpaceDrawer *space_drawer;

	GHashTable *mark_categories;

	CtkSourceBuffer *source_buffer;

	CtkSourceGutter *left_gutter;
	CtkSourceGutter *right_gutter;

	CtkSourceGutterRenderer *line_renderer;
	CtkSourceGutterRenderer *marks_renderer;

	CdkRGBA current_line_color;

	CtkSourceCompletion *completion;

	guint right_margin_pos;
	gint cached_right_margin_pos;
	guint tab_width;
	gint indent_width;
	CtkSourceSmartHomeEndType smart_home_end;
	CtkSourceBackgroundPatternType background_pattern;
	CdkRGBA background_pattern_color;

	guint tabs_set : 1;
	guint show_line_numbers : 1;
	guint show_line_marks : 1;
	guint auto_indent : 1;
	guint insert_spaces : 1;
	guint highlight_current_line : 1;
	guint indent_on_tab : 1;
	guint show_right_margin  : 1;
	guint current_line_color_set : 1;
	guint background_pattern_color_set : 1;
	guint smart_backspace : 1;
};

typedef struct _MarkCategory MarkCategory;

struct _MarkCategory
{
	CtkSourceMarkAttributes *attributes;
	gint priority;
};

static guint signals[N_SIGNALS];

static void ctk_source_view_buildable_interface_init (CtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (CtkSourceView, ctk_source_view, CTK_TYPE_TEXT_VIEW,
			 G_ADD_PRIVATE (CtkSourceView)
			 G_IMPLEMENT_INTERFACE (CTK_TYPE_BUILDABLE,
						ctk_source_view_buildable_interface_init))

/* Implement DnD for application/x-color drops */
typedef enum _CtkSourceViewDropTypes {
	TARGET_COLOR = 200
} CtkSourceViewDropTypes;

static const CtkTargetEntry drop_types[] = {
	{(gchar *)"application/x-color", 0, TARGET_COLOR}
};

/* Prototypes. */
static void 	ctk_source_view_dispose			(GObject            *object);
static void 	ctk_source_view_finalize		(GObject            *object);
static void	ctk_source_view_undo 			(CtkSourceView      *view);
static void	ctk_source_view_redo 			(CtkSourceView      *view);
static void	ctk_source_view_show_completion_real	(CtkSourceView      *view);
static CtkTextBuffer * ctk_source_view_create_buffer	(CtkTextView        *view);
static void	remove_source_buffer			(CtkSourceView      *view);
static void	set_source_buffer			(CtkSourceView      *view,
							 CtkTextBuffer      *buffer);
static void	ctk_source_view_populate_popup 		(CtkTextView        *view,
							 CtkWidget          *popup);
static void	ctk_source_view_move_cursor		(CtkTextView        *text_view,
							 CtkMovementStep     step,
							 gint                count,
							 gboolean            extend_selection);
static void	ctk_source_view_delete_from_cursor	(CtkTextView        *text_view,
							 CtkDeleteType       type,
							 gint                count);
static gboolean ctk_source_view_extend_selection	(CtkTextView            *text_view,
							 CtkTextExtendSelection  granularity,
							 const CtkTextIter      *location,
							 CtkTextIter            *start,
							 CtkTextIter            *end);
static void 	ctk_source_view_get_lines		(CtkTextView       *text_view,
							 gint               first_y,
							 gint               last_y,
							 GArray            *buffer_coords,
							 GArray            *line_heights,
							 GArray            *numbers,
							 gint              *countp);
static gboolean ctk_source_view_draw 			(CtkWidget         *widget,
							 cairo_t           *cr);
static void	ctk_source_view_move_lines		(CtkSourceView     *view,
							 gboolean           down);
static void	ctk_source_view_move_words		(CtkSourceView     *view,
							 gint               step);
static gboolean	ctk_source_view_key_press_event		(CtkWidget         *widget,
							 CdkEventKey       *event);
static void	view_dnd_drop 				(CtkTextView       *view,
							 CdkDragContext    *context,
							 gint               x,
							 gint               y,
							 CtkSelectionData  *selection_data,
							 guint              info,
							 guint              timestamp,
							 gpointer           data);
static gint	calculate_real_tab_width 		(CtkSourceView     *view,
							 guint              tab_size,
							 gchar              c);
static void	ctk_source_view_set_property		(GObject           *object,
							 guint              prop_id,
							 const GValue      *value,
							 GParamSpec        *pspec);
static void	ctk_source_view_get_property		(GObject           *object,
							 guint              prop_id,
							 GValue            *value,
							 GParamSpec        *pspec);
static void	ctk_source_view_style_updated		(CtkWidget         *widget);
static void	ctk_source_view_update_style_scheme	(CtkSourceView     *view);
static void	ctk_source_view_draw_layer		(CtkTextView        *view,
							 CtkTextViewLayer   layer,
							 cairo_t           *cr);

static MarkCategory *mark_category_new                  (CtkSourceMarkAttributes *attributes,
                                                         gint priority);
static void          mark_category_free                 (MarkCategory *category);

static void
ctk_source_view_constructed (GObject *object)
{
	CtkSourceView *view = CTK_SOURCE_VIEW (object);

	set_source_buffer (view, ctk_text_view_get_buffer (CTK_TEXT_VIEW (view)));

	G_OBJECT_CLASS (ctk_source_view_parent_class)->constructed (object);
}

static void
ctk_source_view_move_to_matching_bracket (CtkSourceView *view,
					  gboolean       extend_selection)
{
	CtkTextView *text_view = CTK_TEXT_VIEW (view);
	CtkTextBuffer *buffer;
	CtkTextMark *insert_mark;
	CtkTextIter insert;
	CtkTextIter bracket_match;
	CtkSourceBracketMatchType result;

	buffer = ctk_text_view_get_buffer (text_view);
	insert_mark = ctk_text_buffer_get_insert (buffer);
	ctk_text_buffer_get_iter_at_mark (buffer, &insert, insert_mark);

	result = _ctk_source_buffer_find_bracket_match (CTK_SOURCE_BUFFER (buffer),
							&insert,
							NULL,
							&bracket_match);

	if (result == CTK_SOURCE_BRACKET_MATCH_FOUND)
	{
		if (extend_selection)
		{
			ctk_text_buffer_move_mark (buffer, insert_mark, &bracket_match);
		}
		else
		{
			ctk_text_buffer_place_cursor (buffer, &bracket_match);
		}

		ctk_text_view_scroll_mark_onscreen (text_view, insert_mark);
	}
}

static void
ctk_source_view_change_number (CtkSourceView *view,
			       gint           count)
{
	CtkTextView *text_view = CTK_TEXT_VIEW (view);
	CtkTextBuffer *buffer;
	CtkTextIter start, end;
	gchar *str;

	buffer = ctk_text_view_get_buffer (text_view);
	if (!CTK_SOURCE_IS_BUFFER (buffer))
	{
		return;
	}

	if (!ctk_text_buffer_get_selection_bounds (buffer, &start, &end))
	{
		if (!ctk_text_iter_starts_word (&start))
		{
			CtkTextIter prev;

			ctk_text_iter_backward_word_start (&start);

			/* Include the negative sign if there is one.
			 * https://gitlab.gnome.org/GNOME/ctksourceview/-/issues/117
			 */
			prev = start;
			if (ctk_text_iter_backward_char (&prev) && ctk_text_iter_get_char (&prev) == '-')
			{
				start = prev;
			}
		}

		if (!ctk_text_iter_ends_word (&end))
		{
			ctk_text_iter_forward_word_end (&end);
		}
	}

	str = ctk_text_buffer_get_text (buffer, &start, &end, FALSE);

	if (str != NULL && *str != '\0')
	{
		gchar *p;
		gint64 n;
		glong len;

		len = ctk_text_iter_get_offset (&end) - ctk_text_iter_get_offset (&start);
		g_assert (len > 0);

		n = g_ascii_strtoll (str, &p, 10);

		/* do the action only if strtoll succeeds (p != str) and
		 * the whole string is the number, e.g. not 123abc
		 */
		if ((p - str) == len)
		{
			gchar *newstr;

			newstr = g_strdup_printf ("%"G_GINT64_FORMAT, (n + count));

			ctk_text_buffer_begin_user_action (buffer);
			ctk_text_buffer_delete (buffer, &start, &end);
			ctk_text_buffer_insert (buffer, &start, newstr, -1);
			ctk_text_buffer_end_user_action (buffer);

			g_free (newstr);
		}

		g_free (str);
	}
}

static void
ctk_source_view_change_case (CtkSourceView           *view,
			     CtkSourceChangeCaseType  case_type)
{
	CtkSourceBuffer *buffer;
	CtkTextIter start, end;

	buffer = CTK_SOURCE_BUFFER (ctk_text_view_get_buffer (CTK_TEXT_VIEW (view)));

	ctk_text_view_reset_im_context (CTK_TEXT_VIEW (view));

	if (!ctk_text_buffer_get_selection_bounds (CTK_TEXT_BUFFER (buffer), &start, &end))
	{
		/* if no selection, change the current char */
		ctk_text_iter_forward_char (&end);
	}

	ctk_source_buffer_change_case (buffer, case_type, &start, &end);
}

static void
ctk_source_view_join_lines (CtkSourceView *view)
{
	CtkSourceBuffer *buffer;
	CtkTextIter start;
	CtkTextIter end;

	buffer = CTK_SOURCE_BUFFER (ctk_text_view_get_buffer (CTK_TEXT_VIEW (view)));

	ctk_text_view_reset_im_context (CTK_TEXT_VIEW (view));

	ctk_text_buffer_get_selection_bounds (CTK_TEXT_BUFFER (buffer), &start, &end);

	ctk_source_buffer_join_lines (buffer, &start, &end);
}

static void
ctk_source_view_class_init (CtkSourceViewClass *klass)
{
	GObjectClass *object_class;
	CtkTextViewClass *textview_class;
	CtkBindingSet *binding_set;
	CtkWidgetClass *widget_class;

	object_class = G_OBJECT_CLASS (klass);
	textview_class = CTK_TEXT_VIEW_CLASS (klass);
	widget_class = CTK_WIDGET_CLASS (klass);

	object_class->constructed = ctk_source_view_constructed;
	object_class->dispose = ctk_source_view_dispose;
	object_class->finalize = ctk_source_view_finalize;
	object_class->get_property = ctk_source_view_get_property;
	object_class->set_property = ctk_source_view_set_property;

	widget_class->key_press_event = ctk_source_view_key_press_event;
	widget_class->draw = ctk_source_view_draw;
	widget_class->style_updated = ctk_source_view_style_updated;

	textview_class->populate_popup = ctk_source_view_populate_popup;
	textview_class->move_cursor = ctk_source_view_move_cursor;
	textview_class->delete_from_cursor = ctk_source_view_delete_from_cursor;
	textview_class->extend_selection = ctk_source_view_extend_selection;
	textview_class->create_buffer = ctk_source_view_create_buffer;
	textview_class->draw_layer = ctk_source_view_draw_layer;

	klass->undo = ctk_source_view_undo;
	klass->redo = ctk_source_view_redo;
	klass->show_completion = ctk_source_view_show_completion_real;
	klass->move_lines = ctk_source_view_move_lines;
	klass->move_words = ctk_source_view_move_words;

	/**
	 * CtkSourceView:completion:
	 *
	 * The completion object associated with the view
	 */
	g_object_class_install_property (object_class,
					 PROP_COMPLETION,
					 g_param_spec_object ("completion",
							      "Completion",
							      "The completion object associated with the view",
							      CTK_SOURCE_TYPE_COMPLETION,
							      G_PARAM_READABLE |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceView:show-line-numbers:
	 *
	 * Whether to display line numbers
	 */
	g_object_class_install_property (object_class,
					 PROP_SHOW_LINE_NUMBERS,
					 g_param_spec_boolean ("show-line-numbers",
							       "Show Line Numbers",
							       "Whether to display line numbers",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));
	/**
	 * CtkSourceView:show-line-marks:
	 *
	 * Whether to display line mark pixbufs
	 */
	g_object_class_install_property (object_class,
					 PROP_SHOW_LINE_MARKS,
					 g_param_spec_boolean ("show-line-marks",
							       "Show Line Marks",
							       "Whether to display line mark pixbufs",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceView:tab-width:
	 *
	 * Width of a tab character expressed in number of spaces.
	 */
	g_object_class_install_property (object_class,
					 PROP_TAB_WIDTH,
					 g_param_spec_uint ("tab-width",
							    "Tab Width",
							    "Width of a tab character expressed in spaces",
							    1,
							    MAX_TAB_WIDTH,
							    DEFAULT_TAB_WIDTH,
							    G_PARAM_READWRITE |
							    G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceView:indent-width:
	 *
	 * Width of an indentation step expressed in number of spaces.
	 */
	g_object_class_install_property (object_class,
					 PROP_INDENT_WIDTH,
					 g_param_spec_int ("indent-width",
							   "Indent Width",
							   "Number of spaces to use for each step of indent",
							   -1,
							   MAX_INDENT_WIDTH,
							   -1,
							   G_PARAM_READWRITE |
							   G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class,
					 PROP_AUTO_INDENT,
					 g_param_spec_boolean ("auto-indent",
							       "Auto Indentation",
							       "Whether to enable auto indentation",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class,
					 PROP_INSERT_SPACES,
					 g_param_spec_boolean ("insert-spaces-instead-of-tabs",
							       "Insert Spaces Instead of Tabs",
							       "Whether to insert spaces instead of tabs",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceView:show-right-margin:
	 *
	 * Whether to display the right margin.
	 */
	g_object_class_install_property (object_class,
					 PROP_SHOW_RIGHT_MARGIN,
					 g_param_spec_boolean ("show-right-margin",
							       "Show Right Margin",
							       "Whether to display the right margin",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceView:right-margin-position:
	 *
	 * Position of the right margin.
	 */
	g_object_class_install_property (object_class,
					 PROP_RIGHT_MARGIN_POSITION,
					 g_param_spec_uint ("right-margin-position",
							    "Right Margin Position",
							    "Position of the right margin",
							    1,
							    MAX_RIGHT_MARGIN_POSITION,
							    DEFAULT_RIGHT_MARGIN_POSITION,
							    G_PARAM_READWRITE |
							    G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceView:smart-home-end:
	 *
	 * Set the behavior of the HOME and END keys.
	 *
	 * Since: 2.0
	 */
	g_object_class_install_property (object_class,
					 PROP_SMART_HOME_END,
					 g_param_spec_enum ("smart-home-end",
							    "Smart Home/End",
							    "HOME and END keys move to first/last "
							    "non whitespace characters on line before going "
							    "to the start/end of the line",
							    CTK_SOURCE_TYPE_SMART_HOME_END_TYPE,
							    CTK_SOURCE_SMART_HOME_END_DISABLED,
							    G_PARAM_READWRITE |
							    G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class,
					 PROP_HIGHLIGHT_CURRENT_LINE,
					 g_param_spec_boolean ("highlight-current-line",
							       "Highlight current line",
							       "Whether to highlight the current line",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class,
					 PROP_INDENT_ON_TAB,
					 g_param_spec_boolean ("indent-on-tab",
							       "Indent on tab",
							       "Whether to indent the selected text when the tab key is pressed",
							       TRUE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceView:background-pattern:
	 *
	 * Draw a specific background pattern on the view.
	 *
	 * Since: 3.16
	 */
	g_object_class_install_property (object_class,
					 PROP_BACKGROUND_PATTERN,
					 g_param_spec_enum ("background-pattern",
							    "Background pattern",
							    "Draw a specific background pattern on the view",
							    CTK_SOURCE_TYPE_BACKGROUND_PATTERN_TYPE,
							    CTK_SOURCE_BACKGROUND_PATTERN_TYPE_NONE,
							    G_PARAM_READWRITE |
							    G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceView:smart-backspace:
	 *
	 * Whether smart Backspace should be used.
	 *
	 * Since: 3.18
	 */
	g_object_class_install_property (object_class,
					 PROP_SMART_BACKSPACE,
					 g_param_spec_boolean ("smart-backspace",
							       "Smart Backspace",
							       "",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));

	/**
	 * CtkSourceView:space-drawer:
	 *
	 * The #CtkSourceSpaceDrawer object associated with the view.
	 *
	 * Since: 3.24
	 */
	g_object_class_install_property (object_class,
					 PROP_SPACE_DRAWER,
					 g_param_spec_object ("space-drawer",
							      "Space Drawer",
							      "",
							      CTK_SOURCE_TYPE_SPACE_DRAWER,
							      G_PARAM_READABLE |
							      G_PARAM_STATIC_STRINGS));

	signals[UNDO] =
		g_signal_new ("undo",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (CtkSourceViewClass, undo),
			      NULL, NULL,
		              g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
	g_signal_set_va_marshaller (signals[UNDO],
	                            G_TYPE_FROM_CLASS (klass),
	                            g_cclosure_marshal_VOID__VOIDv);

	signals[REDO] =
		g_signal_new ("redo",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (CtkSourceViewClass, redo),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
	g_signal_set_va_marshaller (signals[REDO],
	                            G_TYPE_FROM_CLASS (klass),
	                            g_cclosure_marshal_VOID__VOIDv);

	/**
	 * CtkSourceView::show-completion:
	 * @view: The #CtkSourceView who emits the signal
	 *
	 * The ::show-completion signal is a key binding signal which gets
	 * emitted when the user requests a completion, by pressing
	 * <keycombo><keycap>Control</keycap><keycap>space</keycap></keycombo>.
	 *
	 * This will create a #CtkSourceCompletionContext with the activation
	 * type as %CTK_SOURCE_COMPLETION_ACTIVATION_USER_REQUESTED.
	 *
	 * Applications should not connect to it, but may emit it with
	 * g_signal_emit_by_name() if they need to activate the completion by
	 * another means, for example with another key binding or a menu entry.
	 */
	signals[SHOW_COMPLETION] =
		g_signal_new ("show-completion",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (CtkSourceViewClass, show_completion),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
	g_signal_set_va_marshaller (signals[SHOW_COMPLETION],
	                            G_TYPE_FROM_CLASS (klass),
	                            g_cclosure_marshal_VOID__VOIDv);

	/**
	 * CtkSourceView::line-mark-activated:
	 * @view: the #CtkSourceView
	 * @iter: a #CtkTextIter
	 * @event: the #CdkEvent that activated the event
	 *
	 * Emitted when a line mark has been activated (for instance when there
	 * was a button press in the line marks gutter). You can use @iter to
	 * determine on which line the activation took place.
	 */
	signals[LINE_MARK_ACTIVATED] =
		g_signal_new ("line-mark-activated",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (CtkSourceViewClass, line_mark_activated),
			      NULL, NULL,
			      _ctk_source_marshal_VOID__BOXED_BOXED,
			      G_TYPE_NONE,
			      2,
			      CTK_TYPE_TEXT_ITER,
			      CDK_TYPE_EVENT | G_SIGNAL_TYPE_STATIC_SCOPE);
	g_signal_set_va_marshaller (signals[LINE_MARK_ACTIVATED],
	                            G_TYPE_FROM_CLASS (klass),
	                            _ctk_source_marshal_VOID__BOXED_BOXEDv);

	/**
	 * CtkSourceView::move-lines:
	 * @view: the #CtkSourceView which received the signal.
	 * @down: %TRUE to move down, %FALSE to move up.
	 *
	 * The ::move-lines signal is a keybinding which gets emitted
	 * when the user initiates moving a line. The default binding key
	 * is Alt+Up/Down arrow. And moves the currently selected lines,
	 * or the current line up or down by one line.
	 */
	signals[MOVE_LINES] =
		g_signal_new ("move-lines",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (CtkSourceViewClass, move_lines),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__BOOLEAN,
			      G_TYPE_NONE, 1,
			      G_TYPE_BOOLEAN);
	g_signal_set_va_marshaller (signals[MOVE_LINES],
	                            G_TYPE_FROM_CLASS (klass),
	                            g_cclosure_marshal_VOID__BOOLEANv);

	/**
	 * CtkSourceView::move-words:
	 * @view: the #CtkSourceView which received the signal
	 * @count: the number of words to move over
	 *
	 * The ::move-words signal is a keybinding which gets emitted
	 * when the user initiates moving a word. The default binding key
	 * is Alt+Left/Right Arrow and moves the current selection, or the current
	 * word by one word.
	 *
	 * Since: 3.0
	 */
	signals[MOVE_WORDS] =
		g_signal_new ("move-words",
			      G_TYPE_FROM_CLASS (klass),
			      G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
			      G_STRUCT_OFFSET (CtkSourceViewClass, move_words),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__INT,
			      G_TYPE_NONE, 1,
			      G_TYPE_INT);
	g_signal_set_va_marshaller (signals[MOVE_WORDS],
	                            G_TYPE_FROM_CLASS (klass),
	                            g_cclosure_marshal_VOID__INTv);

	/**
	 * CtkSourceView::smart-home-end:
	 * @view: the #CtkSourceView
	 * @iter: a #CtkTextIter
	 * @count: the count
	 *
	 * Emitted when a the cursor was moved according to the smart home
	 * end setting. The signal is emitted after the cursor is moved, but
	 * during the CtkTextView::move-cursor action. This can be used to find
	 * out whether the cursor was moved by a normal home/end or by a smart
	 * home/end.
	 *
	 * Since: 3.0
	 */
	signals[SMART_HOME_END] =
		g_signal_new ("smart-home-end",
		              G_TYPE_FROM_CLASS (klass),
		              G_SIGNAL_RUN_LAST,
		              0,
		              NULL, NULL,
		              _ctk_source_marshal_VOID__BOXED_INT,
		              G_TYPE_NONE,
		              2,
		              CTK_TYPE_TEXT_ITER,
		              G_TYPE_INT);
	g_signal_set_va_marshaller (signals[SMART_HOME_END],
	                            G_TYPE_FROM_CLASS (klass),
	                            _ctk_source_marshal_VOID__BOXED_INTv);

	/**
	 * CtkSourceView::move-to-matching-bracket:
	 * @view: the #CtkSourceView
	 * @extend_selection: %TRUE if the move should extend the selection
	 *
	 * Keybinding signal to move the cursor to the matching bracket.
	 *
	 * Since: 3.16
	 */
	signals[MOVE_TO_MATCHING_BRACKET] =
		/* we have to do it this way since we do not have any more vfunc slots */
		g_signal_new_class_handler ("move-to-matching-bracket",
		                            G_TYPE_FROM_CLASS (klass),
		                            G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		                            G_CALLBACK (ctk_source_view_move_to_matching_bracket),
		                            NULL, NULL,
		                            g_cclosure_marshal_VOID__BOOLEAN,
		                            G_TYPE_NONE,
		                            1,
		                            G_TYPE_BOOLEAN);
	g_signal_set_va_marshaller (signals[MOVE_TO_MATCHING_BRACKET],
	                            G_TYPE_FROM_CLASS (klass),
	                            g_cclosure_marshal_VOID__BOOLEANv);

	/**
	 * CtkSourceView::change-number:
	 * @view: the #CtkSourceView
	 * @count: the number to add to the number at the current position
	 *
	 * Keybinding signal to edit a number at the current cursor position.
	 *
	 * Since: 3.16
	 */
	signals[CHANGE_NUMBER] =
		g_signal_new_class_handler ("change-number",
		                            G_TYPE_FROM_CLASS (klass),
		                            G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		                            G_CALLBACK (ctk_source_view_change_number),
		                            NULL, NULL,
		                            g_cclosure_marshal_VOID__INT,
		                            G_TYPE_NONE,
		                            1,
		                            G_TYPE_INT);
	g_signal_set_va_marshaller (signals[CHANGE_NUMBER],
	                            G_TYPE_FROM_CLASS (klass),
	                            g_cclosure_marshal_VOID__INTv);

	/**
	 * CtkSourceView::change-case:
	 * @view: the #CtkSourceView
	 * @case_type: the case to use
	 *
	 * Keybinding signal to change case of the text at the current cursor position.
	 *
	 * Since: 3.16
	 */
	signals[CHANGE_CASE] =
		g_signal_new_class_handler ("change-case",
		                            G_TYPE_FROM_CLASS (klass),
		                            G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		                            G_CALLBACK (ctk_source_view_change_case),
		                            NULL, NULL,
		                            g_cclosure_marshal_VOID__ENUM,
		                            G_TYPE_NONE,
		                            1,
		                            CTK_SOURCE_TYPE_CHANGE_CASE_TYPE);
	g_signal_set_va_marshaller (signals[CHANGE_CASE],
	                            G_TYPE_FROM_CLASS (klass),
	                            g_cclosure_marshal_VOID__ENUMv);

	/**
	 * CtkSourceView::join-lines:
	 * @view: the #CtkSourceView
	 *
	 * Keybinding signal to join the lines currently selected.
	 *
	 * Since: 3.16
	 */
	signals[JOIN_LINES] =
		g_signal_new_class_handler ("join-lines",
		                            G_TYPE_FROM_CLASS (klass),
		                            G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		                            G_CALLBACK (ctk_source_view_join_lines),
		                            NULL, NULL,
		                            g_cclosure_marshal_VOID__VOID,
		                            G_TYPE_NONE,
		                            0);
	g_signal_set_va_marshaller (signals[JOIN_LINES],
	                            G_TYPE_FROM_CLASS (klass),
	                            g_cclosure_marshal_VOID__VOIDv);

	binding_set = ctk_binding_set_by_class (klass);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_z,
				      CDK_CONTROL_MASK,
				      "undo", 0);
	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_z,
				      CDK_CONTROL_MASK | CDK_SHIFT_MASK,
				      "redo", 0);
	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_F14,
				      0,
				      "undo", 0);
	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_space,
				      CDK_CONTROL_MASK,
				      "show-completion", 0);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_Up,
				      CDK_MOD1_MASK,
				      "move-lines", 1,
				      G_TYPE_BOOLEAN, FALSE);
	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_KP_Up,
				      CDK_MOD1_MASK,
				      "move-lines", 1,
				      G_TYPE_BOOLEAN, FALSE);
	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_Down,
				      CDK_MOD1_MASK,
				      "move-lines", 1,
				      G_TYPE_BOOLEAN, TRUE);
	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_KP_Down,
				      CDK_MOD1_MASK,
				      "move-lines", 1,
				      G_TYPE_BOOLEAN, TRUE);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_Left,
				      CDK_MOD1_MASK,
				      "move-words", 1,
				      G_TYPE_INT, -1);
	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_KP_Left,
				      CDK_MOD1_MASK,
				      "move-words", 1,
				      G_TYPE_INT, -1);
	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_Right,
				      CDK_MOD1_MASK,
				      "move-words", 1,
				      G_TYPE_INT, 1);
	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_KP_Right,
				      CDK_MOD1_MASK,
				      "move-words", 1,
				      G_TYPE_INT, 1);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_Up,
				      CDK_MOD1_MASK | CDK_SHIFT_MASK,
				      "move-viewport", 2,
				      CTK_TYPE_SCROLL_STEP, CTK_SCROLL_STEPS,
				      G_TYPE_INT, -1);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_KP_Up,
				      CDK_MOD1_MASK | CDK_SHIFT_MASK,
				      "move-viewport", 2,
				      CTK_TYPE_SCROLL_STEP, CTK_SCROLL_STEPS,
				      G_TYPE_INT, -1);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_Down,
				      CDK_MOD1_MASK | CDK_SHIFT_MASK,
				      "move-viewport", 2,
				      CTK_TYPE_SCROLL_STEP, CTK_SCROLL_STEPS,
				      G_TYPE_INT, 1);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_KP_Down,
				      CDK_MOD1_MASK | CDK_SHIFT_MASK,
				      "move-viewport", 2,
				      CTK_TYPE_SCROLL_STEP, CTK_SCROLL_STEPS,
				      G_TYPE_INT, 1);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_Page_Up,
				      CDK_MOD1_MASK | CDK_SHIFT_MASK,
				      "move-viewport", 2,
				      CTK_TYPE_SCROLL_STEP, CTK_SCROLL_PAGES,
				      G_TYPE_INT, -1);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_KP_Page_Up,
				      CDK_MOD1_MASK | CDK_SHIFT_MASK,
				      "move-viewport", 2,
				      CTK_TYPE_SCROLL_STEP, CTK_SCROLL_PAGES,
				      G_TYPE_INT, -1);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_Page_Down,
				      CDK_MOD1_MASK | CDK_SHIFT_MASK,
				      "move-viewport", 2,
				      CTK_TYPE_SCROLL_STEP, CTK_SCROLL_PAGES,
				      G_TYPE_INT, 1);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_KP_Page_Down,
				      CDK_MOD1_MASK | CDK_SHIFT_MASK,
				      "move-viewport", 2,
				      CTK_TYPE_SCROLL_STEP, CTK_SCROLL_PAGES,
				      G_TYPE_INT, 1);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_Home,
				      CDK_MOD1_MASK | CDK_SHIFT_MASK,
				      "move-viewport", 2,
				      CTK_TYPE_SCROLL_STEP, CTK_SCROLL_ENDS,
				      G_TYPE_INT, -1);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_KP_Home,
				      CDK_MOD1_MASK | CDK_SHIFT_MASK,
				      "move-viewport", 2,
				      CTK_TYPE_SCROLL_STEP, CTK_SCROLL_ENDS,
				      G_TYPE_INT, -1);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_End,
				      CDK_MOD1_MASK | CDK_SHIFT_MASK,
				      "move-viewport", 2,
				      CTK_TYPE_SCROLL_STEP, CTK_SCROLL_ENDS,
				      G_TYPE_INT, 1);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_KP_End,
				      CDK_MOD1_MASK | CDK_SHIFT_MASK,
				      "move-viewport", 2,
				      CTK_TYPE_SCROLL_STEP, CTK_SCROLL_ENDS,
				      G_TYPE_INT, 1);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_percent,
				      CDK_CONTROL_MASK,
				      "move-to-matching-bracket", 1,
				      G_TYPE_BOOLEAN, FALSE);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_a,
				      CDK_CONTROL_MASK | CDK_SHIFT_MASK,
				      "change-number", 1,
				      G_TYPE_INT, 1);

	ctk_binding_entry_add_signal (binding_set,
				      CDK_KEY_x,
				      CDK_CONTROL_MASK | CDK_SHIFT_MASK,
				      "change-number", 1,
				      G_TYPE_INT, -1);
}

static GObject *
ctk_source_view_buildable_get_internal_child (CtkBuildable *buildable,
					      CtkBuilder   *builder,
					      const gchar  *childname)
{
	CtkSourceView *view = CTK_SOURCE_VIEW (buildable);

	if (g_strcmp0 (childname, "completion") == 0)
	{
		return G_OBJECT (ctk_source_view_get_completion (view));
	}

	return NULL;
}

static void
ctk_source_view_buildable_interface_init (CtkBuildableIface *iface)
{
	iface->get_internal_child = ctk_source_view_buildable_get_internal_child;
}

static void
ctk_source_view_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
	CtkSourceView *view;

	g_return_if_fail (CTK_SOURCE_IS_VIEW (object));

	view = CTK_SOURCE_VIEW (object);

	switch (prop_id)
	{
		case PROP_SHOW_LINE_NUMBERS:
			ctk_source_view_set_show_line_numbers (view, g_value_get_boolean (value));
			break;

		case PROP_SHOW_LINE_MARKS:
			ctk_source_view_set_show_line_marks (view, g_value_get_boolean (value));
			break;

		case PROP_TAB_WIDTH:
			ctk_source_view_set_tab_width (view, g_value_get_uint (value));
			break;

		case PROP_INDENT_WIDTH:
			ctk_source_view_set_indent_width (view, g_value_get_int (value));
			break;

		case PROP_AUTO_INDENT:
			ctk_source_view_set_auto_indent (view, g_value_get_boolean (value));
			break;

		case PROP_INSERT_SPACES:
			ctk_source_view_set_insert_spaces_instead_of_tabs (view, g_value_get_boolean (value));
			break;

		case PROP_SHOW_RIGHT_MARGIN:
			ctk_source_view_set_show_right_margin (view, g_value_get_boolean (value));
			break;

		case PROP_RIGHT_MARGIN_POSITION:
			ctk_source_view_set_right_margin_position (view, g_value_get_uint (value));
			break;

		case PROP_SMART_HOME_END:
			ctk_source_view_set_smart_home_end (view, g_value_get_enum (value));
			break;

		case PROP_HIGHLIGHT_CURRENT_LINE:
			ctk_source_view_set_highlight_current_line (view, g_value_get_boolean (value));
			break;

		case PROP_INDENT_ON_TAB:
			ctk_source_view_set_indent_on_tab (view, g_value_get_boolean (value));
			break;

		case PROP_BACKGROUND_PATTERN:
			ctk_source_view_set_background_pattern (view, g_value_get_enum (value));
			break;

		case PROP_SMART_BACKSPACE:
			ctk_source_view_set_smart_backspace (view, g_value_get_boolean (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_view_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
	CtkSourceView *view;

	g_return_if_fail (CTK_SOURCE_IS_VIEW (object));

	view = CTK_SOURCE_VIEW (object);

	switch (prop_id)
	{
		case PROP_COMPLETION:
			g_value_set_object (value, ctk_source_view_get_completion (view));
			break;

		case PROP_SHOW_LINE_NUMBERS:
			g_value_set_boolean (value, ctk_source_view_get_show_line_numbers (view));
			break;

		case PROP_SHOW_LINE_MARKS:
			g_value_set_boolean (value, ctk_source_view_get_show_line_marks (view));
			break;

		case PROP_TAB_WIDTH:
			g_value_set_uint (value, ctk_source_view_get_tab_width (view));
			break;

		case PROP_INDENT_WIDTH:
			g_value_set_int (value, ctk_source_view_get_indent_width (view));
			break;

		case PROP_AUTO_INDENT:
			g_value_set_boolean (value, ctk_source_view_get_auto_indent (view));
			break;

		case PROP_INSERT_SPACES:
			g_value_set_boolean (value, ctk_source_view_get_insert_spaces_instead_of_tabs (view));
			break;

		case PROP_SHOW_RIGHT_MARGIN:
			g_value_set_boolean (value, ctk_source_view_get_show_right_margin (view));
			break;

		case PROP_RIGHT_MARGIN_POSITION:
			g_value_set_uint (value, ctk_source_view_get_right_margin_position (view));
			break;

		case PROP_SMART_HOME_END:
			g_value_set_enum (value, ctk_source_view_get_smart_home_end (view));
			break;

		case PROP_HIGHLIGHT_CURRENT_LINE:
			g_value_set_boolean (value, ctk_source_view_get_highlight_current_line (view));
			break;

		case PROP_INDENT_ON_TAB:
			g_value_set_boolean (value, ctk_source_view_get_indent_on_tab (view));
			break;

		case PROP_BACKGROUND_PATTERN:
			g_value_set_enum (value, ctk_source_view_get_background_pattern (view));
			break;

		case PROP_SMART_BACKSPACE:
			g_value_set_boolean (value, ctk_source_view_get_smart_backspace (view));
			break;

		case PROP_SPACE_DRAWER:
			g_value_set_object (value, ctk_source_view_get_space_drawer (view));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
space_drawer_notify_cb (CtkSourceSpaceDrawer *space_drawer,
			GParamSpec           *pspec,
			CtkSourceView        *view)
{
	ctk_widget_queue_draw (CTK_WIDGET (view));
}

static void
notify_buffer_cb (CtkSourceView *view)
{
	set_source_buffer (view, ctk_text_view_get_buffer (CTK_TEXT_VIEW (view)));
}

static void
ctk_source_view_init (CtkSourceView *view)
{
	CtkStyleContext *context;

	CtkTargetList *target_list;

	view->priv = ctk_source_view_get_instance_private (view);

	view->priv->tab_width = DEFAULT_TAB_WIDTH;
	view->priv->tabs_set = FALSE;
	view->priv->indent_width = -1;
	view->priv->indent_on_tab = TRUE;
	view->priv->smart_home_end = CTK_SOURCE_SMART_HOME_END_DISABLED;
	view->priv->right_margin_pos = DEFAULT_RIGHT_MARGIN_POSITION;
	view->priv->cached_right_margin_pos = -1;

	ctk_text_view_set_left_margin (CTK_TEXT_VIEW (view), 2);
	ctk_text_view_set_right_margin (CTK_TEXT_VIEW (view), 2);

	view->priv->right_margin_line_color = NULL;
	view->priv->right_margin_overlay_color = NULL;

	view->priv->space_drawer = ctk_source_space_drawer_new ();
	g_signal_connect_object (view->priv->space_drawer,
				 "notify",
				 G_CALLBACK (space_drawer_notify_cb),
				 view,
				 0);

	view->priv->mark_categories = g_hash_table_new_full (g_str_hash,
	                                                     g_str_equal,
	                                                     (GDestroyNotify) g_free,
	                                                     (GDestroyNotify) mark_category_free);

	target_list = ctk_drag_dest_get_target_list (CTK_WIDGET (view));
	g_return_if_fail (target_list != NULL);

	ctk_target_list_add_table (target_list, drop_types, G_N_ELEMENTS (drop_types));

	ctk_widget_set_has_tooltip (CTK_WIDGET (view), TRUE);

	g_signal_connect (view,
			  "drag_data_received",
			  G_CALLBACK (view_dnd_drop),
			  NULL);

	g_signal_connect (view,
			  "notify::buffer",
			  G_CALLBACK (notify_buffer_cb),
			  NULL);

	context = ctk_widget_get_style_context (CTK_WIDGET (view));
	ctk_style_context_add_class (context, "sourceview");
}

static void
ctk_source_view_dispose (GObject *object)
{
	CtkSourceView *view = CTK_SOURCE_VIEW (object);

	g_clear_object (&view->priv->completion);
	g_clear_object (&view->priv->left_gutter);
	g_clear_object (&view->priv->right_gutter);
	g_clear_object (&view->priv->style_scheme);
	g_clear_object (&view->priv->space_drawer);

	remove_source_buffer (view);

	/* Disconnect notify buffer because the destroy of the textview will set
	 * the buffer to NULL, and we call get_buffer in the notify which would
	 * reinstate a buffer which we don't want.
	 * There is no problem calling g_signal_handlers_disconnect_by_func()
	 * several times (if dispose() is called several times).
	 */
	g_signal_handlers_disconnect_by_func (view, notify_buffer_cb, NULL);

	G_OBJECT_CLASS (ctk_source_view_parent_class)->dispose (object);
}

static void
ctk_source_view_finalize (GObject *object)
{
	CtkSourceView *view = CTK_SOURCE_VIEW (object);

	if (view->priv->right_margin_line_color != NULL)
	{
		cdk_rgba_free (view->priv->right_margin_line_color);
	}

	if (view->priv->right_margin_overlay_color != NULL)
	{
		cdk_rgba_free (view->priv->right_margin_overlay_color);
	}

	if (view->priv->mark_categories)
	{
		g_hash_table_destroy (view->priv->mark_categories);
	}

	G_OBJECT_CLASS (ctk_source_view_parent_class)->finalize (object);
}

static void
get_visible_region (CtkTextView *text_view,
		    CtkTextIter *start,
		    CtkTextIter *end)
{
	CdkRectangle visible_rect;

	ctk_text_view_get_visible_rect (text_view, &visible_rect);

	ctk_text_view_get_line_at_y (text_view,
				     start,
				     visible_rect.y,
				     NULL);

	ctk_text_view_get_line_at_y (text_view,
				     end,
				     visible_rect.y + visible_rect.height,
				     NULL);

	ctk_text_iter_backward_line (start);
	ctk_text_iter_forward_line (end);
}

static void
highlight_updated_cb (CtkSourceBuffer *buffer,
		      CtkTextIter     *_start,
		      CtkTextIter     *_end,
		      CtkTextView     *text_view)
{
	CtkTextIter start;
	CtkTextIter end;
	CtkTextIter visible_start;
	CtkTextIter visible_end;
	CtkTextIter intersect_start;
	CtkTextIter intersect_end;

#if 0
	{
		static gint nth_call = 0;

		g_message ("%s(view=%p) %d [%d-%d]",
			   G_STRFUNC,
			   text_view,
			   ++nth_call,
			   ctk_text_iter_get_offset (_start),
			   ctk_text_iter_get_offset (_end));
	}
#endif

	start = *_start;
	end = *_end;
	ctk_text_iter_order (&start, &end);

	get_visible_region (text_view, &visible_start, &visible_end);

	if (ctk_text_iter_compare (&end, &visible_start) < 0 ||
	    ctk_text_iter_compare (&visible_end, &start) < 0)
	{
		return;
	}

	if (ctk_text_iter_compare (&start, &visible_start) < 0)
	{
		intersect_start = visible_start;
	}
	else
	{
		intersect_start = start;
	}

	if (ctk_text_iter_compare (&visible_end, &end) < 0)
	{
		intersect_end = visible_end;
	}
	else
	{
		intersect_end = end;
	}

	/* CtkSourceContextEngine sends the highlight-updated signal to notify
	 * the view, and in the view (here) we tell the ContextEngine to update
	 * the highlighting, but only in the visible area. It seems that the
	 * purpose is to reduce the number of tags that the ContextEngine
	 * applies to the buffer.
	 *
	 * A previous implementation of this signal handler queued a redraw on
	 * the view with ctk_widget_queue_draw_area(), instead of calling
	 * directly _ctk_source_buffer_update_syntax_highlight(). The ::draw
	 * handler also calls _ctk_source_buffer_update_syntax_highlight(), so
	 * this had the desired effect, but it was less clear.
	 * See the Git commit 949cd128064201935f90d999544e6a19f8e3baa6.
	 * And: https://bugzilla.gnome.org/show_bug.cgi?id=767565
	 */
	_ctk_source_buffer_update_syntax_highlight (buffer,
						    &intersect_start,
						    &intersect_end,
						    FALSE);
}

static void
search_start_cb (CtkSourceBufferInternal *buffer_internal,
		 CtkSourceSearchContext  *search_context,
		 CtkSourceView           *view)
{
	CtkTextIter visible_start;
	CtkTextIter visible_end;

	get_visible_region (CTK_TEXT_VIEW (view), &visible_start, &visible_end);

#ifndef G_DISABLE_ASSERT
	{
		CtkSourceBuffer *buffer_search = ctk_source_search_context_get_buffer (search_context);
		g_assert (buffer_search == view->priv->source_buffer);
	}
#endif

	_ctk_source_search_context_update_highlight (search_context,
						     &visible_start,
						     &visible_end,
						     FALSE);
}

static void
source_mark_updated_cb (CtkSourceBuffer *buffer,
			CtkSourceMark   *mark,
			CtkTextView     *text_view)
{
	/* TODO do something more intelligent here, namely
	 * invalidate only the area under the mark if possible */
	ctk_widget_queue_draw (CTK_WIDGET (text_view));
}

static void
buffer_style_scheme_changed_cb (CtkSourceBuffer *buffer,
				GParamSpec      *pspec,
				CtkSourceView   *view)
{
	ctk_source_view_update_style_scheme (view);
}

static void
implicit_trailing_newline_changed_cb (CtkSourceBuffer *buffer,
				      GParamSpec      *pspec,
				      CtkSourceView   *view)
{
	/* For drawing or not a trailing newline. */
	ctk_widget_queue_draw (CTK_WIDGET (view));
}

static void
remove_source_buffer (CtkSourceView *view)
{
	if (view->priv->source_buffer != NULL)
	{
		CtkSourceBufferInternal *buffer_internal;

		g_signal_handlers_disconnect_by_func (view->priv->source_buffer,
						      highlight_updated_cb,
						      view);

		g_signal_handlers_disconnect_by_func (view->priv->source_buffer,
						      source_mark_updated_cb,
						      view);

		g_signal_handlers_disconnect_by_func (view->priv->source_buffer,
						      buffer_style_scheme_changed_cb,
						      view);

		g_signal_handlers_disconnect_by_func (view->priv->source_buffer,
						      implicit_trailing_newline_changed_cb,
						      view);

		buffer_internal = _ctk_source_buffer_internal_get_from_buffer (view->priv->source_buffer);

		g_signal_handlers_disconnect_by_func (buffer_internal,
						      search_start_cb,
						      view);

		g_object_unref (view->priv->source_buffer);
		view->priv->source_buffer = NULL;
	}
}

static void
set_source_buffer (CtkSourceView *view,
		   CtkTextBuffer *buffer)
{
	if (buffer == (CtkTextBuffer*) view->priv->source_buffer)
	{
		return;
	}

	remove_source_buffer (view);

	if (CTK_SOURCE_IS_BUFFER (buffer))
	{
		CtkSourceBufferInternal *buffer_internal;

		view->priv->source_buffer = g_object_ref (CTK_SOURCE_BUFFER (buffer));

		g_signal_connect (buffer,
				  "highlight-updated",
				  G_CALLBACK (highlight_updated_cb),
				  view);

		g_signal_connect (buffer,
				  "source-mark-updated",
				  G_CALLBACK (source_mark_updated_cb),
				  view);

		g_signal_connect (buffer,
				  "notify::style-scheme",
				  G_CALLBACK (buffer_style_scheme_changed_cb),
				  view);

		g_signal_connect (buffer,
				  "notify::implicit-trailing-newline",
				  G_CALLBACK (implicit_trailing_newline_changed_cb),
				  view);

		buffer_internal = _ctk_source_buffer_internal_get_from_buffer (view->priv->source_buffer);

		g_signal_connect (buffer_internal,
				  "search-start",
				  G_CALLBACK (search_start_cb),
				  view);
	}

	ctk_source_view_update_style_scheme (view);
}

static void
scroll_to_insert (CtkSourceView *view,
		  CtkTextBuffer *buffer)
{
	CtkTextMark *insert;
	CtkTextIter iter;
	CdkRectangle visible, location;

	insert = ctk_text_buffer_get_insert (buffer);
	ctk_text_buffer_get_iter_at_mark (buffer, &iter, insert);

	ctk_text_view_get_visible_rect (CTK_TEXT_VIEW (view), &visible);
	ctk_text_view_get_iter_location (CTK_TEXT_VIEW (view), &iter, &location);

	if (location.y < visible.y || visible.y + visible.height < location.y)
	{
		ctk_text_view_scroll_to_mark (CTK_TEXT_VIEW (view),
					      insert,
					      0.0,
					      TRUE,
					      0.5, 0.5);
	}
	else if (location.x < visible.x || visible.x + visible.width < location.x)
	{
		gdouble position;
		CtkAdjustment *adjustment;

		/* We revert the vertical position of the view because
		 * _scroll_to_iter will cause it to move and the
		 * insert mark is already visible vertically. */

		adjustment = ctk_scrollable_get_vadjustment (CTK_SCROLLABLE (view));
		position = ctk_adjustment_get_value (adjustment);

		/* Must use _to_iter as _to_mark scrolls in an
		 * idle handler and would prevent use from
		 * reverting the vertical position of the view. */
		ctk_text_view_scroll_to_iter (CTK_TEXT_VIEW (view),
					      &iter,
					      0.0,
					      TRUE,
					      0.5, 0.0);

		ctk_adjustment_set_value (adjustment, position);
	}
}

static void
ctk_source_view_undo (CtkSourceView *view)
{
	CtkTextBuffer *buffer;

	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	buffer = ctk_text_view_get_buffer (CTK_TEXT_VIEW (view));

	if (ctk_text_view_get_editable (CTK_TEXT_VIEW (view)) &&
	    CTK_SOURCE_IS_BUFFER (buffer) &&
	    ctk_source_buffer_can_undo (CTK_SOURCE_BUFFER (buffer)))
	{
		ctk_source_buffer_undo (CTK_SOURCE_BUFFER (buffer));
		scroll_to_insert (view, buffer);
	}
}

static void
ctk_source_view_redo (CtkSourceView *view)
{
	CtkTextBuffer *buffer;

	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	buffer = ctk_text_view_get_buffer (CTK_TEXT_VIEW (view));

	if (ctk_text_view_get_editable (CTK_TEXT_VIEW (view)) &&
	    CTK_SOURCE_IS_BUFFER (buffer) &&
	    ctk_source_buffer_can_redo (CTK_SOURCE_BUFFER (buffer)))
	{
		ctk_source_buffer_redo (CTK_SOURCE_BUFFER (buffer));
		scroll_to_insert (view, buffer);
	}
}

static void
ctk_source_view_show_completion_real (CtkSourceView *view)
{
	CtkSourceCompletion *completion;
	CtkSourceCompletionContext *context;

	completion = ctk_source_view_get_completion (view);
	context = ctk_source_completion_create_context (completion, NULL);

	ctk_source_completion_start (completion,
				     ctk_source_completion_get_providers (completion),
				     context);
}

static void
menu_item_activate_change_case_cb (CtkWidget   *menu_item,
				   CtkTextView *text_view)
{
	CtkTextBuffer *buffer;
	CtkTextIter start, end;

	buffer = ctk_text_view_get_buffer (text_view);
	if (!CTK_SOURCE_IS_BUFFER (buffer))
	{
		return;
	}

	if (ctk_text_buffer_get_selection_bounds (buffer, &start, &end))
	{
		CtkSourceChangeCaseType case_type;

		case_type = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (menu_item), "change-case"));
		ctk_source_buffer_change_case (CTK_SOURCE_BUFFER (buffer), case_type, &start, &end);
	}
}

static void
menu_item_activate_cb (CtkWidget   *menu_item,
		       CtkTextView *text_view)
{
	const gchar *ctksignal;

	ctksignal = g_object_get_data (G_OBJECT (menu_item), "ctk-signal");
	g_signal_emit_by_name (G_OBJECT (text_view), ctksignal);
}

static void
ctk_source_view_populate_popup (CtkTextView *text_view,
				CtkWidget   *popup)
{
	CtkTextBuffer *buffer;
	CtkMenuShell *menu;
	CtkWidget *menu_item;
	CtkMenuShell *case_menu;

	buffer = ctk_text_view_get_buffer (text_view);
	if (!CTK_SOURCE_IS_BUFFER (buffer))
	{
		return;
	}

	if (!CTK_IS_MENU_SHELL (popup))
	{
		return;
	}

	menu = CTK_MENU_SHELL (popup);

	if (_ctk_source_buffer_is_undo_redo_enabled (CTK_SOURCE_BUFFER (buffer)))
	{
		/* separator */
		menu_item = ctk_separator_menu_item_new ();
		ctk_menu_shell_prepend (menu, menu_item);
		ctk_widget_show (menu_item);

		/* create redo menu_item. */
		menu_item = ctk_menu_item_new_with_mnemonic (_("_Redo"));
		g_object_set_data (G_OBJECT (menu_item), "ctk-signal", (gpointer)"redo");
		g_signal_connect (G_OBJECT (menu_item), "activate",
				  G_CALLBACK (menu_item_activate_cb), text_view);
		ctk_menu_shell_prepend (menu, menu_item);
		ctk_widget_set_sensitive (menu_item,
					  (ctk_text_view_get_editable (text_view) &&
					   ctk_source_buffer_can_redo (CTK_SOURCE_BUFFER (buffer))));
		ctk_widget_show (menu_item);

		/* create undo menu_item. */
		menu_item = ctk_menu_item_new_with_mnemonic (_("_Undo"));
		g_object_set_data (G_OBJECT (menu_item), "ctk-signal", (gpointer)"undo");
		g_signal_connect (G_OBJECT (menu_item), "activate",
				  G_CALLBACK (menu_item_activate_cb), text_view);
		ctk_menu_shell_prepend (menu, menu_item);
		ctk_widget_set_sensitive (menu_item,
					  (ctk_text_view_get_editable (text_view) &&
					   ctk_source_buffer_can_undo (CTK_SOURCE_BUFFER (buffer))));
		ctk_widget_show (menu_item);
	}

	/* separator */
	menu_item = ctk_separator_menu_item_new ();
	ctk_menu_shell_append (menu, menu_item);
	ctk_widget_show (menu_item);

	/* create change case menu */
	case_menu = CTK_MENU_SHELL (ctk_menu_new ());

	menu_item = ctk_menu_item_new_with_mnemonic (_("All _Upper Case"));
	g_object_set_data (G_OBJECT (menu_item), "change-case", GINT_TO_POINTER(CTK_SOURCE_CHANGE_CASE_UPPER));
	g_signal_connect (G_OBJECT (menu_item), "activate",
			  G_CALLBACK (menu_item_activate_change_case_cb), text_view);
	ctk_menu_shell_append (case_menu, menu_item);
	ctk_widget_set_sensitive (menu_item,
				  (ctk_text_view_get_editable (text_view) &&
				   ctk_text_buffer_get_has_selection (buffer)));
	ctk_widget_show (menu_item);

	menu_item = ctk_menu_item_new_with_mnemonic (_("All _Lower Case"));
	g_object_set_data (G_OBJECT (menu_item), "change-case", GINT_TO_POINTER(CTK_SOURCE_CHANGE_CASE_LOWER));
	g_signal_connect (G_OBJECT (menu_item), "activate",
			  G_CALLBACK (menu_item_activate_change_case_cb), text_view);
	ctk_menu_shell_append (case_menu, menu_item);
	ctk_widget_set_sensitive (menu_item,
				  (ctk_text_view_get_editable (text_view) &&
				   ctk_text_buffer_get_has_selection (buffer)));
	ctk_widget_show (menu_item);

	menu_item = ctk_menu_item_new_with_mnemonic (_("_Invert Case"));
	g_object_set_data (G_OBJECT (menu_item), "change-case", GINT_TO_POINTER(CTK_SOURCE_CHANGE_CASE_TOGGLE));
	g_signal_connect (G_OBJECT (menu_item), "activate",
			  G_CALLBACK (menu_item_activate_change_case_cb), text_view);
	ctk_menu_shell_append (case_menu, menu_item);
	ctk_widget_set_sensitive (menu_item,
				  (ctk_text_view_get_editable (text_view) &&
				   ctk_text_buffer_get_has_selection (buffer)));
	ctk_widget_show (menu_item);

	menu_item = ctk_menu_item_new_with_mnemonic (_("_Title Case"));
	g_object_set_data (G_OBJECT (menu_item), "change-case", GINT_TO_POINTER(CTK_SOURCE_CHANGE_CASE_TITLE));
	g_signal_connect (G_OBJECT (menu_item), "activate",
			  G_CALLBACK (menu_item_activate_change_case_cb), text_view);
	ctk_menu_shell_append (case_menu, menu_item);
	ctk_widget_set_sensitive (menu_item,
				  (ctk_text_view_get_editable (text_view) &&
				   ctk_text_buffer_get_has_selection (buffer)));
	ctk_widget_show (menu_item);

	menu_item = ctk_menu_item_new_with_mnemonic (_("C_hange Case"));
	ctk_menu_item_set_submenu (CTK_MENU_ITEM (menu_item), CTK_WIDGET (case_menu));
	ctk_menu_shell_append (menu, menu_item);
	ctk_widget_set_sensitive (menu_item,
				  (ctk_text_view_get_editable (text_view) &&
				   ctk_text_buffer_get_has_selection (buffer)));
	ctk_widget_show (menu_item);
}

static void
move_cursor (CtkTextView       *text_view,
	     const CtkTextIter *new_location,
	     gboolean           extend_selection)
{
	CtkTextBuffer *buffer = ctk_text_view_get_buffer (text_view);
	CtkTextMark *insert = ctk_text_buffer_get_insert (buffer);

	if (extend_selection)
	{
		ctk_text_buffer_move_mark (buffer, insert, new_location);
	}
	else
	{
		ctk_text_buffer_place_cursor (buffer, new_location);
	}

	ctk_text_view_scroll_mark_onscreen (text_view, insert);
}

static void
move_to_first_char (CtkTextView *text_view,
		    CtkTextIter *iter,
		    gboolean     display_line)
{
	CtkTextIter last;

	last = *iter;

	if (display_line)
	{
		ctk_text_view_backward_display_line_start (text_view, iter);
		ctk_text_view_forward_display_line_end (text_view, &last);
	}
	else
	{
		ctk_text_iter_set_line_offset (iter, 0);

		if (!ctk_text_iter_ends_line (&last))
		{
			ctk_text_iter_forward_to_line_end (&last);
		}
	}


	while (ctk_text_iter_compare (iter, &last) < 0)
	{
		gunichar c;

		c = ctk_text_iter_get_char (iter);

		if (g_unichar_isspace (c))
		{
			if (!ctk_text_iter_forward_visible_cursor_position (iter))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
}

static void
move_to_last_char (CtkTextView *text_view,
		   CtkTextIter *iter,
		   gboolean     display_line)
{
	CtkTextIter first;

	first = *iter;

	if (display_line)
	{
		ctk_text_view_forward_display_line_end (text_view, iter);
		ctk_text_view_backward_display_line_start (text_view, &first);
	}
	else
	{
		if (!ctk_text_iter_ends_line (iter))
		{
			ctk_text_iter_forward_to_line_end (iter);
		}

		ctk_text_iter_set_line_offset (&first, 0);
	}

	while (ctk_text_iter_compare (iter, &first) > 0)
	{
		gunichar c;

		if (!ctk_text_iter_backward_visible_cursor_position (iter))
		{
			break;
		}

		c = ctk_text_iter_get_char (iter);

		if (!g_unichar_isspace (c))
		{
			/* We've gone one cursor position too far. */
			ctk_text_iter_forward_visible_cursor_position (iter);
			break;
		}
	}
}

static void
do_cursor_move_home_end (CtkTextView *text_view,
			 CtkTextIter *cur,
			 CtkTextIter *iter,
			 gboolean     extend_selection,
			 gint         count)
{
	/* if we are clearing selection, we need to move_cursor even
	 * if we are at proper iter because selection_bound may need
	 * to be moved */
	if (!ctk_text_iter_equal (cur, iter) || !extend_selection)
	{
		move_cursor (text_view, iter, extend_selection);
		g_signal_emit (text_view, signals[SMART_HOME_END], 0, iter, count);
	}
}

/* Returns %TRUE if handled. */
static gboolean
move_cursor_smart_home_end (CtkTextView     *text_view,
			    CtkMovementStep  step,
			    gint             count,
			    gboolean         extend_selection)
{
	CtkSourceView *source_view = CTK_SOURCE_VIEW (text_view);
	CtkTextBuffer *buffer = ctk_text_view_get_buffer (text_view);
	gboolean move_display_line;
	CtkTextMark *mark;
	CtkTextIter cur;
	CtkTextIter iter;

	g_assert (step == CTK_MOVEMENT_DISPLAY_LINE_ENDS ||
		  step == CTK_MOVEMENT_PARAGRAPH_ENDS);

	move_display_line = step == CTK_MOVEMENT_DISPLAY_LINE_ENDS;

	mark = ctk_text_buffer_get_insert (buffer);
	ctk_text_buffer_get_iter_at_mark (buffer, &cur, mark);
	iter = cur;

	if (count == -1)
	{
		gboolean at_home;

		move_to_first_char (text_view, &iter, move_display_line);

		if (move_display_line)
		{
			at_home = ctk_text_view_starts_display_line (text_view, &cur);
		}
		else
		{
			at_home = ctk_text_iter_starts_line (&cur);
		}

		switch (source_view->priv->smart_home_end)
		{
			case CTK_SOURCE_SMART_HOME_END_BEFORE:
				if (!ctk_text_iter_equal (&cur, &iter) || at_home)
				{
					do_cursor_move_home_end (text_view,
					                         &cur,
					                         &iter,
					                         extend_selection,
					                         count);
					return TRUE;
				}
				break;

			case CTK_SOURCE_SMART_HOME_END_AFTER:
				if (at_home)
				{
					do_cursor_move_home_end (text_view,
					                         &cur,
					                         &iter,
					                         extend_selection,
					                         count);
					return TRUE;
				}
				break;

			case CTK_SOURCE_SMART_HOME_END_ALWAYS:
				do_cursor_move_home_end (text_view,
				                         &cur,
				                         &iter,
				                         extend_selection,
				                         count);
				return TRUE;

			case CTK_SOURCE_SMART_HOME_END_DISABLED:
			default:
				break;
		}
	}
	else if (count == 1)
	{
		gboolean at_end;

		move_to_last_char (text_view, &iter, move_display_line);

		if (move_display_line)
		{
			CtkTextIter display_end;
			display_end = cur;

			ctk_text_view_forward_display_line_end (text_view, &display_end);
			at_end = ctk_text_iter_equal (&cur, &display_end);
		}
		else
		{
			at_end = ctk_text_iter_ends_line (&cur);
		}

		switch (source_view->priv->smart_home_end)
		{
			case CTK_SOURCE_SMART_HOME_END_BEFORE:
				if (!ctk_text_iter_equal (&cur, &iter) || at_end)
				{
					do_cursor_move_home_end (text_view,
					                         &cur,
					                         &iter,
					                         extend_selection,
					                         count);
					return TRUE;
				}
				break;

			case CTK_SOURCE_SMART_HOME_END_AFTER:
				if (at_end)
				{
					do_cursor_move_home_end (text_view,
					                         &cur,
					                         &iter,
					                         extend_selection,
					                         count);
					return TRUE;
				}
				break;

			case CTK_SOURCE_SMART_HOME_END_ALWAYS:
				do_cursor_move_home_end (text_view,
				                         &cur,
				                         &iter,
				                         extend_selection,
				                         count);
				return TRUE;

			case CTK_SOURCE_SMART_HOME_END_DISABLED:
			default:
				break;
		}
	}

	return FALSE;
}

static void
move_cursor_words (CtkTextView *text_view,
		   gint         count,
		   gboolean     extend_selection)
{
	CtkTextBuffer *buffer;
	CtkTextIter insert;
	CtkTextIter newplace;
	CtkTextIter line_start;
	CtkTextIter line_end;
	const gchar *ptr;
	gchar *line_text;

	buffer = ctk_text_view_get_buffer (text_view);

	ctk_text_buffer_get_iter_at_mark (buffer,
					  &insert,
					  ctk_text_buffer_get_insert (buffer));

	line_start = line_end = newplace = insert;

	/* Get the text of the current line for RTL analysis */
	ctk_text_iter_set_line_offset (&line_start, 0);
	ctk_text_iter_forward_line (&line_end);
	line_text = ctk_text_iter_get_visible_text (&line_start, &line_end);

	/* Swap direction for RTL to maintain visual cursor movement.
	 * Otherwise, cursor will move in opposite direction which is counter
	 * intuitve and causes confusion for RTL users.
	 *
	 * You would think we could iterate using the textiter, but we cannot
	 * since there is no way in CtkTextIter to check if it is visible (as
	 * that is not exposed by CtkTextBTree). So we use the allocated string
	 * contents instead.
	 */
	for (ptr = line_text; *ptr; ptr = g_utf8_next_char (ptr))
	{
		gunichar ch = g_utf8_get_char (ptr);
		FriBidiCharType ch_type = fribidi_get_bidi_type (ch);

		if (FRIBIDI_IS_STRONG (ch_type))
		{
			if (FRIBIDI_IS_RTL (ch_type))
			{
				count *= -1;
			}

			break;
		}
	}

	g_free (line_text);

	if (count < 0)
	{
		if (!_ctk_source_iter_backward_visible_word_starts (&newplace, -count))
		{
			ctk_text_iter_set_line_offset (&newplace, 0);
		}
	}
	else if (count > 0)
	{
		if (!_ctk_source_iter_forward_visible_word_ends (&newplace, count))
		{
			ctk_text_iter_forward_to_line_end (&newplace);
		}
	}

	move_cursor (text_view, &newplace, extend_selection);
}

static void
ctk_source_view_move_cursor (CtkTextView     *text_view,
			     CtkMovementStep  step,
			     gint             count,
			     gboolean         extend_selection)
{
	if (!ctk_text_view_get_cursor_visible (text_view))
	{
		goto chain_up;
	}

	ctk_text_view_reset_im_context (text_view);

	switch (step)
	{
		case CTK_MOVEMENT_DISPLAY_LINE_ENDS:
		case CTK_MOVEMENT_PARAGRAPH_ENDS:
			if (move_cursor_smart_home_end (text_view, step, count, extend_selection))
			{
				return;
			}
			break;

		case CTK_MOVEMENT_WORDS:
			move_cursor_words (text_view, count, extend_selection);
			return;

		case CTK_MOVEMENT_LOGICAL_POSITIONS:
		case CTK_MOVEMENT_VISUAL_POSITIONS:
		case CTK_MOVEMENT_DISPLAY_LINES:
		case CTK_MOVEMENT_PARAGRAPHS:
		case CTK_MOVEMENT_PAGES:
		case CTK_MOVEMENT_BUFFER_ENDS:
		case CTK_MOVEMENT_HORIZONTAL_PAGES:
		default:
			break;
	}

chain_up:
	CTK_TEXT_VIEW_CLASS (ctk_source_view_parent_class)->move_cursor (text_view,
									 step,
									 count,
									 extend_selection);
}

static void
ctk_source_view_delete_from_cursor (CtkTextView   *text_view,
				    CtkDeleteType  type,
				    gint           count)
{
	CtkTextBuffer *buffer = ctk_text_view_get_buffer (text_view);
	CtkTextIter insert;
	CtkTextIter start;
	CtkTextIter end;

	if (type != CTK_DELETE_WORD_ENDS)
	{
		CTK_TEXT_VIEW_CLASS (ctk_source_view_parent_class)->delete_from_cursor (text_view,
											type,
											count);
		return;
	}

	ctk_text_view_reset_im_context (text_view);

	ctk_text_buffer_get_iter_at_mark (buffer,
					  &insert,
					  ctk_text_buffer_get_insert (buffer));

	start = insert;
	end = insert;

	if (count > 0)
	{
		if (!_ctk_source_iter_forward_visible_word_ends (&end, count))
		{
			ctk_text_iter_forward_to_line_end (&end);
		}
	}
	else
	{
		if (!_ctk_source_iter_backward_visible_word_starts (&start, -count))
		{
			ctk_text_iter_set_line_offset (&start, 0);
		}
	}

	ctk_text_buffer_delete_interactive (buffer, &start, &end,
					    ctk_text_view_get_editable (text_view));
}

static gboolean
ctk_source_view_extend_selection (CtkTextView            *text_view,
				  CtkTextExtendSelection  granularity,
				  const CtkTextIter      *location,
				  CtkTextIter            *start,
				  CtkTextIter            *end)
{
	if (granularity == CTK_TEXT_EXTEND_SELECTION_WORD)
	{
		_ctk_source_iter_extend_selection_word (location, start, end);
		return CDK_EVENT_STOP;
	}

	return CTK_TEXT_VIEW_CLASS (ctk_source_view_parent_class)->extend_selection (text_view,
										     granularity,
										     location,
										     start,
										     end);
}

static void
ctk_source_view_ensure_redrawn_rect_is_highlighted (CtkSourceView *view,
						    cairo_t       *cr)
{
	CdkRectangle clip;
	CtkTextIter iter1, iter2;

	if (view->priv->source_buffer == NULL ||
	    !cdk_cairo_get_clip_rectangle (cr, &clip))
	{
		return;
	}

	ctk_text_view_get_line_at_y (CTK_TEXT_VIEW (view), &iter1, clip.y, NULL);
	ctk_text_iter_backward_line (&iter1);
	ctk_text_view_get_line_at_y (CTK_TEXT_VIEW (view), &iter2, clip.y + clip.height, NULL);
	ctk_text_iter_forward_line (&iter2);

	DEBUG ({
		g_print ("    draw area: %d - %d\n", clip.y, clip.y + clip.height);
		g_print ("    lines to update: %d - %d\n",
			 ctk_text_iter_get_line (&iter1),
			 ctk_text_iter_get_line (&iter2));
	});

	_ctk_source_buffer_update_syntax_highlight (view->priv->source_buffer,
						    &iter1, &iter2, FALSE);
	_ctk_source_buffer_update_search_highlight (view->priv->source_buffer,
						    &iter1, &iter2, FALSE);
}

/* This function is taken from ctk+/tests/testtext.c */
static void
ctk_source_view_get_lines (CtkTextView *text_view,
			   gint         first_y,
			   gint         last_y,
			   GArray      *buffer_coords,
			   GArray      *line_heights,
			   GArray      *numbers,
			   gint        *countp)
{
	CtkTextIter iter;
	gint count;
	gint last_line_num = -1;

	g_array_set_size (buffer_coords, 0);
	g_array_set_size (numbers, 0);
	if (line_heights != NULL)
		g_array_set_size (line_heights, 0);

	/* Get iter at first y */
	ctk_text_view_get_line_at_y (text_view, &iter, first_y, NULL);

	/* For each iter, get its location and add it to the arrays.
	 * Stop when we pass last_y */
	count = 0;

	while (!ctk_text_iter_is_end (&iter))
	{
		gint y, height;

		ctk_text_view_get_line_yrange (text_view, &iter, &y, &height);

		g_array_append_val (buffer_coords, y);
		if (line_heights)
		{
			g_array_append_val (line_heights, height);
		}

		last_line_num = ctk_text_iter_get_line (&iter);
		g_array_append_val (numbers, last_line_num);

		++count;

		if ((y + height) >= last_y)
			break;

		ctk_text_iter_forward_line (&iter);
	}

	if (ctk_text_iter_is_end (&iter))
	{
		gint y, height;
		gint line_num;

		ctk_text_view_get_line_yrange (text_view, &iter, &y, &height);

		line_num = ctk_text_iter_get_line (&iter);

		if (line_num != last_line_num)
		{
			g_array_append_val (buffer_coords, y);
			if (line_heights)
				g_array_append_val (line_heights, height);
			g_array_append_val (numbers, line_num);
			++count;
		}
	}

	*countp = count;
}

/* Another solution to paint the line background is to use the
 * CtkTextTag:paragraph-background property. But there are several issues:
 * - CtkTextTags are per buffer, not per view. It's better to keep the line
 *   highlighting per view.
 * - There is a problem for empty lines: a text tag can not be applied to an
 *   empty region. And it can not be worked around easily for the last line.
 *
 * See https://bugzilla.gnome.org/show_bug.cgi?id=310847 for more details.
 */
static void
ctk_source_view_paint_line_background (CtkTextView    *text_view,
				       cairo_t        *cr,
				       int             y, /* in buffer coordinates */
				       int             height,
				       const CdkRGBA  *color)
{
	gdouble x1, y1, x2, y2;

	cairo_save (cr);
	cairo_clip_extents (cr, &x1, &y1, &x2, &y2);

	cdk_cairo_set_source_rgba (cr, (CdkRGBA *)color);
	cairo_set_line_width (cr, 1);
	cairo_rectangle (cr, x1 + .5, y + .5, x2 - x1 - 1, height - 1);
	cairo_stroke_preserve (cr);
	cairo_fill (cr);
	cairo_restore (cr);
}

static void
ctk_source_view_paint_marks_background (CtkSourceView *view,
					cairo_t       *cr)
{
	CtkTextView *text_view;
	CdkRectangle clip;
	GArray *numbers;
	GArray *pixels;
	GArray *heights;
	gint y1, y2;
	gint count;
	gint i;

	if (view->priv->source_buffer == NULL ||
	    !_ctk_source_buffer_has_source_marks (view->priv->source_buffer) ||
	    !cdk_cairo_get_clip_rectangle (cr, &clip))
	{
		return;
	}

	text_view = CTK_TEXT_VIEW (view);

	y1 = clip.y;
	y2 = y1 + clip.height;

	numbers = g_array_new (FALSE, FALSE, sizeof (gint));
	pixels = g_array_new (FALSE, FALSE, sizeof (gint));
	heights = g_array_new (FALSE, FALSE, sizeof (gint));

	/* get the line numbers and y coordinates. */
	ctk_source_view_get_lines (text_view,
	                           y1,
	                           y2,
	                           pixels,
	                           heights,
	                           numbers,
	                           &count);

	if (count == 0)
	{
		gint n = 0;
		gint y;
		gint height;
		CtkTextIter iter;

		ctk_text_buffer_get_start_iter (ctk_text_view_get_buffer (text_view), &iter);
		ctk_text_view_get_line_yrange (text_view, &iter, &y, &height);

		g_array_append_val (pixels, y);
		g_array_append_val (pixels, height);
		g_array_append_val (numbers, n);
		count = 1;
	}

	DEBUG ({
		g_print ("    Painting marks background for line numbers %d - %d\n",
		         g_array_index (numbers, gint, 0),
		         g_array_index (numbers, gint, count - 1));
	});

	for (i = 0; i < count; ++i)
	{
		gint line_to_paint;
		GSList *marks;
		CdkRGBA background;
		int priority;

		line_to_paint = g_array_index (numbers, gint, i);

		marks = ctk_source_buffer_get_source_marks_at_line (view->priv->source_buffer,
		                                                    line_to_paint,
		                                                    NULL);

		priority = -1;

		while (marks != NULL)
		{
			CtkSourceMarkAttributes *attrs;
			gint prio;
			CdkRGBA bg;

			attrs = ctk_source_view_get_mark_attributes (view,
			                                             ctk_source_mark_get_category (marks->data),
			                                             &prio);

			if (attrs != NULL &&
			    prio > priority &&
			    ctk_source_mark_attributes_get_background (attrs, &bg))
			{
				priority = prio;
				background = bg;
			}

			marks = g_slist_delete_link (marks, marks);
		}

		if (priority != -1)
		{
			ctk_source_view_paint_line_background (text_view,
			                                       cr,
			                                       g_array_index (pixels, gint, i),
			                                       g_array_index (heights, gint, i),
			                                       &background);
		}
	}

	g_array_free (heights, TRUE);
	g_array_free (pixels, TRUE);
	g_array_free (numbers, TRUE);
}

static void
ctk_source_view_paint_right_margin (CtkSourceView *view,
				    cairo_t       *cr)
{
	CdkRectangle clip;
	gdouble x;

	CtkTextView *text_view = CTK_TEXT_VIEW (view);

#ifdef ENABLE_PROFILE
	static GTimer *timer = NULL;

	if (timer == NULL)
	{
		timer = g_timer_new ();
	}

	g_timer_start (timer);
#endif

	g_return_if_fail (view->priv->right_margin_line_color != NULL);

	if (!cdk_cairo_get_clip_rectangle (cr, &clip))
	{
		return;
	}

	if (view->priv->cached_right_margin_pos < 0)
	{
		view->priv->cached_right_margin_pos =
			calculate_real_tab_width (view,
						  view->priv->right_margin_pos,
						  '_');
	}

	x = view->priv->cached_right_margin_pos + ctk_text_view_get_left_margin (text_view);

	cairo_save (cr);
	cairo_set_line_width (cr, 1.0);

	if (x + 1 >= clip.x && x <= clip.x + clip.width)
	{
		cairo_move_to (cr, x + 0.5, clip.y);
		cairo_line_to (cr, x + 0.5, clip.y + clip.height);

		cdk_cairo_set_source_rgba (cr, view->priv->right_margin_line_color);
		cairo_stroke (cr);
	}

	/* Only draw the overlay when the style scheme explicitly sets it. */
	if (view->priv->right_margin_overlay_color != NULL && clip.x + clip.width > x + 1)
	{
		/* Draw the rectangle next to the line (x+1). */
		cairo_rectangle (cr,
				 x + 1, clip.y,
				 clip.x + clip.width - (x + 1), clip.height);

		cdk_cairo_set_source_rgba (cr, view->priv->right_margin_overlay_color);
		cairo_fill (cr);
	}

	cairo_restore (cr);

	PROFILE ({
		g_timer_stop (timer);
		g_print ("    ctk_source_view_paint_right_margin time: "
			 "%g (sec * 1000)\n",
			 g_timer_elapsed (timer, NULL) * 1000);
	});
}

static gint
realign (gint  offset,
	 guint align)
{
	if (offset > 0 && align > 0)
	{
		gint padding;

		padding = (align - (offset % align)) % align;
		return offset + padding;
	}

	return 0;
}

static void
ctk_source_view_paint_background_pattern_grid (CtkSourceView *view,
					       cairo_t       *cr)
{
	CdkRectangle clip;
	gint x, y, x2, y2;
	PangoContext *context;
	PangoLayout *layout;
	gint grid_width = 16;
	gint grid_height = 16;

	context = ctk_widget_get_pango_context (CTK_WIDGET (view));
	layout = pango_layout_new (context);
	pango_layout_set_text (layout, "X", 1);
	pango_layout_get_pixel_size (layout, &grid_width, &grid_height);
	g_object_unref (layout);

	/* each character becomes 2 stacked boxes. */
	grid_height = MAX (1, grid_height / 2);
	grid_width = MAX (1, grid_width);

	cairo_save (cr);

	cdk_cairo_get_clip_rectangle (cr, &clip);

	cairo_set_line_width (cr, 1.0);
	cdk_cairo_set_source_rgba (cr, &view->priv->background_pattern_color);

	/* Align our drawing position with a multiple of the grid size. */
	x = realign (clip.x - grid_width, grid_width);
	y = realign (clip.y - grid_height, grid_height);
	x2 = realign (x + clip.width + grid_width * 2, grid_width);
	y2 = realign (y + clip.height + grid_height * 2, grid_height);

	for (; x <= x2; x += grid_width)
	{
		cairo_move_to (cr, x + .5, clip.y - .5);
		cairo_line_to (cr, x + .5, clip.y + clip.height - .5);
	}

	for (; y <= y2; y += grid_height)
	{
		cairo_move_to (cr, clip.x + .5, y - .5);
		cairo_line_to (cr, clip.x + clip.width + .5, y - .5);
	}

	cairo_stroke (cr);
	cairo_restore (cr);
}

static void
ctk_source_view_paint_current_line_highlight (CtkSourceView *view,
					      cairo_t       *cr)
{
	CtkTextBuffer *buffer;
	CtkTextIter cur;
	gint y;
	gint height;

	buffer = ctk_text_view_get_buffer (CTK_TEXT_VIEW (view));
	ctk_text_buffer_get_iter_at_mark (buffer,
					  &cur,
					  ctk_text_buffer_get_insert (buffer));
	ctk_text_view_get_line_yrange (CTK_TEXT_VIEW (view), &cur, &y, &height);

	ctk_source_view_paint_line_background (CTK_TEXT_VIEW (view),
					       cr,
					       y, height,
					       &view->priv->current_line_color);
}

static void
ctk_source_view_draw_layer (CtkTextView      *text_view,
			    CtkTextViewLayer  layer,
			    cairo_t          *cr)
{
	CtkSourceView *view;

	view = CTK_SOURCE_VIEW (text_view);

	cairo_save (cr);

	if (layer == CTK_TEXT_VIEW_LAYER_BELOW_TEXT)
	{
		ctk_source_view_ensure_redrawn_rect_is_highlighted (view, cr);

		if (view->priv->background_pattern == CTK_SOURCE_BACKGROUND_PATTERN_TYPE_GRID &&
		    view->priv->background_pattern_color_set)
		{
			ctk_source_view_paint_background_pattern_grid (view, cr);
		}

		if (ctk_widget_is_sensitive (CTK_WIDGET (view)) &&
		    view->priv->highlight_current_line &&
		    view->priv->current_line_color_set)
		{
			ctk_source_view_paint_current_line_highlight (view, cr);
		}

		ctk_source_view_paint_marks_background (view, cr);
	}
	else if (layer == CTK_TEXT_VIEW_LAYER_ABOVE_TEXT)
	{
		/* Draw the right margin vertical line + overlay. */
		if (view->priv->show_right_margin)
		{
			ctk_source_view_paint_right_margin (view, cr);
		}

		if (view->priv->space_drawer != NULL)
		{
			_ctk_source_space_drawer_draw (view->priv->space_drawer, view, cr);
		}
	}

	cairo_restore (cr);
}

static gboolean
ctk_source_view_draw (CtkWidget *widget,
		      cairo_t   *cr)
{
	CtkSourceView *view = CTK_SOURCE_VIEW (widget);
	gboolean event_handled;

#ifdef ENABLE_PROFILE
	static GTimer *timer = NULL;
	if (timer == NULL)
	{
		timer = g_timer_new ();
	}

	g_timer_start (timer);
#endif

	DEBUG ({
		g_print ("> ctk_source_view_draw start\n");
	});

	event_handled = CTK_WIDGET_CLASS (ctk_source_view_parent_class)->draw (widget, cr);

	if (view->priv->left_gutter != NULL)
	{
		_ctk_source_gutter_draw (view->priv->left_gutter, view, cr);
	}

	if (view->priv->right_gutter != NULL)
	{
		_ctk_source_gutter_draw (view->priv->right_gutter, view, cr);
	}

	PROFILE ({
		g_timer_stop (timer);
		g_print ("    ctk_source_view_draw time: %g (sec * 1000)\n",
		         g_timer_elapsed (timer, NULL) * 1000);
	});
	DEBUG ({
		g_print ("> ctk_source_view_draw end\n");
	});

	return event_handled;
}

/* This is a pretty important function... We call it when the tab_stop is changed,
 * and when the font is changed.
 * NOTE: You must change this with the default font for now...
 * It may be a good idea to set the tab_width for each CtkTextTag as well
 * based on the font that we set at creation time
 * something like style_cache_set_tabs_from_font or the like.
 * Now, this *may* not be necessary because most tabs wont be inside of another highlight,
 * except for things like multi-line comments (which will usually have an italic font which
 * may or may not be a different size than the standard one), or if some random language
 * definition decides that it would be spiffy to have a bg color for "start of line" whitespace
 * "^\(\t\| \)+" would probably do the trick for that.
 */
static gint
calculate_real_tab_width (CtkSourceView *view, guint tab_size, gchar c)
{
	PangoLayout *layout;
	gchar *tab_string;
	gint tab_width = 0;

	if (tab_size == 0)
	{
		return -1;
	}

	tab_string = g_strnfill (tab_size, c);
	layout = ctk_widget_create_pango_layout (CTK_WIDGET (view), tab_string);
	g_free (tab_string);

	if (layout != NULL)
	{
		pango_layout_get_pixel_size (layout, &tab_width, NULL);
		g_object_unref (G_OBJECT (layout));
	}
	else
	{
		tab_width = -1;
	}

	return tab_width;
}

static CtkTextBuffer *
ctk_source_view_create_buffer (CtkTextView *text_view)
{
	return CTK_TEXT_BUFFER (ctk_source_buffer_new (NULL));
}


/* ----------------------------------------------------------------------
 * Public interface
 * ----------------------------------------------------------------------
 */

/**
 * ctk_source_view_new:
 *
 * Creates a new #CtkSourceView.
 *
 * By default, an empty #CtkSourceBuffer will be lazily created and can be
 * retrieved with ctk_text_view_get_buffer().
 *
 * If you want to specify your own buffer, either override the
 * #CtkTextViewClass create_buffer factory method, or use
 * ctk_source_view_new_with_buffer().
 *
 * Returns: a new #CtkSourceView.
 */
CtkWidget *
ctk_source_view_new (void)
{
	return g_object_new (CTK_SOURCE_TYPE_VIEW, NULL);
}

/**
 * ctk_source_view_new_with_buffer:
 * @buffer: a #CtkSourceBuffer.
 *
 * Creates a new #CtkSourceView widget displaying the buffer
 * @buffer. One buffer can be shared among many widgets.
 *
 * Returns: a new #CtkSourceView.
 */
CtkWidget *
ctk_source_view_new_with_buffer (CtkSourceBuffer *buffer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), NULL);

	return g_object_new (CTK_SOURCE_TYPE_VIEW,
			     "buffer", buffer,
			     NULL);
}

/**
 * ctk_source_view_get_show_line_numbers:
 * @view: a #CtkSourceView.
 *
 * Returns whether line numbers are displayed beside the text.
 *
 * Return value: %TRUE if the line numbers are displayed.
 */
gboolean
ctk_source_view_get_show_line_numbers (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), FALSE);

	return view->priv->show_line_numbers;
}

/**
 * ctk_source_view_set_show_line_numbers:
 * @view: a #CtkSourceView.
 * @show: whether line numbers should be displayed.
 *
 * If %TRUE line numbers will be displayed beside the text.
 */
void
ctk_source_view_set_show_line_numbers (CtkSourceView *view,
				       gboolean       show)
{
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	show = show != FALSE;

	if (show == view->priv->show_line_numbers)
	{
		return;
	}

	if (view->priv->line_renderer == NULL)
	{
		CtkSourceGutter *gutter;

		gutter = ctk_source_view_get_gutter (view, CTK_TEXT_WINDOW_LEFT);

		view->priv->line_renderer = ctk_source_gutter_renderer_lines_new ();
		g_object_set (view->priv->line_renderer,
		              "alignment-mode", CTK_SOURCE_GUTTER_RENDERER_ALIGNMENT_MODE_FIRST,
		              "yalign", 0.5,
		              "xalign", 1.0,
		              "xpad", 3,
		              NULL);

		ctk_source_gutter_insert (gutter,
		                          view->priv->line_renderer,
		                          CTK_SOURCE_VIEW_GUTTER_POSITION_LINES);
	}

	ctk_source_gutter_renderer_set_visible (view->priv->line_renderer, show);
	view->priv->show_line_numbers = show;

	g_object_notify (G_OBJECT (view), "show_line_numbers");
}

/**
 * ctk_source_view_get_show_line_marks:
 * @view: a #CtkSourceView.
 *
 * Returns whether line marks are displayed beside the text.
 *
 * Return value: %TRUE if the line marks are displayed.
 *
 * Since: 2.2
 */
gboolean
ctk_source_view_get_show_line_marks (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), FALSE);

	return view->priv->show_line_marks;
}

static void
gutter_renderer_marks_activate (CtkSourceGutterRenderer *renderer,
				CtkTextIter             *iter,
				const CdkRectangle      *area,
				CdkEvent                *event,
				CtkSourceView           *view)
{
	g_signal_emit (view,
	               signals[LINE_MARK_ACTIVATED],
	               0,
	               iter,
	               event);
}

/**
 * ctk_source_view_set_show_line_marks:
 * @view: a #CtkSourceView.
 * @show: whether line marks should be displayed.
 *
 * If %TRUE line marks will be displayed beside the text.
 *
 * Since: 2.2
 */
void
ctk_source_view_set_show_line_marks (CtkSourceView *view,
				     gboolean       show)
{
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	show = show != FALSE;

	if (show == view->priv->show_line_marks)
	{
		return;
	}

	if (view->priv->marks_renderer == NULL)
	{
		CtkSourceGutter *gutter;

		gutter = ctk_source_view_get_gutter (view, CTK_TEXT_WINDOW_LEFT);

		view->priv->marks_renderer = ctk_source_gutter_renderer_marks_new ();

		ctk_source_gutter_insert (gutter,
		                          view->priv->marks_renderer,
		                          CTK_SOURCE_VIEW_GUTTER_POSITION_MARKS);

		g_signal_connect (view->priv->marks_renderer,
		                  "activate",
		                  G_CALLBACK (gutter_renderer_marks_activate),
		                  view);
	}

	ctk_source_gutter_renderer_set_visible (view->priv->marks_renderer, show);
	view->priv->show_line_marks = show;

	g_object_notify (G_OBJECT (view), "show_line_marks");
}

static gboolean
set_tab_stops_internal (CtkSourceView *view)
{
	PangoTabArray *tab_array;
	gint real_tab_width;

	real_tab_width = calculate_real_tab_width (view, view->priv->tab_width, ' ');

	if (real_tab_width < 0)
	{
		return FALSE;
	}

	tab_array = pango_tab_array_new (1, TRUE);
	pango_tab_array_set_tab (tab_array, 0, PANGO_TAB_LEFT, real_tab_width);

	ctk_text_view_set_tabs (CTK_TEXT_VIEW (view),
				tab_array);
	view->priv->tabs_set = TRUE;

	pango_tab_array_free (tab_array);

	return TRUE;
}

/**
 * ctk_source_view_set_tab_width:
 * @view: a #CtkSourceView.
 * @width: width of tab in characters.
 *
 * Sets the width of tabulation in characters. The #CtkTextBuffer still contains
 * \t characters, but they can take a different visual width in a #CtkSourceView
 * widget.
 */
void
ctk_source_view_set_tab_width (CtkSourceView *view,
			       guint          width)
{
	guint save_width;

	g_return_if_fail (CTK_SOURCE_VIEW (view));
	g_return_if_fail (0 < width && width <= MAX_TAB_WIDTH);

	if (view->priv->tab_width == width)
	{
		return;
	}

	save_width = view->priv->tab_width;
	view->priv->tab_width = width;
	if (set_tab_stops_internal (view))
	{
		g_object_notify (G_OBJECT (view), "tab-width");
	}
	else
	{
		g_warning ("Impossible to set tab width.");
		view->priv->tab_width = save_width;
	}
}

/**
 * ctk_source_view_get_tab_width:
 * @view: a #CtkSourceView.
 *
 * Returns the width of tabulation in characters.
 *
 * Return value: width of tab.
 */
guint
ctk_source_view_get_tab_width (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), DEFAULT_TAB_WIDTH);

	return view->priv->tab_width;
}

/**
 * ctk_source_view_set_indent_width:
 * @view: a #CtkSourceView.
 * @width: indent width in characters.
 *
 * Sets the number of spaces to use for each step of indent when the tab key is
 * pressed. If @width is -1, the value of the #CtkSourceView:tab-width property
 * will be used.
 *
 * The #CtkSourceView:indent-width interacts with the
 * #CtkSourceView:insert-spaces-instead-of-tabs property and
 * #CtkSourceView:tab-width. An example will be clearer: if the
 * #CtkSourceView:indent-width is 4 and
 * #CtkSourceView:tab-width is 8 and
 * #CtkSourceView:insert-spaces-instead-of-tabs is %FALSE, then pressing the tab
 * key at the beginning of a line will insert 4 spaces. So far so good. Pressing
 * the tab key a second time will remove the 4 spaces and insert a \t character
 * instead (since #CtkSourceView:tab-width is 8). On the other hand, if
 * #CtkSourceView:insert-spaces-instead-of-tabs is %TRUE, the second tab key
 * pressed will insert 4 more spaces for a total of 8 spaces in the
 * #CtkTextBuffer.
 *
 * The test-widget program (available in the CtkSourceView repository) may be
 * useful to better understand the indentation settings (enable the space
 * drawing!).
 */
void
ctk_source_view_set_indent_width (CtkSourceView *view,
				  gint           width)
{
	g_return_if_fail (CTK_SOURCE_VIEW (view));
	g_return_if_fail (width == -1 || (0 < width && width <= MAX_INDENT_WIDTH));

	if (view->priv->indent_width != width)
	{
		view->priv->indent_width = width;
		g_object_notify (G_OBJECT (view), "indent-width");
	}
}

/**
 * ctk_source_view_get_indent_width:
 * @view: a #CtkSourceView.
 *
 * Returns the number of spaces to use for each step of indent.
 * See ctk_source_view_set_indent_width() for details.
 *
 * Return value: indent width.
 */
gint
ctk_source_view_get_indent_width (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), 0);

	return view->priv->indent_width;
}

static gchar *
compute_indentation (CtkSourceView *view,
		     CtkTextIter   *cur)
{
	CtkTextIter start;
	CtkTextIter end;
	gunichar ch;

	start = *cur;
	ctk_text_iter_set_line_offset (&start, 0);

	end = start;

	ch = ctk_text_iter_get_char (&end);

	while (g_unichar_isspace (ch) &&
	       (ch != '\n') &&
	       (ch != '\r') &&
	       (ctk_text_iter_compare (&end, cur) < 0))
	{
		if (!ctk_text_iter_forward_char (&end))
		{
			break;
		}

		ch = ctk_text_iter_get_char (&end);
	}

	if (ctk_text_iter_equal (&start, &end))
	{
		return NULL;
	}

	return ctk_text_iter_get_slice (&start, &end);
}

static guint
get_real_indent_width (CtkSourceView *view)
{
	return view->priv->indent_width < 0 ?
	       view->priv->tab_width :
	       (guint) view->priv->indent_width;
}

static gchar *
get_indent_string (guint tabs,
		   guint spaces)
{
	gchar *str;

	str = g_malloc (tabs + spaces + 1);

	if (tabs > 0)
	{
		memset (str, '\t', tabs);
	}

	if (spaces > 0)
	{
		memset (str + tabs, ' ', spaces);
	}

	str[tabs + spaces] = '\0';

	return str;
}

/**
 * ctk_source_view_indent_lines:
 * @view: a #CtkSourceView.
 * @start: #CtkTextIter of the first line to indent
 * @end: #CtkTextIter of the last line to indent
 *
 * Inserts one indentation level at the beginning of the specified lines. The
 * empty lines are not indented.
 *
 * Since: 3.16
 */
void
ctk_source_view_indent_lines (CtkSourceView *view,
			      CtkTextIter   *start,
			      CtkTextIter   *end)
{
	CtkTextBuffer *buf;
	gboolean bracket_hl;
	CtkTextMark *start_mark, *end_mark;
	gint start_line, end_line;
	gchar *tab_buffer = NULL;
	guint tabs = 0;
	guint spaces = 0;
	gint i;

	if (view->priv->completion != NULL)
	{
		ctk_source_completion_block_interactive (view->priv->completion);
	}

	buf = ctk_text_view_get_buffer (CTK_TEXT_VIEW (view));

	bracket_hl = ctk_source_buffer_get_highlight_matching_brackets (CTK_SOURCE_BUFFER (buf));
	ctk_source_buffer_set_highlight_matching_brackets (CTK_SOURCE_BUFFER (buf), FALSE);

	start_mark = ctk_text_buffer_create_mark (buf, NULL, start, FALSE);
	end_mark = ctk_text_buffer_create_mark (buf, NULL, end, FALSE);

	start_line = ctk_text_iter_get_line (start);
	end_line = ctk_text_iter_get_line (end);

	if ((ctk_text_iter_get_visible_line_offset (end) == 0) &&
	    (end_line > start_line))
	{
		end_line--;
	}

	if (view->priv->insert_spaces)
	{
		spaces = get_real_indent_width (view);

		tab_buffer = g_strnfill (spaces, ' ');
	}
	else if (view->priv->indent_width > 0 &&
	         view->priv->indent_width != (gint)view->priv->tab_width)
	{
		guint indent_width;

		indent_width = get_real_indent_width (view);
		spaces = indent_width % view->priv->tab_width;
		tabs = indent_width / view->priv->tab_width;

		tab_buffer = get_indent_string (tabs, spaces);
	}
	else
	{
		tabs = 1;
		tab_buffer = g_strdup ("\t");
	}

	ctk_text_buffer_begin_user_action (buf);

	for (i = start_line; i <= end_line; i++)
	{
		CtkTextIter iter;
		CtkTextIter iter2;
		guint replaced_spaces = 0;

		ctk_text_buffer_get_iter_at_line (buf, &iter, i);

		/* Don't add indentation on completely empty lines, to not add
		 * trailing spaces.
		 * Note that non-empty lines containing only whitespaces are
		 * indented like any other non-empty line, because those lines
		 * already contain trailing spaces, some users use those
		 * whitespaces to more easily insert text at the right place
		 * without the need to insert the indentation each time.
		 */
		if (ctk_text_iter_ends_line (&iter))
		{
			continue;
		}

		/* add spaces always after tabs, to avoid the case
		 * where "\t" becomes "  \t" with no visual difference */
		while (ctk_text_iter_get_char (&iter) == '\t')
		{
			ctk_text_iter_forward_char (&iter);
		}

		/* if tabs are allowed try to merge the spaces
		 * with the tab we are going to insert (if any) */
		iter2 = iter;
		while (!view->priv->insert_spaces &&
		       (ctk_text_iter_get_char (&iter2) == ' ') &&
			replaced_spaces < view->priv->tab_width)
		{
			++replaced_spaces;

			ctk_text_iter_forward_char (&iter2);
		}

		if (replaced_spaces > 0)
		{
			gchar *indent_buf;
			guint t, s;

			t = tabs + (spaces + replaced_spaces) / view->priv->tab_width;
			s = (spaces + replaced_spaces) % view->priv->tab_width;
			indent_buf = get_indent_string (t, s);

			ctk_text_buffer_delete (buf, &iter, &iter2);
			ctk_text_buffer_insert (buf, &iter, indent_buf, -1);

			g_free (indent_buf);
		}
		else
		{
			ctk_text_buffer_insert (buf, &iter, tab_buffer, -1);
		}
	}

	ctk_text_buffer_end_user_action (buf);

	g_free (tab_buffer);

	ctk_source_buffer_set_highlight_matching_brackets (CTK_SOURCE_BUFFER (buf), bracket_hl);

	if (view->priv->completion != NULL)
	{
		ctk_source_completion_unblock_interactive (view->priv->completion);
	}

	ctk_text_view_scroll_mark_onscreen (CTK_TEXT_VIEW (view),
					    ctk_text_buffer_get_insert (buf));

	/* revalidate iters */
	ctk_text_buffer_get_iter_at_mark (buf, start, start_mark);
	ctk_text_buffer_get_iter_at_mark (buf, end, end_mark);

	ctk_text_buffer_delete_mark (buf, start_mark);
	ctk_text_buffer_delete_mark (buf, end_mark);
}

/**
 * ctk_source_view_unindent_lines:
 * @view: a #CtkSourceView.
 * @start: #CtkTextIter of the first line to indent
 * @end: #CtkTextIter of the last line to indent
 *
 * Removes one indentation level at the beginning of the
 * specified lines.
 *
 * Since: 3.16
 */
void
ctk_source_view_unindent_lines (CtkSourceView *view,
				CtkTextIter   *start,
				CtkTextIter   *end)
{
	CtkTextBuffer *buf;
	gboolean bracket_hl;
	CtkTextMark *start_mark, *end_mark;
	gint start_line, end_line;
	gint tab_width;
	gint indent_width;
	gint i;

	if (view->priv->completion != NULL)
	{
		ctk_source_completion_block_interactive (view->priv->completion);
	}

	buf = ctk_text_view_get_buffer (CTK_TEXT_VIEW (view));

	bracket_hl = ctk_source_buffer_get_highlight_matching_brackets (CTK_SOURCE_BUFFER (buf));
	ctk_source_buffer_set_highlight_matching_brackets (CTK_SOURCE_BUFFER (buf), FALSE);

	start_mark = ctk_text_buffer_create_mark (buf, NULL, start, FALSE);
	end_mark = ctk_text_buffer_create_mark (buf, NULL, end, FALSE);

	start_line = ctk_text_iter_get_line (start);
	end_line = ctk_text_iter_get_line (end);

	if ((ctk_text_iter_get_visible_line_offset (end) == 0) &&
	    (end_line > start_line))
	{
		end_line--;
	}

	tab_width = view->priv->tab_width;
	indent_width = get_real_indent_width (view);

	ctk_text_buffer_begin_user_action (buf);

	for (i = start_line; i <= end_line; i++)
	{
		CtkTextIter iter, iter2;
		gint to_delete = 0;
		gint to_delete_equiv = 0;

		ctk_text_buffer_get_iter_at_line (buf, &iter, i);

		iter2 = iter;

		while (!ctk_text_iter_ends_line (&iter2) &&
		       to_delete_equiv < indent_width)
		{
			gunichar c;

			c = ctk_text_iter_get_char (&iter2);
			if (c == '\t')
			{
				to_delete_equiv += tab_width - to_delete_equiv % tab_width;
				++to_delete;
			}
			else if (c == ' ')
			{
				++to_delete_equiv;
				++to_delete;
			}
			else
			{
				break;
			}

			ctk_text_iter_forward_char (&iter2);
		}

		if (to_delete > 0)
		{
			ctk_text_iter_set_line_offset (&iter2, to_delete);
			ctk_text_buffer_delete (buf, &iter, &iter2);
		}
	}

	ctk_text_buffer_end_user_action (buf);

	ctk_source_buffer_set_highlight_matching_brackets (CTK_SOURCE_BUFFER (buf), bracket_hl);

	if (view->priv->completion != NULL)
	{
		ctk_source_completion_unblock_interactive (view->priv->completion);
	}

	ctk_text_view_scroll_mark_onscreen (CTK_TEXT_VIEW (view),
					    ctk_text_buffer_get_insert (buf));

	/* revalidate iters */
	ctk_text_buffer_get_iter_at_mark (buf, start, start_mark);
	ctk_text_buffer_get_iter_at_mark (buf, end, end_mark);

	ctk_text_buffer_delete_mark (buf, start_mark);
	ctk_text_buffer_delete_mark (buf, end_mark);
}

static gint
get_line_offset_in_equivalent_spaces (CtkSourceView *view,
				      CtkTextIter   *iter)
{
	CtkTextIter i;
	gint tab_width;
	gint n = 0;

	tab_width = view->priv->tab_width;

	i = *iter;
	ctk_text_iter_set_line_offset (&i, 0);

	while (!ctk_text_iter_equal (&i, iter))
	{
		gunichar c;

		c = ctk_text_iter_get_char (&i);
		if (c == '\t')
		{
			n += tab_width - n % tab_width;
		}
		else
		{
			++n;
		}

		ctk_text_iter_forward_char (&i);
	}

	return n;
}

static void
insert_tab_or_spaces (CtkSourceView *view,
		      CtkTextIter   *start,
		      CtkTextIter   *end)
{
	CtkTextBuffer *buf;
	gchar *tab_buf;
	gint cursor_offset = 0;

	if (view->priv->insert_spaces)
	{
		gint indent_width;
		gint pos;
		gint spaces;

		indent_width = get_real_indent_width (view);

		/* CHECK: is this a performance problem? */
		pos = get_line_offset_in_equivalent_spaces (view, start);
		spaces = indent_width - pos % indent_width;

		tab_buf = g_strnfill (spaces, ' ');
	}
	else if (view->priv->indent_width > 0 &&
	         view->priv->indent_width != (gint)view->priv->tab_width)
	{
		CtkTextIter iter;
		gint i;
		gint tab_width;
		gint indent_width;
		gint from;
		gint to;
		gint preceding_spaces = 0;
		gint following_tabs = 0;
		gint equiv_spaces;
		gint tabs;
		gint spaces;

		tab_width = view->priv->tab_width;
		indent_width = get_real_indent_width (view);

		/* CHECK: is this a performance problem? */
		from = get_line_offset_in_equivalent_spaces (view, start);
		to = indent_width * (1 + from / indent_width);
		equiv_spaces = to - from;

		/* extend the selection to include
		 * preceding spaces so that if indentation
		 * width < tab width, two conseutive indentation
		 * width units get compressed into a tab */
		iter = *start;
		for (i = 0; i < tab_width; ++i)
		{
			ctk_text_iter_backward_char (&iter);

			if (ctk_text_iter_get_char (&iter) == ' ')
			{
				++preceding_spaces;
			}
			else
			{
				break;
			}
		}

		ctk_text_iter_backward_chars (start, preceding_spaces);

		/* now also extend the selection to the following tabs
		 * since we do not want to insert spaces before a tab
		 * since it may have no visual effect */
		while (ctk_text_iter_get_char (end) == '\t')
		{
			++following_tabs;
			ctk_text_iter_forward_char (end);
		}

		tabs = (preceding_spaces + equiv_spaces) / tab_width;
		spaces = (preceding_spaces + equiv_spaces) % tab_width;

		tab_buf = get_indent_string (tabs + following_tabs, spaces);

		cursor_offset = ctk_text_iter_get_offset (start) +
			        tabs +
		                (following_tabs > 0 ? 1 : spaces);
	}
	else
	{
		tab_buf = g_strdup ("\t");
	}

	buf = ctk_text_view_get_buffer (CTK_TEXT_VIEW (view));

	ctk_text_buffer_begin_user_action (buf);

	ctk_text_buffer_delete (buf, start, end);
	ctk_text_buffer_insert (buf, start, tab_buf, -1);

	/* adjust cursor position if needed */
	if (cursor_offset > 0)
	{
		CtkTextIter iter;

		ctk_text_buffer_get_iter_at_offset (buf, &iter, cursor_offset);
		ctk_text_buffer_place_cursor (buf, &iter);
	}

	ctk_text_buffer_end_user_action (buf);

	g_free (tab_buf);
}

static void
ctk_source_view_move_words (CtkSourceView *view,
			    gint           step)
{
	CtkTextBuffer *buf;
	CtkTextIter s, e, ns, ne;
	CtkTextMark *nsmark, *nemark;
	gchar *old_text, *new_text;

	buf = ctk_text_view_get_buffer (CTK_TEXT_VIEW (view));

	if (step == 0 || ctk_text_view_get_editable (CTK_TEXT_VIEW (view)) == FALSE)
	{
		return;
	}

	ctk_text_buffer_get_selection_bounds (buf, &s, &e);

	if (ctk_text_iter_compare (&s, &e) == 0)
	{
		if (!ctk_text_iter_starts_word (&s))
		{
			if (!ctk_text_iter_inside_word (&s) && !ctk_text_iter_ends_word (&s))
			{
				return;
			}
			else
			{
				ctk_text_iter_backward_word_start (&s);
			}
		}

		if (!ctk_text_iter_starts_word (&s))
		{
			return;
		}

		e = s;

		if (!ctk_text_iter_ends_word (&e))
		{
			if (!ctk_text_iter_forward_word_end (&e))
			{
				ctk_text_iter_forward_to_end (&e);
			}

			if (!ctk_text_iter_ends_word (&e))
			{
				return;
			}
		}
	}

	/* Swap the selection with the next or previous word, based on step */
	if (step > 0)
	{
		ne = e;

		if (!ctk_text_iter_forward_word_ends (&ne, step))
		{
			ctk_text_iter_forward_to_end (&ne);
		}

		if (!ctk_text_iter_ends_word (&ne) || ctk_text_iter_equal (&ne, &e))
		{
			return;
		}

		ns = ne;

		if (!ctk_text_iter_backward_word_start (&ns))
		{
			return;
		}
	}
	else
	{
		ns = s;

		if (!ctk_text_iter_backward_word_starts (&ns, -step))
		{
			return;
		}

		ne = ns;

		if (!ctk_text_iter_forward_word_end (&ne))
		{
			return;
		}
	}

	if (ctk_text_iter_in_range (&ns, &s, &e) ||
	    (!ctk_text_iter_equal (&s, &ne) &&
	     ctk_text_iter_in_range (&ne, &s, &e)))
	{
		return;
	}

	old_text = ctk_text_buffer_get_text (buf, &s, &e, TRUE);
	new_text = ctk_text_buffer_get_text (buf, &ns, &ne, TRUE);

	ctk_text_buffer_begin_user_action (buf);

	nsmark = ctk_text_buffer_create_mark (buf, NULL, &ns, step < 0);
	nemark = ctk_text_buffer_create_mark (buf, NULL, &ne, step < 0);

	ctk_text_buffer_delete (buf, &s, &e);
	ctk_text_buffer_insert (buf, &s, new_text, -1);

	ctk_text_buffer_get_iter_at_mark (buf, &ns, nsmark);
	ctk_text_buffer_get_iter_at_mark (buf, &ne, nemark);

	ctk_text_buffer_delete (buf, &ns, &ne);
	ctk_text_buffer_insert (buf, &ns, old_text, -1);

	ne = ns;
	ctk_text_buffer_get_iter_at_mark (buf, &ns, nsmark);

	ctk_text_buffer_select_range (buf, &ns, &ne);

	ctk_text_buffer_delete_mark (buf, nsmark);
	ctk_text_buffer_delete_mark (buf, nemark);

	ctk_text_buffer_end_user_action (buf);

	ctk_text_view_scroll_mark_onscreen (CTK_TEXT_VIEW (view),
	                                    ctk_text_buffer_get_insert (buf));

	g_free (old_text);
	g_free (new_text);
}

static gboolean
buffer_contains_trailing_newline (CtkTextBuffer *buffer)
{
	CtkTextIter iter;
	gunichar ch;

	ctk_text_buffer_get_end_iter (buffer, &iter);
	ctk_text_iter_backward_char (&iter);
	ch = ctk_text_iter_get_char (&iter);

	return (ch == '\n' || ch == '\r');
}

/* FIXME could be a function of CtkSourceBuffer, it's also useful for the
 * FileLoader.
 */
static void
remove_trailing_newline (CtkTextBuffer *buffer)
{
	CtkTextIter start;
	CtkTextIter end;

	ctk_text_buffer_get_end_iter (buffer, &end);
	start = end;

	ctk_text_iter_set_line_offset (&start, 0);

	if (ctk_text_iter_ends_line (&start) &&
	    ctk_text_iter_backward_line (&start))
	{
		if (!ctk_text_iter_ends_line (&start))
		{
			ctk_text_iter_forward_to_line_end (&start);
		}

		ctk_text_buffer_delete (buffer, &start, &end);
	}
}

static void
ctk_source_view_move_lines (CtkSourceView *view,
			    gboolean       down)
{
	CtkTextBuffer *buffer;
	CtkTextIter start;
	CtkTextIter end;
	CtkTextIter insert_pos;
	CtkTextMark *start_mark;
	CtkTextMark *end_mark;
	gchar *text;
	gboolean initially_contains_trailing_newline;

	buffer = ctk_text_view_get_buffer (CTK_TEXT_VIEW (view));

	if (!ctk_text_view_get_editable (CTK_TEXT_VIEW (view)))
	{
		return;
	}

	ctk_text_buffer_get_selection_bounds (buffer, &start, &end);

	/* Get the entire lines, including the paragraph terminator. */
	ctk_text_iter_set_line_offset (&start, 0);
	if (!ctk_text_iter_starts_line (&end) ||
	    ctk_text_iter_get_line (&start) == ctk_text_iter_get_line (&end))
	{
		ctk_text_iter_forward_line (&end);
	}

	if ((!down && ctk_text_iter_is_start (&start)) ||
	    (down && ctk_text_iter_is_end (&end)))
	{
		/* Nothing to do, and the undo/redo history must remain
		 * unchanged.
		 */
		return;
	}

	start_mark = ctk_text_buffer_create_mark (buffer, NULL, &start, TRUE);
	end_mark = ctk_text_buffer_create_mark (buffer, NULL, &end, FALSE);

	ctk_text_buffer_begin_user_action (buffer);

	initially_contains_trailing_newline = buffer_contains_trailing_newline (buffer);

	if (!initially_contains_trailing_newline)
	{
		/* Insert a trailing newline. */
		ctk_text_buffer_get_end_iter (buffer, &end);
		ctk_text_buffer_insert (buffer, &end, "\n", -1);
	}

	/* At this point all lines finish with a newline or carriage return, so
	 * there are no special cases for the last line.
	 */

	ctk_text_buffer_get_iter_at_mark (buffer, &start, start_mark);
	ctk_text_buffer_get_iter_at_mark (buffer, &end, end_mark);
	ctk_text_buffer_delete_mark (buffer, start_mark);
	ctk_text_buffer_delete_mark (buffer, end_mark);
	start_mark = NULL;
	end_mark = NULL;

	text = ctk_text_buffer_get_text (buffer, &start, &end, TRUE);

	ctk_text_buffer_delete (buffer, &start, &end);

	if (down)
	{
		insert_pos = end;
		ctk_text_iter_forward_line (&insert_pos);
	}
	else
	{
		insert_pos = start;
		ctk_text_iter_backward_line (&insert_pos);
	}

	start_mark = ctk_text_buffer_create_mark (buffer, NULL, &insert_pos, TRUE);

	ctk_text_buffer_insert (buffer, &insert_pos, text, -1);
	g_free (text);

	/* Select the moved text. */
	ctk_text_buffer_get_iter_at_mark (buffer, &start, start_mark);
	ctk_text_buffer_delete_mark (buffer, start_mark);

	ctk_text_buffer_select_range (buffer, &start, &insert_pos);

	if (!initially_contains_trailing_newline)
	{
		remove_trailing_newline (buffer);
	}

	ctk_text_buffer_end_user_action (buffer);

	ctk_text_view_scroll_mark_onscreen (CTK_TEXT_VIEW (view),
					    ctk_text_buffer_get_insert (buffer));
}

static gboolean
do_smart_backspace (CtkSourceView *view)
{
	CtkTextBuffer *buffer;
	gboolean default_editable;
	CtkTextIter insert;
	CtkTextIter end;
	CtkTextIter leading_end;
	guint visual_column;
	gint indent_width;

	buffer = CTK_TEXT_BUFFER (view->priv->source_buffer);
	default_editable = ctk_text_view_get_editable (CTK_TEXT_VIEW (view));

	if (ctk_text_buffer_get_selection_bounds (buffer, &insert, &end))
	{
		return FALSE;
	}

	/* If the line isn't empty up to our cursor, ignore. */
	_ctk_source_iter_get_leading_spaces_end_boundary (&insert, &leading_end);
	if (ctk_text_iter_compare (&leading_end, &insert) < 0)
	{
		return FALSE;
	}

	visual_column = ctk_source_view_get_visual_column (view, &insert);
	indent_width = view->priv->indent_width;
	if (indent_width <= 0)
	{
		indent_width = view->priv->tab_width;
	}

	g_return_val_if_fail (indent_width > 0, FALSE);

	/* If the cursor is not at an indent_width boundary, it probably means
	 * that we want to adjust the spaces.
	 */
	if ((gint)visual_column < indent_width)
	{
		return FALSE;
	}

	if ((visual_column % indent_width) == 0)
	{
		guint target_column;

		g_assert ((gint)visual_column >= indent_width);
		target_column = visual_column - indent_width;

		while (ctk_source_view_get_visual_column (view, &insert) > target_column)
		{
			ctk_text_iter_backward_cursor_position (&insert);
		}

		ctk_text_buffer_begin_user_action (buffer);
		ctk_text_buffer_delete_interactive (buffer, &insert, &end, default_editable);
		while (ctk_source_view_get_visual_column (view, &insert) < target_column)
		{
			if (!ctk_text_buffer_insert_interactive (buffer, &insert, " ", 1, default_editable))
			{
				break;
			}
		}
		ctk_text_buffer_end_user_action (buffer);

		return TRUE;
	}

	return FALSE;
}

static gboolean
do_ctrl_backspace (CtkSourceView *view)
{
	CtkTextBuffer *buffer;
	CtkTextIter insert;
	CtkTextIter end;
	CtkTextIter leading_end;
	gboolean default_editable;

	buffer = CTK_TEXT_BUFFER (view->priv->source_buffer);
	default_editable = ctk_text_view_get_editable (CTK_TEXT_VIEW (view));

	if (ctk_text_buffer_get_selection_bounds (buffer, &insert, &end))
	{
		return FALSE;
	}

	/* A <Control>BackSpace at the beginning of the line should only move us to the
	 * end of the previous line. Anything more than that is non-obvious because it requires
	 * looking in a position other than where the cursor is.
	 */
	if ((ctk_text_iter_get_line_offset (&insert) == 0) &&
	    (ctk_text_iter_get_line (&insert) > 0))
	{
		ctk_text_iter_backward_cursor_position (&insert);
		ctk_text_buffer_delete_interactive (buffer, &insert, &end, default_editable);
		return TRUE;
	}

	/* If only leading whitespaces are on the left of the cursor, delete up
	 * to the zero position.
	 */
	_ctk_source_iter_get_leading_spaces_end_boundary (&insert, &leading_end);
	if (ctk_text_iter_compare (&insert, &leading_end) <= 0)
	{
		ctk_text_iter_set_line_offset (&insert, 0);
		ctk_text_buffer_delete_interactive (buffer, &insert, &end, default_editable);
		return TRUE;
	}

	return FALSE;
}

static gboolean
ctk_source_view_key_press_event (CtkWidget   *widget,
				 CdkEventKey *event)
{
	CtkSourceView *view;
	CtkTextBuffer *buf;
	CtkTextIter cur;
	CtkTextMark *mark;
	guint modifiers;
	gint key;
	gboolean editable;

	view = CTK_SOURCE_VIEW (widget);
	buf = ctk_text_view_get_buffer (CTK_TEXT_VIEW (widget));

	editable = ctk_text_view_get_editable (CTK_TEXT_VIEW (widget));

	/* Be careful when testing for modifier state equality:
	 * caps lock, num lock,etc need to be taken into account */
	modifiers = ctk_accelerator_get_default_mod_mask ();

	key = event->keyval;

	mark = ctk_text_buffer_get_insert (buf);
	ctk_text_buffer_get_iter_at_mark (buf, &cur, mark);

	if ((key == CDK_KEY_Return || key == CDK_KEY_KP_Enter) &&
	    !(event->state & CDK_SHIFT_MASK) &&
	    view->priv->auto_indent)
	{
		/* Auto-indent means that when you press ENTER at the end of a
		 * line, the new line is automatically indented at the same
		 * level as the previous line.
		 * SHIFT+ENTER allows to avoid autoindentation.
		 */
		gchar *indent = NULL;

		/* Calculate line indentation and create indent string. */
		indent = compute_indentation (view, &cur);

		if (indent != NULL)
		{
			/* Allow input methods to internally handle a key press event.
			 * If this function returns TRUE, then no further processing should be done
			 * for this keystroke. */
			if (ctk_text_view_im_context_filter_keypress (CTK_TEXT_VIEW (view), event))
			{
				g_free (indent);
				return CDK_EVENT_STOP;
			}

			/* Delete any selected text to preserve behavior without auto-indent */
			ctk_text_buffer_delete_selection (buf,
			                                  TRUE,
			                                  ctk_text_view_get_editable (CTK_TEXT_VIEW (view)));

			/* If an input method or deletion has inserted some text while handling the
			 * key press event, the cur iterm may be invalid, so get the iter again
			 */
			ctk_text_buffer_get_iter_at_mark (buf, &cur, mark);

			/* Insert new line and auto-indent. */
			ctk_text_buffer_begin_user_action (buf);
			ctk_text_buffer_insert (buf, &cur, "\n", 1);
			ctk_text_buffer_insert (buf, &cur, indent, strlen (indent));
			g_free (indent);
			ctk_text_buffer_end_user_action (buf);
			ctk_text_view_scroll_mark_onscreen (CTK_TEXT_VIEW (widget),
							    mark);
			return CDK_EVENT_STOP;
		}
	}

	/* if tab or shift+tab:
	 * with shift+tab key is CDK_ISO_Left_Tab (yay! on win32 and mac too!)
	 */
	if ((key == CDK_KEY_Tab || key == CDK_KEY_KP_Tab || key == CDK_KEY_ISO_Left_Tab) &&
	    ((event->state & modifiers) == 0 ||
	     (event->state & modifiers) == CDK_SHIFT_MASK) &&
	    editable &&
	    ctk_text_view_get_accepts_tab (CTK_TEXT_VIEW (view)))
	{
		CtkTextIter s, e;
		gboolean has_selection;

		has_selection = ctk_text_buffer_get_selection_bounds (buf, &s, &e);

		if (view->priv->indent_on_tab)
		{
			/* shift+tab: always unindent */
			if (event->state & CDK_SHIFT_MASK)
			{
				_ctk_source_buffer_save_and_clear_selection (CTK_SOURCE_BUFFER (buf));
				ctk_source_view_unindent_lines (view, &s, &e);
				_ctk_source_buffer_restore_selection (CTK_SOURCE_BUFFER (buf));
				return CDK_EVENT_STOP;
			}

			/* tab: if we have a selection which spans one whole line
			 * or more, we mass indent, if the selection spans less then
			 * the full line just replace the text with \t
			 */
			if (has_selection &&
			    ((ctk_text_iter_starts_line (&s) && ctk_text_iter_ends_line (&e)) ||
			     (ctk_text_iter_get_line (&s) != ctk_text_iter_get_line (&e))))
			{
				_ctk_source_buffer_save_and_clear_selection (CTK_SOURCE_BUFFER (buf));
				ctk_source_view_indent_lines (view, &s, &e);
				_ctk_source_buffer_restore_selection (CTK_SOURCE_BUFFER (buf));
				return CDK_EVENT_STOP;
			}
		}

		insert_tab_or_spaces (view, &s, &e);
		return CDK_EVENT_STOP;
	}

	if (key == CDK_KEY_BackSpace)
	{
		if ((event->state & modifiers) == 0)
		{
			if (view->priv->smart_backspace && do_smart_backspace (view))
			{
				return CDK_EVENT_STOP;
			}
		}
		else if ((event->state & modifiers) == CDK_CONTROL_MASK)
		{
			if (do_ctrl_backspace (view))
			{
				return CDK_EVENT_STOP;
			}
		}
	}

	return CTK_WIDGET_CLASS (ctk_source_view_parent_class)->key_press_event (widget, event);
}

/**
 * ctk_source_view_get_auto_indent:
 * @view: a #CtkSourceView.
 *
 * Returns whether auto-indentation of text is enabled.
 *
 * Returns: %TRUE if auto indentation is enabled.
 */
gboolean
ctk_source_view_get_auto_indent (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), FALSE);

	return view->priv->auto_indent;
}

/**
 * ctk_source_view_set_auto_indent:
 * @view: a #CtkSourceView.
 * @enable: whether to enable auto indentation.
 *
 * If %TRUE auto-indentation of text is enabled.
 *
 * When Enter is pressed to create a new line, the auto-indentation inserts the
 * same indentation as the previous line. This is <emphasis>not</emphasis> a
 * "smart indentation" where an indentation level is added or removed depending
 * on the context.
 */
void
ctk_source_view_set_auto_indent (CtkSourceView *view,
				 gboolean       enable)
{
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	enable = enable != FALSE;

	if (view->priv->auto_indent != enable)
	{
		view->priv->auto_indent = enable;
		g_object_notify (G_OBJECT (view), "auto_indent");
	}
}

/**
 * ctk_source_view_get_insert_spaces_instead_of_tabs:
 * @view: a #CtkSourceView.
 *
 * Returns whether when inserting a tabulator character it should
 * be replaced by a group of space characters.
 *
 * Returns: %TRUE if spaces are inserted instead of tabs.
 */
gboolean
ctk_source_view_get_insert_spaces_instead_of_tabs (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), FALSE);

	return view->priv->insert_spaces;
}

/**
 * ctk_source_view_set_insert_spaces_instead_of_tabs:
 * @view: a #CtkSourceView.
 * @enable: whether to insert spaces instead of tabs.
 *
 * If %TRUE a tab key pressed is replaced by a group of space characters. Of
 * course it is still possible to insert a real \t programmatically with the
 * #CtkTextBuffer API.
 */
void
ctk_source_view_set_insert_spaces_instead_of_tabs (CtkSourceView *view,
						   gboolean       enable)
{
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	enable = enable != FALSE;

	if (view->priv->insert_spaces != enable)
	{
		view->priv->insert_spaces = enable;
		g_object_notify (G_OBJECT (view), "insert_spaces_instead_of_tabs");
	}
}

/**
 * ctk_source_view_get_indent_on_tab:
 * @view: a #CtkSourceView.
 *
 * Returns whether when the tab key is pressed the current selection
 * should get indented instead of replaced with the \t character.
 *
 * Return value: %TRUE if the selection is indented when tab is pressed.
 */
gboolean
ctk_source_view_get_indent_on_tab (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), FALSE);

	return view->priv->indent_on_tab;
}

/**
 * ctk_source_view_set_indent_on_tab:
 * @view: a #CtkSourceView.
 * @enable: whether to indent a block when tab is pressed.
 *
 * If %TRUE, when the tab key is pressed when several lines are selected, the
 * selected lines are indented of one level instead of being replaced with a \t
 * character. Shift+Tab unindents the selection.
 *
 * If the first or last line is not selected completely, it is also indented or
 * unindented.
 *
 * When the selection doesn't span several lines, the tab key always replaces
 * the selection with a normal \t character.
 */
void
ctk_source_view_set_indent_on_tab (CtkSourceView *view,
				   gboolean       enable)
{
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	enable = enable != FALSE;

	if (view->priv->indent_on_tab != enable)
	{
		view->priv->indent_on_tab = enable;
		g_object_notify (G_OBJECT (view), "indent_on_tab");
	}
}

static void
view_dnd_drop (CtkTextView      *view,
	       CdkDragContext   *context,
	       gint              x,
	       gint              y,
	       CtkSelectionData *selection_data,
	       guint             info,
	       guint             timestamp,
	       gpointer          data)
{

	CtkTextIter iter;

	if (info == TARGET_COLOR)
	{
		CdkRGBA rgba;
		gchar string[] = "#000000";
		gint buffer_x;
		gint buffer_y;
		gint length = ctk_selection_data_get_length (selection_data);
		guint format;

		if (length < 0)
		{
			return;
		}

		format = ctk_selection_data_get_format (selection_data);

		if (format == 8 && length == 4)
		{
			guint8 *vals;

			vals = (gpointer) ctk_selection_data_get_data (selection_data);

			rgba.red = vals[0] / 256.0;
			rgba.green = vals[1] / 256.0;
			rgba.blue = vals[2] / 256.0;
			rgba.alpha = 1.0;
		}
		else if (format == 16 && length == 8)
		{
			guint16 *vals;

			vals = (gpointer) ctk_selection_data_get_data (selection_data);

			rgba.red = vals[0] / 65535.0;
			rgba.green = vals[1] / 65535.0;
			rgba.blue = vals[2] / 65535.0;
			rgba.alpha = 1.0;
		}
		else
		{
			g_warning ("Received invalid color data\n");
			return;
		}

		g_snprintf (string, sizeof string, "#%02X%02X%02X",
		            (gint)(rgba.red * 256),
		            (gint)(rgba.green * 256),
		            (gint)(rgba.blue * 256));

		ctk_text_view_window_to_buffer_coords (view,
						       CTK_TEXT_WINDOW_TEXT,
						       x,
						       y,
						       &buffer_x,
						       &buffer_y);
		ctk_text_view_get_iter_at_location (view, &iter, buffer_x, buffer_y);

		if (ctk_text_view_get_editable (view))
		{
			ctk_text_buffer_insert (ctk_text_view_get_buffer (view),
						&iter,
						string,
						strlen (string));
			ctk_text_buffer_place_cursor (ctk_text_view_get_buffer (view),
						&iter);
		}

		/*
		 * FIXME: Check if the iter is inside a selection
		 * If it is, remove the selection and then insert at
		 * the cursor position - Paolo
		 */

		return;
	}
}

/**
 * ctk_source_view_get_highlight_current_line:
 * @view: a #CtkSourceView.
 *
 * Returns whether the current line is highlighted.
 *
 * Return value: %TRUE if the current line is highlighted.
 */
gboolean
ctk_source_view_get_highlight_current_line (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), FALSE);

	return view->priv->highlight_current_line;
}

/**
 * ctk_source_view_set_highlight_current_line:
 * @view: a #CtkSourceView.
 * @highlight: whether to highlight the current line.
 *
 * If @highlight is %TRUE the current line will be highlighted.
 */
void
ctk_source_view_set_highlight_current_line (CtkSourceView *view,
					    gboolean       highlight)
{
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	highlight = highlight != FALSE;

	if (view->priv->highlight_current_line != highlight)
	{
		view->priv->highlight_current_line = highlight;

		ctk_widget_queue_draw (CTK_WIDGET (view));

		g_object_notify (G_OBJECT (view), "highlight_current_line");
	}
}

/**
 * ctk_source_view_get_show_right_margin:
 * @view: a #CtkSourceView.
 *
 * Returns whether a right margin is displayed.
 *
 * Return value: %TRUE if the right margin is shown.
 */
gboolean
ctk_source_view_get_show_right_margin (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), FALSE);

	return view->priv->show_right_margin;
}

/**
 * ctk_source_view_set_show_right_margin:
 * @view: a #CtkSourceView.
 * @show: whether to show a right margin.
 *
 * If %TRUE a right margin is displayed.
 */
void
ctk_source_view_set_show_right_margin (CtkSourceView *view,
				       gboolean       show)
{
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	show = show != FALSE;

	if (view->priv->show_right_margin != show)
	{
		view->priv->show_right_margin = show;

		ctk_widget_queue_draw (CTK_WIDGET (view));

		g_object_notify (G_OBJECT (view), "show-right-margin");
	}
}

/**
 * ctk_source_view_get_right_margin_position:
 * @view: a #CtkSourceView.
 *
 * Gets the position of the right margin in the given @view.
 *
 * Return value: the position of the right margin.
 */
guint
ctk_source_view_get_right_margin_position (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), DEFAULT_RIGHT_MARGIN_POSITION);

	return view->priv->right_margin_pos;
}

/**
 * ctk_source_view_set_right_margin_position:
 * @view: a #CtkSourceView.
 * @pos: the width in characters where to position the right margin.
 *
 * Sets the position of the right margin in the given @view.
 */
void
ctk_source_view_set_right_margin_position (CtkSourceView *view,
					   guint          pos)
{
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));
	g_return_if_fail (1 <= pos && pos <= MAX_RIGHT_MARGIN_POSITION);

	if (view->priv->right_margin_pos != pos)
	{
		view->priv->right_margin_pos = pos;
		view->priv->cached_right_margin_pos = -1;

		ctk_widget_queue_draw (CTK_WIDGET (view));

		g_object_notify (G_OBJECT (view), "right-margin-position");
	}
}

/**
 * ctk_source_view_set_smart_backspace:
 * @view: a #CtkSourceView.
 * @smart_backspace: whether to enable smart Backspace handling.
 *
 * When set to %TRUE, pressing the Backspace key will try to delete spaces
 * up to the previous tab stop.
 *
 * Since: 3.18
 */
void
ctk_source_view_set_smart_backspace (CtkSourceView *view,
                                     gboolean       smart_backspace)
{
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	smart_backspace = smart_backspace != FALSE;

	if (smart_backspace != view->priv->smart_backspace)
	{
		view->priv->smart_backspace = smart_backspace;
		g_object_notify (G_OBJECT (view), "smart-backspace");
	}
}

/**
 * ctk_source_view_get_smart_backspace:
 * @view: a #CtkSourceView.
 *
 * Returns %TRUE if pressing the Backspace key will try to delete spaces
 * up to the previous tab stop.
 *
 * Returns: %TRUE if smart Backspace handling is enabled.
 *
 * Since: 3.18
 */
gboolean
ctk_source_view_get_smart_backspace (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), FALSE);

	return view->priv->smart_backspace;
}

/**
 * ctk_source_view_set_smart_home_end:
 * @view: a #CtkSourceView.
 * @smart_home_end: the desired behavior among #CtkSourceSmartHomeEndType.
 *
 * Set the desired movement of the cursor when HOME and END keys
 * are pressed.
 */
void
ctk_source_view_set_smart_home_end (CtkSourceView             *view,
				    CtkSourceSmartHomeEndType  smart_home_end)
{
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	if (view->priv->smart_home_end != smart_home_end)
	{
		view->priv->smart_home_end = smart_home_end;
		g_object_notify (G_OBJECT (view), "smart_home_end");
	}
}

/**
 * ctk_source_view_get_smart_home_end:
 * @view: a #CtkSourceView.
 *
 * Returns a #CtkSourceSmartHomeEndType end value specifying
 * how the cursor will move when HOME and END keys are pressed.
 *
 * Returns: a #CtkSourceSmartHomeEndType value.
 */
CtkSourceSmartHomeEndType
ctk_source_view_get_smart_home_end (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), FALSE);

	return view->priv->smart_home_end;
}

/**
 * ctk_source_view_get_visual_column:
 * @view: a #CtkSourceView.
 * @iter: a position in @view.
 *
 * Determines the visual column at @iter taking into consideration the
 * #CtkSourceView:tab-width of @view.
 *
 * Returns: the visual column at @iter.
 */
guint
ctk_source_view_get_visual_column (CtkSourceView     *view,
				   const CtkTextIter *iter)
{
	CtkTextIter position;
	guint column;
	guint tab_width;

	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), 0);
	g_return_val_if_fail (iter != NULL, 0);

	column = 0;
	tab_width = view->priv->tab_width;

	position = *iter;
	ctk_text_iter_set_line_offset (&position, 0);

	while (!ctk_text_iter_equal (&position, iter))
	{
		if (ctk_text_iter_get_char (&position) == '\t')
		{
			column += (tab_width - (column % tab_width));
		}
		else
		{
			++column;
		}

		/* FIXME: this does not handle invisible text correctly, but
		 * ctk_text_iter_forward_visible_cursor_position is too slow */
		if (!ctk_text_iter_forward_char (&position))
		{
			break;
		}
	}

	return column;
}

static void
update_background_pattern_color (CtkSourceView *view)
{
	if (view->priv->style_scheme == NULL)
	{
		view->priv->background_pattern_color_set = FALSE;
		return;
	}

	view->priv->background_pattern_color_set =
		_ctk_source_style_scheme_get_background_pattern_color (view->priv->style_scheme,
								       &view->priv->background_pattern_color);
}

static void
update_current_line_color (CtkSourceView *view)
{
	if (view->priv->style_scheme == NULL)
	{
		view->priv->current_line_color_set = FALSE;
		return;
	}

	view->priv->current_line_color_set =
		_ctk_source_style_scheme_get_current_line_color (view->priv->style_scheme,
								 &view->priv->current_line_color);
}

static void
update_right_margin_colors (CtkSourceView *view)
{
	CtkWidget *widget = CTK_WIDGET (view);

	if (view->priv->right_margin_line_color != NULL)
	{
		cdk_rgba_free (view->priv->right_margin_line_color);
		view->priv->right_margin_line_color = NULL;
	}

	if (view->priv->right_margin_overlay_color != NULL)
	{
		cdk_rgba_free (view->priv->right_margin_overlay_color);
		view->priv->right_margin_overlay_color = NULL;
	}

	if (view->priv->style_scheme != NULL)
	{
		CtkSourceStyle *style;

		style = _ctk_source_style_scheme_get_right_margin_style (view->priv->style_scheme);

		if (style != NULL)
		{
			gchar *color_str = NULL;
			gboolean color_set;
			CdkRGBA color;

			g_object_get (style,
				      "foreground", &color_str,
				      "foreground-set", &color_set,
				      NULL);

			if (color_set &&
			    color_str != NULL &&
			    cdk_rgba_parse (&color, color_str))
			{
				view->priv->right_margin_line_color = cdk_rgba_copy (&color);
				view->priv->right_margin_line_color->alpha =
					RIGHT_MARGIN_LINE_ALPHA / 255.;
			}

			g_free (color_str);
			color_str = NULL;

			g_object_get (style,
				      "background", &color_str,
				      "background-set", &color_set,
				      NULL);

			if (color_set &&
			    color_str != NULL &&
			    cdk_rgba_parse (&color, color_str))
			{
				view->priv->right_margin_overlay_color = cdk_rgba_copy (&color);
				view->priv->right_margin_overlay_color->alpha =
					RIGHT_MARGIN_OVERLAY_ALPHA / 255.;
			}

			g_free (color_str);
		}
	}

	if (view->priv->right_margin_line_color == NULL)
	{
		CtkStyleContext *context;
		CdkRGBA color;

		context = ctk_widget_get_style_context (widget);
		ctk_style_context_save (context);
		ctk_style_context_set_state (context, CTK_STATE_FLAG_NORMAL);
		ctk_style_context_get_color (context,
					     ctk_style_context_get_state (context),
					     &color);
		ctk_style_context_restore (context);

		view->priv->right_margin_line_color = cdk_rgba_copy (&color);
		view->priv->right_margin_line_color->alpha =
			RIGHT_MARGIN_LINE_ALPHA / 255.;
	}
}

static void
update_style (CtkSourceView *view)
{
	update_background_pattern_color (view);
	update_current_line_color (view);
	update_right_margin_colors (view);

	if (view->priv->space_drawer != NULL)
	{
		_ctk_source_space_drawer_update_color (view->priv->space_drawer, view);
	}

	ctk_widget_queue_draw (CTK_WIDGET (view));
}

static void
ctk_source_view_update_style_scheme (CtkSourceView *view)
{
	CtkTextBuffer *buffer;
	CtkSourceStyleScheme *new_scheme = NULL;

	buffer = ctk_text_view_get_buffer (CTK_TEXT_VIEW (view));

	if (CTK_SOURCE_IS_BUFFER (buffer))
	{
		new_scheme = ctk_source_buffer_get_style_scheme (CTK_SOURCE_BUFFER (buffer));
	}

	if (view->priv->style_scheme == new_scheme)
	{
		return;
	}

	if (view->priv->style_scheme != NULL)
	{
		_ctk_source_style_scheme_unapply (view->priv->style_scheme, view);
	}

	g_set_object (&view->priv->style_scheme, new_scheme);

	if (view->priv->style_scheme != NULL)
	{
		_ctk_source_style_scheme_apply (view->priv->style_scheme, view);
	}

	update_style (view);
}

static void
ctk_source_view_style_updated (CtkWidget *widget)
{
	CtkSourceView *view = CTK_SOURCE_VIEW (widget);

	/* Call default handler first. */
	if (CTK_WIDGET_CLASS (ctk_source_view_parent_class)->style_updated != NULL)
	{
		CTK_WIDGET_CLASS (ctk_source_view_parent_class)->style_updated (widget);
	}

	/* Re-set tab stops, but only if we already modified them, i.e.
	 * do nothing with good old 8-space tabs.
	 */
	if (view->priv->tabs_set)
	{
		set_tab_stops_internal (view);
	}

	/* Make sure the margin position is recalculated on next redraw. */
	view->priv->cached_right_margin_pos = -1;

	update_style (view);
}

static MarkCategory *
mark_category_new (CtkSourceMarkAttributes *attributes,
		   gint                     priority)
{
	MarkCategory* category = g_slice_new (MarkCategory);

	category->attributes = g_object_ref (attributes);
	category->priority = priority;

	return category;
}

static void
mark_category_free (MarkCategory *category)
{
	if (category != NULL)
	{
		g_object_unref (category->attributes);
		g_slice_free (MarkCategory, category);
	}
}

/**
 * ctk_source_view_get_completion:
 * @view: a #CtkSourceView.
 *
 * Gets the #CtkSourceCompletion associated with @view. The returned object is
 * guaranteed to be the same for the lifetime of @view. Each #CtkSourceView
 * object has a different #CtkSourceCompletion.
 *
 * Returns: (transfer none): the #CtkSourceCompletion associated with @view.
 */
CtkSourceCompletion *
ctk_source_view_get_completion (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), NULL);

	if (view->priv->completion == NULL)
	{
		view->priv->completion = ctk_source_completion_new (view);
	}

	return view->priv->completion;
}

/**
 * ctk_source_view_get_gutter:
 * @view: a #CtkSourceView.
 * @window_type: the gutter window type.
 *
 * Returns the #CtkSourceGutter object associated with @window_type for @view.
 * Only CTK_TEXT_WINDOW_LEFT and CTK_TEXT_WINDOW_RIGHT are supported,
 * respectively corresponding to the left and right gutter. The line numbers
 * and mark category icons are rendered in the left gutter.
 *
 * Since: 2.8
 *
 * Returns: (transfer none): the #CtkSourceGutter.
 */
CtkSourceGutter *
ctk_source_view_get_gutter (CtkSourceView     *view,
			    CtkTextWindowType  window_type)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), NULL);
	g_return_val_if_fail (window_type == CTK_TEXT_WINDOW_LEFT ||
	                      window_type == CTK_TEXT_WINDOW_RIGHT, NULL);

	if (window_type == CTK_TEXT_WINDOW_LEFT)
	{
		if (view->priv->left_gutter == NULL)
		{
			view->priv->left_gutter = _ctk_source_gutter_new (view, window_type);
		}

		return view->priv->left_gutter;
	}
	else
	{
		if (view->priv->right_gutter == NULL)
		{
			view->priv->right_gutter = _ctk_source_gutter_new (view, window_type);
		}

		return view->priv->right_gutter;
	}
}

/**
 * ctk_source_view_set_mark_attributes:
 * @view: a #CtkSourceView.
 * @category: the category.
 * @attributes: mark attributes.
 * @priority: priority of the category.
 *
 * Sets attributes and priority for the @category.
 */
void
ctk_source_view_set_mark_attributes (CtkSourceView           *view,
				     const gchar             *category,
				     CtkSourceMarkAttributes *attributes,
				     gint                     priority)
{
	MarkCategory *mark_category;

	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));
	g_return_if_fail (category != NULL);
	g_return_if_fail (CTK_SOURCE_IS_MARK_ATTRIBUTES (attributes));
	g_return_if_fail (priority >= 0);

	mark_category = mark_category_new (attributes, priority);
	g_hash_table_replace (view->priv->mark_categories,
	                      g_strdup (category),
	                      mark_category);
}

/**
 * ctk_source_view_get_mark_attributes:
 * @view: a #CtkSourceView.
 * @category: the category.
 * @priority: place where priority of the category will be stored.
 *
 * Gets attributes and priority for the @category.
 *
 * Returns: (transfer none): #CtkSourceMarkAttributes for the @category.
 * The object belongs to @view, so it must not be unreffed.
 */
CtkSourceMarkAttributes *
ctk_source_view_get_mark_attributes (CtkSourceView *view,
				     const gchar   *category,
				     gint          *priority)
{
	MarkCategory *mark_category;

	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), NULL);
	g_return_val_if_fail (category != NULL, NULL);

	mark_category = g_hash_table_lookup (view->priv->mark_categories,
	                                     category);

	if (mark_category != NULL)
	{
		if (priority != NULL)
		{
			*priority = mark_category->priority;
		}

		return mark_category->attributes;
	}

	return NULL;
}

/**
 * ctk_source_view_set_background_pattern:
 * @view: a #CtkSourceView.
 * @background_pattern: the #CtkSourceBackgroundPatternType.
 *
 * Set if and how the background pattern should be displayed.
 *
 * Since: 3.16
 */
void
ctk_source_view_set_background_pattern (CtkSourceView                  *view,
					CtkSourceBackgroundPatternType  background_pattern)
{
	g_return_if_fail (CTK_SOURCE_IS_VIEW (view));

	if (view->priv->background_pattern != background_pattern)
	{
		view->priv->background_pattern = background_pattern;

		ctk_widget_queue_draw (CTK_WIDGET (view));

		g_object_notify (G_OBJECT (view), "background-pattern");
	}
}

/**
 * ctk_source_view_get_background_pattern:
 * @view: a #CtkSourceView
 *
 * Returns the #CtkSourceBackgroundPatternType specifying if and how
 * the background pattern should be displayed for this @view.
 *
 * Returns: the #CtkSourceBackgroundPatternType.
 * Since: 3.16
 */
CtkSourceBackgroundPatternType
ctk_source_view_get_background_pattern (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), CTK_SOURCE_BACKGROUND_PATTERN_TYPE_NONE);

	return view->priv->background_pattern;
}

/**
 * ctk_source_view_get_space_drawer:
 * @view: a #CtkSourceView.
 *
 * Gets the #CtkSourceSpaceDrawer associated with @view. The returned object is
 * guaranteed to be the same for the lifetime of @view. Each #CtkSourceView
 * object has a different #CtkSourceSpaceDrawer.
 *
 * Returns: (transfer none): the #CtkSourceSpaceDrawer associated with @view.
 * Since: 3.24
 */
CtkSourceSpaceDrawer *
ctk_source_view_get_space_drawer (CtkSourceView *view)
{
	g_return_val_if_fail (CTK_SOURCE_IS_VIEW (view), NULL);

	return view->priv->space_drawer;
}
