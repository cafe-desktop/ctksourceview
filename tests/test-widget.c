/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2001 - Mikael Hermansson <tyan@linux.se>
 * Copyright (C) 2003 - Gustavo Giráldez <gustavo.giraldez@gmx.net>
 * Copyright (C) 2014 - Sébastien Wilmet <swilmet@gnome.org>
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

#include <string.h>
#include <ctksourceview/ctksource.h>

#define TEST_TYPE_WIDGET             (test_widget_get_type ())
#define TEST_WIDGET(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEST_TYPE_WIDGET, TestWidget))
#define TEST_WIDGET_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), TEST_TYPE_WIDGET, TestWidgetClass))
#define TEST_IS_WIDGET(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEST_TYPE_WIDGET))
#define TEST_IS_WIDGET_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEST_TYPE_WIDGET))
#define TEST_WIDGET_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), TEST_TYPE_WIDGET, TestWidgetClass))

typedef struct _TestWidget         TestWidget;
typedef struct _TestWidgetClass    TestWidgetClass;
typedef struct _TestWidgetPrivate  TestWidgetPrivate;

struct _TestWidget
{
	CtkGrid parent;

	TestWidgetPrivate *priv;
};

struct _TestWidgetClass
{
	CtkGridClass parent_class;
};

struct _TestWidgetPrivate
{
	CtkSourceView *view;
	CtkSourceBuffer *buffer;
	CtkSourceFile *file;
	CtkSourceMap *map;
	CtkCheckButton *show_top_border_window_checkbutton;
	CtkCheckButton *show_map_checkbutton;
	CtkCheckButton *draw_spaces_checkbutton;
	CtkCheckButton *smart_backspace_checkbutton;
	CtkCheckButton *indent_width_checkbutton;
	CtkSpinButton *indent_width_spinbutton;
	CtkLabel *cursor_position_info;
	CtkSourceStyleSchemeChooserButton *chooser_button;
	CtkComboBoxText *background_pattern;
};

GType test_widget_get_type (void);

G_DEFINE_TYPE_WITH_PRIVATE (TestWidget, test_widget, CTK_TYPE_GRID)

#define MARK_TYPE_1      "one"
#define MARK_TYPE_2      "two"

static const char *cmd_filename;

static void
remove_all_marks (CtkSourceBuffer *buffer)
{
	CtkTextIter start;
	CtkTextIter end;

	ctk_text_buffer_get_bounds (CTK_TEXT_BUFFER (buffer), &start, &end);

	ctk_source_buffer_remove_source_marks (buffer, &start, &end, NULL);
}

static CtkSourceLanguage *
get_language_for_file (CtkTextBuffer *buffer,
		       const gchar   *filename)
{
	CtkSourceLanguageManager *manager;
	CtkSourceLanguage *language;
	CtkTextIter start;
	CtkTextIter end;
	gchar *text;
	gchar *content_type;
	gboolean result_uncertain;

	ctk_text_buffer_get_start_iter (buffer, &start);
	ctk_text_buffer_get_iter_at_offset (buffer, &end, 1024);
	text = ctk_text_buffer_get_slice (buffer, &start, &end, TRUE);

	content_type = g_content_type_guess (filename,
					     (guchar*) text,
					     strlen (text),
					     &result_uncertain);

	if (result_uncertain)
	{
		g_free (content_type);
		content_type = NULL;
	}

	manager = ctk_source_language_manager_get_default ();
	language = ctk_source_language_manager_guess_language (manager,
							       filename,
							       content_type);

	g_message ("Detected '%s' mime type for file %s, chose language %s",
		   content_type != NULL ? content_type : "(null)",
		   filename,
		   language != NULL ? ctk_source_language_get_id (language) : "(none)");

	g_free (content_type);
	g_free (text);
	return language;
}

static CtkSourceLanguage *
get_language_by_id (const gchar *id)
{
	CtkSourceLanguageManager *manager;
	manager = ctk_source_language_manager_get_default ();
	return ctk_source_language_manager_get_language (manager, id);
}

static CtkSourceLanguage *
get_language (CtkTextBuffer *buffer,
	      GFile         *location)
{
	CtkSourceLanguage *language = NULL;
	CtkTextIter start;
	CtkTextIter end;
	gchar *text;
	gchar *lang_string;

	ctk_text_buffer_get_start_iter (buffer, &start);
	end = start;
	ctk_text_iter_forward_line (&end);

#define LANG_STRING "ctk-source-lang:"

	text = ctk_text_iter_get_slice (&start, &end);
	lang_string = strstr (text, LANG_STRING);

	if (lang_string != NULL)
	{
		gchar **tokens;

		lang_string += strlen (LANG_STRING);
		g_strchug (lang_string);

		tokens = g_strsplit_set (lang_string, " \t\n", 2);

		if (tokens != NULL && tokens[0] != NULL)
		{
			language = get_language_by_id (tokens[0]);
		}

		g_strfreev (tokens);
	}

	if (language == NULL)
	{
		gchar *filename = g_file_get_path (location);
		language = get_language_for_file (buffer, filename);
		g_free (filename);
	}

	g_free (text);
	return language;
}

static void
print_language_style_ids (CtkSourceLanguage *language)
{
	gchar **styles;

	g_assert_nonnull (language);

	styles = ctk_source_language_get_style_ids (language);

	if (styles == NULL)
	{
		g_print ("No styles in language '%s'\n",
			 ctk_source_language_get_name (language));
	}
	else
	{
		gchar **ids;
		g_print ("Styles in language '%s':\n",
			 ctk_source_language_get_name (language));

		for (ids = styles; *ids != NULL; ids++)
		{
			const gchar *name = ctk_source_language_get_style_name (language, *ids);

			g_print ("- %s (name: '%s')\n", *ids, name);
		}

		g_strfreev (styles);
	}

	g_print ("\n");
}

static void
load_cb (CtkSourceFileLoader *loader,
	 GAsyncResult        *result,
	 TestWidget          *self)
{
	CtkTextIter iter;
	GFile *location;
	CtkSourceLanguage *language = NULL;
	GError *error = NULL;

	ctk_source_file_loader_load_finish (loader, result, &error);

	if (error != NULL)
	{
		g_warning ("Error while loading the file: %s", error->message);
		g_clear_error (&error);
		g_clear_object (&self->priv->file);
		goto end;
	}

	/* move cursor to the beginning */
	ctk_text_buffer_get_start_iter (CTK_TEXT_BUFFER (self->priv->buffer), &iter);
	ctk_text_buffer_place_cursor (CTK_TEXT_BUFFER (self->priv->buffer), &iter);
	ctk_widget_grab_focus (CTK_WIDGET (self->priv->view));

	location = ctk_source_file_loader_get_location (loader);

	language = get_language (CTK_TEXT_BUFFER (self->priv->buffer), location);
	ctk_source_buffer_set_language (self->priv->buffer, language);

	if (language != NULL)
	{
		print_language_style_ids (language);
	}
	else
	{
		gchar *path = g_file_get_path (location);
		g_print ("No language found for file '%s'\n", path);
		g_free (path);
	}

end:
	g_object_unref (loader);
}

static void
open_file (TestWidget *self,
	   GFile      *location)
{
	CtkSourceFileLoader *loader;

	g_clear_object (&self->priv->file);
	self->priv->file = ctk_source_file_new ();

	ctk_source_file_set_location (self->priv->file, location);

	loader = ctk_source_file_loader_new (self->priv->buffer,
					     self->priv->file);

	remove_all_marks (self->priv->buffer);

	ctk_source_file_loader_load_async (loader,
					   G_PRIORITY_DEFAULT,
					   NULL,
					   NULL, NULL, NULL,
					   (GAsyncReadyCallback) load_cb,
					   self);
}

static void
show_line_numbers_toggled_cb (TestWidget     *self,
			      CtkCheckButton *button)
{
	gboolean enabled = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (button));
	ctk_source_view_set_show_line_numbers (self->priv->view, enabled);
}

static void
show_line_marks_toggled_cb (TestWidget     *self,
			    CtkCheckButton *button)
{
	gboolean enabled = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (button));
	ctk_source_view_set_show_line_marks (self->priv->view, enabled);
}

static void
show_right_margin_toggled_cb (TestWidget     *self,
			      CtkCheckButton *button)
{
	gboolean enabled = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (button));
	ctk_source_view_set_show_right_margin (self->priv->view, enabled);
}

static void
right_margin_position_value_changed_cb (TestWidget    *self,
					CtkSpinButton *button)
{
	gint position = ctk_spin_button_get_value_as_int (button);
	ctk_source_view_set_right_margin_position (self->priv->view, position);
}

static void
highlight_syntax_toggled_cb (TestWidget     *self,
			     CtkCheckButton *button)
{
	gboolean enabled = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (button));
	ctk_source_buffer_set_highlight_syntax (self->priv->buffer, enabled);
}

static void
highlight_matching_bracket_toggled_cb (TestWidget     *self,
				       CtkCheckButton *button)
{
	gboolean enabled = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (button));
	ctk_source_buffer_set_highlight_matching_brackets (self->priv->buffer, enabled);
}

static void
highlight_current_line_toggled_cb (TestWidget     *self,
				   CtkCheckButton *button)
{
	gboolean enabled = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (button));
	ctk_source_view_set_highlight_current_line (self->priv->view, enabled);
}

static void
wrap_lines_toggled_cb (TestWidget     *self,
		       CtkCheckButton *button)
{
	gboolean enabled = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (button));
	ctk_text_view_set_wrap_mode (CTK_TEXT_VIEW (self->priv->view),
				     enabled ? CTK_WRAP_WORD : CTK_WRAP_NONE);
}

static void
auto_indent_toggled_cb (TestWidget     *self,
			CtkCheckButton *button)
{
	gboolean enabled = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (button));
	ctk_source_view_set_auto_indent (self->priv->view, enabled);
}

static void
indent_spaces_toggled_cb (TestWidget     *self,
			  CtkCheckButton *button)
{
	gboolean enabled = ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (button));
	ctk_source_view_set_insert_spaces_instead_of_tabs (self->priv->view, enabled);
}

static void
tab_width_value_changed_cb (TestWidget    *self,
			    CtkSpinButton *button)
{
	gint tab_width = ctk_spin_button_get_value_as_int (button);
	ctk_source_view_set_tab_width (self->priv->view, tab_width);
}

static void
update_indent_width (TestWidget *self)
{
	gint indent_width = -1;

	if (ctk_toggle_button_get_active (CTK_TOGGLE_BUTTON (self->priv->indent_width_checkbutton)))
	{
		indent_width = ctk_spin_button_get_value_as_int (self->priv->indent_width_spinbutton);
	}

	ctk_source_view_set_indent_width (self->priv->view, indent_width);
}

static void
smart_home_end_changed_cb (TestWidget  *self,
			   CtkComboBox *combo)
{
	CtkSourceSmartHomeEndType type;
	gint active = ctk_combo_box_get_active (combo);

	switch (active)
	{
		case 0:
			type = CTK_SOURCE_SMART_HOME_END_DISABLED;
			break;

		case 1:
			type = CTK_SOURCE_SMART_HOME_END_BEFORE;
			break;

		case 2:
			type = CTK_SOURCE_SMART_HOME_END_AFTER;
			break;

		case 3:
			type = CTK_SOURCE_SMART_HOME_END_ALWAYS;
			break;

		default:
			type = CTK_SOURCE_SMART_HOME_END_DISABLED;
			break;
	}

	ctk_source_view_set_smart_home_end (self->priv->view, type);
}

static void
backward_string_clicked_cb (TestWidget *self)
{
	CtkTextIter iter;
	CtkTextMark *insert;

	insert = ctk_text_buffer_get_insert (CTK_TEXT_BUFFER (self->priv->buffer));

	ctk_text_buffer_get_iter_at_mark (CTK_TEXT_BUFFER (self->priv->buffer),
	                                  &iter,
	                                  insert);

	if (ctk_source_buffer_iter_backward_to_context_class_toggle (self->priv->buffer,
	                                                             &iter,
	                                                             "string"))
	{
		ctk_text_buffer_place_cursor (CTK_TEXT_BUFFER (self->priv->buffer), &iter);
		ctk_text_view_scroll_mark_onscreen (CTK_TEXT_VIEW (self->priv->view), insert);
	}

	ctk_widget_grab_focus (CTK_WIDGET (self->priv->view));
}

static void
forward_string_clicked_cb (TestWidget *self)
{
	CtkTextIter iter;
	CtkTextMark *insert;

	insert = ctk_text_buffer_get_insert (CTK_TEXT_BUFFER (self->priv->buffer));

	ctk_text_buffer_get_iter_at_mark (CTK_TEXT_BUFFER (self->priv->buffer),
	                                  &iter,
	                                  insert);

	if (ctk_source_buffer_iter_forward_to_context_class_toggle (self->priv->buffer,
								    &iter,
								    "string"))
	{
		ctk_text_buffer_place_cursor (CTK_TEXT_BUFFER (self->priv->buffer), &iter);
		ctk_text_view_scroll_mark_onscreen (CTK_TEXT_VIEW (self->priv->view), insert);
	}

	ctk_widget_grab_focus (CTK_WIDGET (self->priv->view));
}

static void
open_button_clicked_cb (TestWidget *self)
{
	CtkWidget *main_window;
	CtkWidget *chooser;
	gint response;
	static gchar *last_dir;

	main_window = ctk_widget_get_toplevel (CTK_WIDGET (self->priv->view));

	chooser = ctk_file_chooser_dialog_new ("Open file...",
					       CTK_WINDOW (main_window),
					       CTK_FILE_CHOOSER_ACTION_OPEN,
					       "Cancel", CTK_RESPONSE_CANCEL,
					       "Open", CTK_RESPONSE_OK,
					       NULL);

	if (last_dir == NULL)
	{
		last_dir = g_strdup (TOP_SRCDIR "/ctksourceview");
	}

	if (last_dir != NULL && g_path_is_absolute (last_dir))
	{
		ctk_file_chooser_set_current_folder (CTK_FILE_CHOOSER (chooser),
						     last_dir);
	}

	response = ctk_dialog_run (CTK_DIALOG (chooser));

	if (response == CTK_RESPONSE_OK)
	{
		GFile *file;

		file = ctk_file_chooser_get_file (CTK_FILE_CHOOSER (chooser));

		if (file != NULL)
		{
			g_free (last_dir);
			last_dir = ctk_file_chooser_get_current_folder (CTK_FILE_CHOOSER (chooser));
			open_file (self, file);
			g_object_unref (file);
		}
	}

	ctk_widget_destroy (chooser);
}

#define NON_BLOCKING_PAGINATION

#ifndef NON_BLOCKING_PAGINATION

static void
begin_print (CtkPrintOperation        *operation,
	     CtkPrintContext          *context,
	     CtkSourcePrintCompositor *compositor)
{
	gint n_pages;

	while (!ctk_source_print_compositor_paginate (compositor, context))
		;

	n_pages = ctk_source_print_compositor_get_n_pages (compositor);
	ctk_print_operation_set_n_pages (operation, n_pages);
}

#else

static gboolean
paginate (CtkPrintOperation        *operation,
	  CtkPrintContext          *context,
	  CtkSourcePrintCompositor *compositor)
{
	g_print ("Pagination progress: %.2f %%\n", ctk_source_print_compositor_get_pagination_progress (compositor) * 100.0);

	if (ctk_source_print_compositor_paginate (compositor, context))
	{
		gint n_pages;

		g_assert_cmpint (ctk_source_print_compositor_get_pagination_progress (compositor), ==, 1.0);
		g_print ("Pagination progress: %.2f %%\n", ctk_source_print_compositor_get_pagination_progress (compositor) * 100.0);

		n_pages = ctk_source_print_compositor_get_n_pages (compositor);
		ctk_print_operation_set_n_pages (operation, n_pages);

		return TRUE;
	}

	return FALSE;
}

#endif

#define ENABLE_CUSTOM_OVERLAY

static void
draw_page (CtkPrintOperation        *operation,
	   CtkPrintContext          *context,
	   gint                      page_nr,
	   CtkSourcePrintCompositor *compositor)
{
#ifdef ENABLE_CUSTOM_OVERLAY

	/* This part of the code shows how to add a custom overlay to the
	   printed text generated by CtkSourcePrintCompositor */

	cairo_t *cr;
	PangoLayout *layout;
	PangoFontDescription *desc;
	PangoRectangle rect;


	cr = ctk_print_context_get_cairo_context (context);

	cairo_save (cr);

	layout = ctk_print_context_create_pango_layout (context);

	pango_layout_set_text (layout, "Draft", -1);

	desc = pango_font_description_from_string ("Sans Bold 120");
	pango_layout_set_font_description (layout, desc);
	pango_font_description_free (desc);


	pango_layout_get_extents (layout, NULL, &rect);

  	cairo_move_to (cr,
  		       (ctk_print_context_get_width (context) - ((double) rect.width / (double) PANGO_SCALE)) / 2,
  		       (ctk_print_context_get_height (context) - ((double) rect.height / (double) PANGO_SCALE)) / 2);

	pango_cairo_layout_path (cr, layout);

  	/* Font Outline */
	cairo_set_source_rgba (cr, 0.85, 0.85, 0.85, 0.80);
	cairo_set_line_width (cr, 0.5);
	cairo_stroke_preserve (cr);

	/* Font Fill */
	cairo_set_source_rgba (cr, 0.8, 0.8, 0.8, 0.60);
	cairo_fill (cr);

	g_object_unref (layout);
	cairo_restore (cr);
#endif

	/* To print page_nr you only need to call the following function */
	ctk_source_print_compositor_draw_page (compositor, context, page_nr);
}

static void
end_print (CtkPrintOperation        *operation,
	   CtkPrintContext          *context,
	   CtkSourcePrintCompositor *compositor)
{
	g_object_unref (compositor);
}

#define LINE_NUMBERS_FONT_NAME	"Sans 8"
#define HEADER_FONT_NAME	"Sans 11"
#define FOOTER_FONT_NAME	"Sans 11"
#define BODY_FONT_NAME		"Monospace 9"

/*
#define SETUP_FROM_VIEW
*/

#undef SETUP_FROM_VIEW

static void
print_button_clicked_cb (TestWidget *self)
{
	gchar *basename = NULL;
	CtkSourcePrintCompositor *compositor;
	CtkPrintOperation *operation;

	if (self->priv->file != NULL)
	{
		GFile *location;

		location = ctk_source_file_get_location (self->priv->file);

		if (location != NULL)
		{
			basename = g_file_get_basename (location);
		}
	}

#ifdef SETUP_FROM_VIEW
	compositor = ctk_source_print_compositor_new_from_view (self->priv->view);
#else
	compositor = ctk_source_print_compositor_new (self->priv->buffer);

	ctk_source_print_compositor_set_tab_width (compositor,
						   ctk_source_view_get_tab_width (self->priv->view));

	ctk_source_print_compositor_set_wrap_mode (compositor,
						   ctk_text_view_get_wrap_mode (CTK_TEXT_VIEW (self->priv->view)));

	ctk_source_print_compositor_set_print_line_numbers (compositor, 1);

	ctk_source_print_compositor_set_body_font_name (compositor,
							BODY_FONT_NAME);

	/* To test line numbers font != text font */
	ctk_source_print_compositor_set_line_numbers_font_name (compositor,
								LINE_NUMBERS_FONT_NAME);

	ctk_source_print_compositor_set_header_format (compositor,
						       TRUE,
						       "Printed on %A",
						       "test-widget",
						       "%F");

	ctk_source_print_compositor_set_footer_format (compositor,
						       TRUE,
						       "%T",
						       basename,
						       "Page %N/%Q");

	ctk_source_print_compositor_set_print_header (compositor, TRUE);
	ctk_source_print_compositor_set_print_footer (compositor, TRUE);

	ctk_source_print_compositor_set_header_font_name (compositor,
							  HEADER_FONT_NAME);

	ctk_source_print_compositor_set_footer_font_name (compositor,
							  FOOTER_FONT_NAME);
#endif
	operation = ctk_print_operation_new ();

	ctk_print_operation_set_job_name (operation, basename);

	ctk_print_operation_set_show_progress (operation, TRUE);

#ifndef NON_BLOCKING_PAGINATION
  	g_signal_connect (G_OBJECT (operation), "begin-print",
			  G_CALLBACK (begin_print), compositor);
#else
  	g_signal_connect (G_OBJECT (operation), "paginate",
			  G_CALLBACK (paginate), compositor);
#endif
	g_signal_connect (G_OBJECT (operation), "draw-page",
			  G_CALLBACK (draw_page), compositor);
	g_signal_connect (G_OBJECT (operation), "end-print",
			  G_CALLBACK (end_print), compositor);

	ctk_print_operation_run (operation,
				 CTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
				 NULL, NULL);

	g_object_unref (operation);
	g_free (basename);
}

static void
update_cursor_position_info (TestWidget *self)
{
	gchar *msg;
	gint offset;
	gint line;
	guint column;
	CtkTextIter iter;
	gchar **classes;
	gchar **classes_ptr;
	GString *classes_str;
	CtkSourceLanguage *lang;
	const char *language = "none";

	ctk_text_buffer_get_iter_at_mark (CTK_TEXT_BUFFER (self->priv->buffer),
					  &iter,
					  ctk_text_buffer_get_insert (CTK_TEXT_BUFFER (self->priv->buffer)));

	offset = ctk_text_iter_get_offset (&iter);
	line = ctk_text_iter_get_line (&iter) + 1;
	column = ctk_source_view_get_visual_column (self->priv->view, &iter) + 1;

	classes = ctk_source_buffer_get_context_classes_at_iter (self->priv->buffer, &iter);

	classes_str = g_string_new ("");

	for (classes_ptr = classes; classes_ptr != NULL && *classes_ptr != NULL; classes_ptr++)
	{
		if (classes_ptr != classes)
		{
			g_string_append (classes_str, ", ");
		}

		g_string_append_printf (classes_str, "%s", *classes_ptr);
	}

	g_strfreev (classes);

	if ((lang = ctk_source_buffer_get_language (self->priv->buffer)))
		language = ctk_source_language_get_id (lang);

	msg = g_strdup_printf ("language: %s offset: %d, line: %d, column: %u, classes: %s",
	                       language,
	                       offset,
	                       line,
	                       column,
	                       classes_str->str);

	ctk_label_set_text (self->priv->cursor_position_info, msg);

	g_free (msg);
	g_string_free (classes_str, TRUE);
}

static void
mark_set_cb (CtkTextBuffer *buffer,
	     CtkTextIter   *iter,
	     CtkTextMark   *mark,
	     TestWidget    *self)
{
	if (mark == ctk_text_buffer_get_insert (buffer))
	{
		update_cursor_position_info (self);
	}
}

static void
line_mark_activated_cb (CtkSourceGutter *gutter,
			CtkTextIter     *iter,
			CdkEventButton  *event,
			TestWidget      *self)
{
	GSList *mark_list;
	const gchar *mark_type;

	mark_type = event->button == 1 ? MARK_TYPE_1 : MARK_TYPE_2;

	/* get the marks already in the line */
	mark_list = ctk_source_buffer_get_source_marks_at_line (self->priv->buffer,
								ctk_text_iter_get_line (iter),
								mark_type);

	if (mark_list != NULL)
	{
		/* just take the first and delete it */
		ctk_text_buffer_delete_mark (CTK_TEXT_BUFFER (self->priv->buffer),
					     CTK_TEXT_MARK (mark_list->data));
	}
	else
	{
		/* no mark found: create one */
		ctk_source_buffer_create_source_mark (self->priv->buffer,
						      NULL,
						      mark_type,
						      iter);
	}

	g_slist_free (mark_list);
}

static void
bracket_matched_cb (CtkSourceBuffer           *buffer,
		    CtkTextIter               *iter,
		    CtkSourceBracketMatchType  state)
{
	GEnumClass *eclass;
	GEnumValue *evalue;

	eclass = G_ENUM_CLASS (g_type_class_ref (CTK_SOURCE_TYPE_BRACKET_MATCH_TYPE));
	evalue = g_enum_get_value (eclass, state);

	g_print ("Bracket match state: '%s'\n", evalue->value_nick);

	g_type_class_unref (eclass);

	if (state == CTK_SOURCE_BRACKET_MATCH_FOUND)
	{
		g_return_if_fail (iter != NULL);

		g_print ("Matched bracket: '%c' at row: %"G_GINT32_FORMAT", col: %"G_GINT32_FORMAT"\n",
		         ctk_text_iter_get_char (iter),
		         ctk_text_iter_get_line (iter) + 1,
		         ctk_text_iter_get_line_offset (iter) + 1);
	}
}

static gchar *
mark_tooltip_func (CtkSourceMarkAttributes *attrs,
                   CtkSourceMark           *mark,
                   CtkSourceView           *view)
{
	CtkTextBuffer *buffer;
	CtkTextIter iter;
	gint line;
	gint column;

	buffer = ctk_text_mark_get_buffer (CTK_TEXT_MARK (mark));

	ctk_text_buffer_get_iter_at_mark (buffer, &iter, CTK_TEXT_MARK (mark));
	line = ctk_text_iter_get_line (&iter) + 1;
	column = ctk_text_iter_get_line_offset (&iter);

	if (g_strcmp0 (ctk_source_mark_get_category (mark), MARK_TYPE_1) == 0)
	{
		return g_strdup_printf ("Line: %d, Column: %d", line, column);
	}
	else
	{
		return g_strdup_printf ("<b>Line</b>: %d\n<i>Column</i>: %d", line, column);
	}
}

static void
add_source_mark_attributes (CtkSourceView *view)
{
	CdkRGBA color;
	CtkSourceMarkAttributes *attrs;

	attrs = ctk_source_mark_attributes_new ();

	cdk_rgba_parse (&color, "lightgreen");
	ctk_source_mark_attributes_set_background (attrs, &color);

	ctk_source_mark_attributes_set_icon_name (attrs, "list-add");

	g_signal_connect_object (attrs,
				 "query-tooltip-markup",
				 G_CALLBACK (mark_tooltip_func),
				 view,
				 0);

	ctk_source_view_set_mark_attributes (view, MARK_TYPE_1, attrs, 1);
	g_object_unref (attrs);

	attrs = ctk_source_mark_attributes_new ();

	cdk_rgba_parse (&color, "pink");
	ctk_source_mark_attributes_set_background (attrs, &color);

	ctk_source_mark_attributes_set_icon_name (attrs, "list-remove");

	g_signal_connect_object (attrs,
				 "query-tooltip-markup",
				 G_CALLBACK (mark_tooltip_func),
				 view,
				 0);

	ctk_source_view_set_mark_attributes (view, MARK_TYPE_2, attrs, 2);
	g_object_unref (attrs);
}

static void
on_background_pattern_changed (CtkComboBox *combobox,
                               TestWidget  *self)
{
	gchar *text;

	text = ctk_combo_box_text_get_active_text (CTK_COMBO_BOX_TEXT (combobox));

	if (g_strcmp0 (text, "Grid") == 0)
	{
		ctk_source_view_set_background_pattern (self->priv->view,
		                                        CTK_SOURCE_BACKGROUND_PATTERN_TYPE_GRID);
	}
	else
	{
		ctk_source_view_set_background_pattern (self->priv->view,
		                                        CTK_SOURCE_BACKGROUND_PATTERN_TYPE_NONE);
	}

	g_free (text);
}

static void
test_widget_dispose (GObject *object)
{
	TestWidget *self = TEST_WIDGET (object);

	g_clear_object (&self->priv->buffer);
	g_clear_object (&self->priv->file);

	G_OBJECT_CLASS (test_widget_parent_class)->dispose (object);
}

static void
test_widget_class_init (TestWidgetClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	CtkWidgetClass *widget_class = CTK_WIDGET_CLASS (klass);

	object_class->dispose = test_widget_dispose;

	ctk_widget_class_set_template_from_resource (widget_class,
						     "/org/gnome/ctksourceview/tests/ui/test-widget.ui");

	ctk_widget_class_bind_template_callback (widget_class, open_button_clicked_cb);
	ctk_widget_class_bind_template_callback (widget_class, print_button_clicked_cb);
	ctk_widget_class_bind_template_callback (widget_class, highlight_syntax_toggled_cb);
	ctk_widget_class_bind_template_callback (widget_class, highlight_matching_bracket_toggled_cb);
	ctk_widget_class_bind_template_callback (widget_class, show_line_numbers_toggled_cb);
	ctk_widget_class_bind_template_callback (widget_class, show_line_marks_toggled_cb);
	ctk_widget_class_bind_template_callback (widget_class, show_right_margin_toggled_cb);
	ctk_widget_class_bind_template_callback (widget_class, right_margin_position_value_changed_cb);
	ctk_widget_class_bind_template_callback (widget_class, highlight_current_line_toggled_cb);
	ctk_widget_class_bind_template_callback (widget_class, wrap_lines_toggled_cb);
	ctk_widget_class_bind_template_callback (widget_class, auto_indent_toggled_cb);
	ctk_widget_class_bind_template_callback (widget_class, indent_spaces_toggled_cb);
	ctk_widget_class_bind_template_callback (widget_class, tab_width_value_changed_cb);
	ctk_widget_class_bind_template_callback (widget_class, backward_string_clicked_cb);
	ctk_widget_class_bind_template_callback (widget_class, forward_string_clicked_cb);
	ctk_widget_class_bind_template_callback (widget_class, smart_home_end_changed_cb);

	ctk_widget_class_bind_template_child_private (widget_class, TestWidget, view);
	ctk_widget_class_bind_template_child_private (widget_class, TestWidget, map);
	ctk_widget_class_bind_template_child_private (widget_class, TestWidget, show_top_border_window_checkbutton);
	ctk_widget_class_bind_template_child_private (widget_class, TestWidget, show_map_checkbutton);
	ctk_widget_class_bind_template_child_private (widget_class, TestWidget, draw_spaces_checkbutton);
	ctk_widget_class_bind_template_child_private (widget_class, TestWidget, smart_backspace_checkbutton);
	ctk_widget_class_bind_template_child_private (widget_class, TestWidget, indent_width_checkbutton);
	ctk_widget_class_bind_template_child_private (widget_class, TestWidget, indent_width_spinbutton);
	ctk_widget_class_bind_template_child_private (widget_class, TestWidget, cursor_position_info);
	ctk_widget_class_bind_template_child_private (widget_class, TestWidget, chooser_button);
	ctk_widget_class_bind_template_child_private (widget_class, TestWidget, background_pattern);
}

static void
show_top_border_window_toggled_cb (CtkToggleButton *checkbutton,
				   TestWidget      *self)
{
	gint size;

	size = ctk_toggle_button_get_active (checkbutton) ? 20 : 0;

	ctk_text_view_set_border_window_size (CTK_TEXT_VIEW (self->priv->view),
					      CTK_TEXT_WINDOW_TOP,
					      size);
}

static void
test_widget_init (TestWidget *self)
{
	CtkSourceSpaceDrawer *space_drawer;
	GFile *file;

	self->priv = test_widget_get_instance_private (self);

	ctk_widget_init_template (CTK_WIDGET (self));

	self->priv->buffer = CTK_SOURCE_BUFFER (
		ctk_text_view_get_buffer (CTK_TEXT_VIEW (self->priv->view)));

	g_signal_connect_swapped (self->priv->buffer,
	                          "notify::language",
	                          G_CALLBACK (update_cursor_position_info),
	                          self);

	g_object_ref (self->priv->buffer);

	g_signal_connect (self->priv->show_top_border_window_checkbutton,
			  "toggled",
			  G_CALLBACK (show_top_border_window_toggled_cb),
			  self);

	g_signal_connect_swapped (self->priv->indent_width_checkbutton,
				  "toggled",
				  G_CALLBACK (update_indent_width),
				  self);

	g_signal_connect_swapped (self->priv->indent_width_spinbutton,
				  "value-changed",
				  G_CALLBACK (update_indent_width),
				  self);

	g_signal_connect (self->priv->buffer,
			  "mark-set",
			  G_CALLBACK (mark_set_cb),
			  self);

	g_signal_connect_swapped (self->priv->buffer,
				  "changed",
				  G_CALLBACK (update_cursor_position_info),
				  self);

	g_signal_connect (self->priv->buffer,
			  "bracket-matched",
			  G_CALLBACK (bracket_matched_cb),
			  NULL);

	add_source_mark_attributes (self->priv->view);

	g_signal_connect (self->priv->view,
			  "line-mark-activated",
			  G_CALLBACK (line_mark_activated_cb),
			  self);

	g_object_bind_property (self->priv->chooser_button,
	                        "style-scheme",
	                        self->priv->buffer,
	                        "style-scheme",
	                        G_BINDING_SYNC_CREATE);

	g_object_bind_property (self->priv->show_map_checkbutton,
	                        "active",
	                        self->priv->map,
	                        "visible",
	                        G_BINDING_SYNC_CREATE);

	g_object_bind_property (self->priv->smart_backspace_checkbutton,
	                        "active",
	                        self->priv->view,
	                        "smart-backspace",
	                        G_BINDING_SYNC_CREATE);

	g_signal_connect (self->priv->background_pattern,
	                  "changed",
	                  G_CALLBACK (on_background_pattern_changed),
	                  self);

	space_drawer = ctk_source_view_get_space_drawer (self->priv->view);
	g_object_bind_property (self->priv->draw_spaces_checkbutton, "active",
				space_drawer, "enable-matrix",
				G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

	if (cmd_filename)
		file = g_file_new_for_commandline_arg (cmd_filename);
	else
		file = g_file_new_for_path (TOP_SRCDIR "/ctksourceview/ctksourcebuffer.c");

	open_file (self, file);
	g_object_unref (file);
}

static TestWidget *
test_widget_new (void)
{
	return g_object_new (test_widget_get_type (), NULL);
}

int
main (int argc, char *argv[])
{
	CtkWidget *window;
	TestWidget *test_widget;

	ctk_init (&argc, &argv);
	ctk_source_init ();

	if (argc == 2 && g_file_test (argv[1], G_FILE_TEST_IS_REGULAR))
	{
		cmd_filename = argv[1];
	}

	window = ctk_window_new (CTK_WINDOW_TOPLEVEL);
	ctk_window_set_default_size (CTK_WINDOW (window), 900, 600);

	g_signal_connect (window, "destroy", ctk_main_quit, NULL);

	test_widget = test_widget_new ();
	ctk_container_add (CTK_CONTAINER (window), CTK_WIDGET (test_widget));

	ctk_widget_show (window);

	ctk_main ();

	ctk_source_finalize ();

	return 0;
}
