/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 1999-2002 - Mikael Hermansson <tyan@linux.se>,
 *                           Chris Phelps <chicane@reninet.com> and
 *                           Jeroen Zwartepoorte <jeroen@xs4all.nl>
 * Copyright (C) 2003 - Paolo Maggi <paolo.maggi@polito.it> and
 *                      Gustavo Giráldez <gustavo.giraldez@gmx.net>
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

#include "ctksourcebuffer.h"
#include "ctksourcebuffer-private.h"

#include <string.h>
#include <stdlib.h>
#include <ctk/ctk.h>

#include "ctksourcelanguage.h"
#include "ctksourcelanguage-private.h"
#include "ctksource-marshal.h"
#include "ctksourceundomanager.h"
#include "ctksourceundomanagerdefault.h"
#include "ctksourcestyle.h"
#include "ctksourcestylescheme.h"
#include "ctksourcestyleschememanager.h"
#include "ctksourcemark.h"
#include "ctksourcemarkssequence.h"
#include "ctksourcesearchcontext.h"
#include "ctksourcetag.h"
#include "ctksource-enumtypes.h"

/**
 * SECTION:buffer
 * @Short_description: Subclass of #CtkTextBuffer
 * @Title: CtkSourceBuffer
 * @See_also: #CtkTextBuffer, #CtkSourceView
 *
 * A #CtkSourceBuffer object is the model for #CtkSourceView widgets.
 * It extends the #CtkTextBuffer class by adding features useful to display
 * and edit source code such as syntax highlighting and bracket matching. It
 * also implements support for the undo/redo.
 *
 * To create a #CtkSourceBuffer use ctk_source_buffer_new() or
 * ctk_source_buffer_new_with_language(). The second form is just a convenience
 * function which allows you to initially set a #CtkSourceLanguage. You can also
 * directly create a #CtkSourceView and get its #CtkSourceBuffer with
 * ctk_text_view_get_buffer().
 *
 * The highlighting is enabled by default, but you can disable it with
 * ctk_source_buffer_set_highlight_syntax().
 *
 * # Undo/Redo
 *
 * A custom #CtkSourceUndoManager can be implemented and set with
 * ctk_source_buffer_set_undo_manager(). However the default implementation
 * should be suitable for most uses, so you can use the API provided by
 * #CtkSourceBuffer instead of using #CtkSourceUndoManager. By default, actions
 * that can be undone or redone are defined as groups of operations between a
 * call to ctk_text_buffer_begin_user_action() and
 * ctk_text_buffer_end_user_action(). In general, this happens whenever the user
 * presses any key which modifies the buffer. But the default undo manager will
 * try to merge similar consecutive actions into one undo/redo level. The
 * merging is done word by word, so after writing a new sentence (character by
 * character), each undo will remove the previous word.
 *
 * The default undo manager remembers the "modified" state of the buffer, and
 * restores it when an action is undone or redone. It can be useful in a text
 * editor to know whether the file is saved. See ctk_text_buffer_get_modified()
 * and ctk_text_buffer_set_modified().
 *
 * The default undo manager also restores the selected text (or cursor
 * position), if the selection was related to the action. For example if the
 * user selects some text and deletes it, an undo will restore the selection. On
 * the other hand, if some text is selected but a deletion occurs elsewhere (the
 * deletion was done programmatically), an undo will not restore the selection,
 * it will only moves the cursor (the cursor is moved so that the user sees the
 * undo's effect). Warning: the selection restoring behavior might change in the
 * future.
 *
 * # Context Classes # {#context-classes}
 *
 * It is possible to retrieve some information from the syntax highlighting
 * engine. The default context classes that are applied to regions of a
 * #CtkSourceBuffer:
 *  - <emphasis>comment</emphasis>: the region delimits a comment;
 *  - <emphasis>no-spell-check</emphasis>: the region should not be spell checked;
 *  - <emphasis>path</emphasis>: the region delimits a path to a file;
 *  - <emphasis>string</emphasis>: the region delimits a string.
 *
 * Custom language definition files can create their own context classes,
 * since the functions like ctk_source_buffer_iter_has_context_class() take
 * a string parameter as the context class.
 *
 * #CtkSourceBuffer provides an API to access the context classes:
 * ctk_source_buffer_iter_has_context_class(),
 * ctk_source_buffer_get_context_classes_at_iter(),
 * ctk_source_buffer_iter_forward_to_context_class_toggle() and
 * ctk_source_buffer_iter_backward_to_context_class_toggle().
 *
 * And the #CtkSourceBuffer::highlight-updated signal permits to be notified
 * when a context class region changes.
 *
 * Each context class has also an associated #CtkTextTag with the name
 * <emphasis>ctksourceview:context-classes:&lt;name&gt;</emphasis>. For example to
 * retrieve the #CtkTextTag for the string context class, one can write:
 * |[
 * CtkTextTagTable *tag_table;
 * CtkTextTag *tag;
 *
 * tag_table = ctk_text_buffer_get_tag_table (buffer);
 * tag = ctk_text_tag_table_lookup (tag_table, "ctksourceview:context-classes:string");
 * ]|
 *
 * The tag must be used for read-only purposes.
 *
 * Accessing a context class via the associated #CtkTextTag is less
 * convenient than the #CtkSourceBuffer API, because:
 *  - The tag doesn't always exist, you need to listen to the
 *    #CtkTextTagTable::tag-added and #CtkTextTagTable::tag-removed signals.
 *  - Instead of the #CtkSourceBuffer::highlight-updated signal, you can listen
 *    to the #CtkTextBuffer::apply-tag and #CtkTextBuffer::remove-tag signals.
 *
 * A possible use-case for accessing a context class via the associated
 * #CtkTextTag is to read the region but without adding a hard dependency on the
 * CtkSourceView library (for example for a spell-checking library that wants to
 * read the no-spell-check region).
 */

/*
#define ENABLE_DEBUG
#define ENABLE_PROFILE
*/
#undef ENABLE_DEBUG
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

#define UPDATE_BRACKET_DELAY		50
#define BRACKET_MATCHING_CHARS_LIMIT	10000
#define CONTEXT_CLASSES_PREFIX		"ctksourceview:context-classes:"

enum
{
	HIGHLIGHT_UPDATED,
	SOURCE_MARK_UPDATED,
	UNDO,
	REDO,
	BRACKET_MATCHED,
	N_SIGNALS
};

enum
{
	PROP_0,
	PROP_CAN_UNDO,
	PROP_CAN_REDO,
	PROP_HIGHLIGHT_SYNTAX,
	PROP_HIGHLIGHT_MATCHING_BRACKETS,
	PROP_MAX_UNDO_LEVELS,
	PROP_LANGUAGE,
	PROP_STYLE_SCHEME,
	PROP_UNDO_MANAGER,
	PROP_IMPLICIT_TRAILING_NEWLINE,
	N_PROPERTIES
};

struct _CtkSourceBufferPrivate
{
	CtkTextTag *bracket_match_tag;
	CtkSourceBracketMatchType bracket_match_state;
	guint bracket_highlighting_timeout_id;

	/* Hash table: category -> MarksSequence */
	GHashTable *source_marks;
	CtkSourceMarksSequence *all_source_marks;

	CtkSourceStyleScheme *style_scheme;
	CtkSourceLanguage *language;
	CtkSourceEngine *highlight_engine;

	CtkSourceUndoManager *undo_manager;
	gint max_undo_levels;

	CtkTextMark *tmp_insert_mark;
	CtkTextMark *tmp_selection_bound_mark;

	GList *search_contexts;

	CtkTextTag *invalid_char_tag;

	guint has_draw_spaces_tag : 1;
	guint highlight_syntax : 1;
	guint highlight_brackets : 1;
	guint implicit_trailing_newline : 1;
};

static guint buffer_signals[N_SIGNALS];
static GParamSpec *buffer_properties[N_PROPERTIES];

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceBuffer, ctk_source_buffer, CTK_TYPE_TEXT_BUFFER)

/* Prototypes */
static void 	 ctk_source_buffer_dispose		(GObject                 *object);
static void      ctk_source_buffer_set_property         (GObject                 *object,
							 guint                    prop_id,
							 const GValue            *value,
							 GParamSpec              *pspec);
static void      ctk_source_buffer_get_property         (GObject                 *object,
							 guint                    prop_id,
							 GValue                  *value,
							 GParamSpec              *pspec);
static void 	 ctk_source_buffer_can_undo_handler 	(CtkSourceUndoManager    *manager,
							 CtkSourceBuffer         *buffer);
static void 	 ctk_source_buffer_can_redo_handler	(CtkSourceUndoManager    *manager,
							 CtkSourceBuffer         *buffer);
static void 	 ctk_source_buffer_real_insert_text 	(CtkTextBuffer           *buffer,
							 CtkTextIter             *iter,
							 const gchar             *text,
							 gint                     len);
static void	 ctk_source_buffer_real_insert_pixbuf	(CtkTextBuffer           *buffer,
							 CtkTextIter             *pos,
							 GdkPixbuf               *pixbuf);
static void	 ctk_source_buffer_real_insert_child_anchor
							(CtkTextBuffer           *buffer,
							 CtkTextIter             *pos,
							 CtkTextChildAnchor      *anchor);
static void 	 ctk_source_buffer_real_delete_range 	(CtkTextBuffer           *buffer,
							 CtkTextIter             *iter,
							 CtkTextIter             *end);
static void 	 ctk_source_buffer_real_mark_set	(CtkTextBuffer		 *buffer,
							 const CtkTextIter	 *location,
							 CtkTextMark		 *mark);

static void 	 ctk_source_buffer_real_mark_deleted	(CtkTextBuffer		 *buffer,
							 CtkTextMark		 *mark);

static void	 ctk_source_buffer_real_undo		(CtkSourceBuffer	 *buffer);
static void	 ctk_source_buffer_real_redo		(CtkSourceBuffer	 *buffer);

static void	 ctk_source_buffer_real_highlight_updated
							(CtkSourceBuffer         *buffer,
							 CtkTextIter             *start,
							 CtkTextIter             *end);

static void
ctk_source_buffer_check_tag_for_spaces (CtkSourceBuffer *buffer,
                                        CtkSourceTag    *tag)
{
	if (!buffer->priv->has_draw_spaces_tag)
	{
		gboolean draw_spaces_set;

		g_object_get (tag,
		              "draw-spaces-set", &draw_spaces_set,
		              NULL);

		if (draw_spaces_set)
		{
			buffer->priv->has_draw_spaces_tag = TRUE;
		}
	}
}

static void
ctk_source_buffer_tag_changed_cb (CtkTextTagTable *table,
                                  CtkTextTag      *tag,
                                  gboolean         size_changed,
                                  CtkSourceBuffer *buffer)
{
	if (CTK_SOURCE_IS_TAG (tag))
	{
		ctk_source_buffer_check_tag_for_spaces (buffer, CTK_SOURCE_TAG (tag));
	}
}

static void
ctk_source_buffer_tag_added_cb (CtkTextTagTable *table,
                                CtkTextTag      *tag,
                                CtkSourceBuffer *buffer)
{
	if (CTK_SOURCE_IS_TAG (tag))
	{
		ctk_source_buffer_check_tag_for_spaces (buffer, CTK_SOURCE_TAG (tag));
	}
}

static void
ctk_source_buffer_constructed (GObject *object)
{
	CtkSourceBuffer *buffer = CTK_SOURCE_BUFFER (object);
	CtkTextTagTable *table;

	if (buffer->priv->undo_manager == NULL)
	{
		/* This will install the default undo manager */
		ctk_source_buffer_set_undo_manager (buffer, NULL);
	}

	G_OBJECT_CLASS (ctk_source_buffer_parent_class)->constructed (object);

	table = ctk_text_buffer_get_tag_table (CTK_TEXT_BUFFER (buffer));
	g_signal_connect_object (table,
	                         "tag-changed",
	                         G_CALLBACK (ctk_source_buffer_tag_changed_cb),
	                         buffer, 0);
	g_signal_connect_object (table,
	                         "tag-added",
	                         G_CALLBACK (ctk_source_buffer_tag_added_cb),
	                         buffer, 0);
}

static void
ctk_source_buffer_class_init (CtkSourceBufferClass *klass)
{
	GObjectClass *object_class;
	CtkTextBufferClass *text_buffer_class;

	object_class = G_OBJECT_CLASS (klass);
	text_buffer_class = CTK_TEXT_BUFFER_CLASS (klass);

	object_class->constructed = ctk_source_buffer_constructed;
	object_class->dispose = ctk_source_buffer_dispose;
	object_class->get_property = ctk_source_buffer_get_property;
	object_class->set_property = ctk_source_buffer_set_property;

	text_buffer_class->delete_range = ctk_source_buffer_real_delete_range;
	text_buffer_class->insert_text = ctk_source_buffer_real_insert_text;
	text_buffer_class->insert_pixbuf = ctk_source_buffer_real_insert_pixbuf;
	text_buffer_class->insert_child_anchor = ctk_source_buffer_real_insert_child_anchor;
	text_buffer_class->mark_set = ctk_source_buffer_real_mark_set;
	text_buffer_class->mark_deleted = ctk_source_buffer_real_mark_deleted;

	klass->undo = ctk_source_buffer_real_undo;
	klass->redo = ctk_source_buffer_real_redo;

	/**
	 * CtkSourceBuffer:highlight-syntax:
	 *
	 * Whether to highlight syntax in the buffer.
	 */
	buffer_properties[PROP_HIGHLIGHT_SYNTAX] =
		g_param_spec_boolean ("highlight-syntax",
				      "Highlight Syntax",
				      "Whether to highlight syntax in the buffer",
				      TRUE,
				      G_PARAM_READWRITE |
				      G_PARAM_STATIC_STRINGS);

	/**
	 * CtkSourceBuffer:highlight-matching-brackets:
	 *
	 * Whether to highlight matching brackets in the buffer.
	 */
	buffer_properties[PROP_HIGHLIGHT_MATCHING_BRACKETS] =
		g_param_spec_boolean ("highlight-matching-brackets",
				      "Highlight Matching Brackets",
				      "Whether to highlight matching brackets",
				      TRUE,
				      G_PARAM_READWRITE |
				      G_PARAM_STATIC_STRINGS);

	/**
	 * CtkSourceBuffer:max-undo-levels:
	 *
	 * Number of undo levels for the buffer. -1 means no limit. This property
	 * will only affect the default undo manager.
	 */
	buffer_properties[PROP_MAX_UNDO_LEVELS] =
		g_param_spec_int ("max-undo-levels",
				  "Maximum Undo Levels",
				  "Number of undo levels for the buffer",
				  -1,
				  G_MAXINT,
				  -1,
				  G_PARAM_READWRITE |
				  G_PARAM_STATIC_STRINGS);

	buffer_properties[PROP_LANGUAGE] =
		g_param_spec_object ("language",
				     "Language",
				     "Language object to get highlighting patterns from",
				     CTK_SOURCE_TYPE_LANGUAGE,
				     G_PARAM_READWRITE |
				     G_PARAM_STATIC_STRINGS);

	buffer_properties[PROP_CAN_UNDO] =
		g_param_spec_boolean ("can-undo",
				      "Can undo",
				      "Whether Undo operation is possible",
				      FALSE,
				      G_PARAM_READABLE |
				      G_PARAM_STATIC_STRINGS);

	buffer_properties[PROP_CAN_REDO] =
		g_param_spec_boolean ("can-redo",
				      "Can redo",
				      "Whether Redo operation is possible",
				      FALSE,
				      G_PARAM_READABLE |
				      G_PARAM_STATIC_STRINGS);

	/**
	 * CtkSourceBuffer:style-scheme:
	 *
	 * Style scheme. It contains styles for syntax highlighting, optionally
	 * foreground, background, cursor color, current line color, and matching
	 * brackets style.
	 */
	buffer_properties[PROP_STYLE_SCHEME] =
		g_param_spec_object ("style-scheme",
				     "Style scheme",
				     "Style scheme",
				     CTK_SOURCE_TYPE_STYLE_SCHEME,
				     G_PARAM_READWRITE |
				     G_PARAM_STATIC_STRINGS);

	buffer_properties[PROP_UNDO_MANAGER] =
		g_param_spec_object ("undo-manager",
				     "Undo manager",
				     "The buffer undo manager",
				     CTK_SOURCE_TYPE_UNDO_MANAGER,
				     G_PARAM_READWRITE |
				     G_PARAM_CONSTRUCT |
				     G_PARAM_STATIC_STRINGS);

	/**
	 * CtkSourceBuffer:implicit-trailing-newline:
	 *
	 * Whether the buffer has an implicit trailing newline. See
	 * ctk_source_buffer_set_implicit_trailing_newline().
	 *
	 * Since: 3.14
	 */
	buffer_properties[PROP_IMPLICIT_TRAILING_NEWLINE] =
		g_param_spec_boolean ("implicit-trailing-newline",
				      "Implicit trailing newline",
				      "",
				      TRUE,
				      G_PARAM_READWRITE |
				      G_PARAM_CONSTRUCT |
				      G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, N_PROPERTIES, buffer_properties);

	/**
	 * CtkSourceBuffer::highlight-updated:
	 * @buffer: the buffer that received the signal
	 * @start: the start of the updated region
	 * @end: the end of the updated region
	 *
	 * The ::highlight-updated signal is emitted when the syntax
	 * highlighting and [context classes][context-classes] are updated in a
	 * certain region of the @buffer.
	 */
	buffer_signals[HIGHLIGHT_UPDATED] =
	    g_signal_new_class_handler ("highlight-updated",
	                                G_OBJECT_CLASS_TYPE (object_class),
	                                G_SIGNAL_RUN_LAST,
	                                G_CALLBACK (ctk_source_buffer_real_highlight_updated),
					NULL, NULL,
					_ctk_source_marshal_VOID__BOXED_BOXED,
	                                G_TYPE_NONE,
	                                2,
	                                CTK_TYPE_TEXT_ITER | G_SIGNAL_TYPE_STATIC_SCOPE,
	                                CTK_TYPE_TEXT_ITER | G_SIGNAL_TYPE_STATIC_SCOPE);
	g_signal_set_va_marshaller (buffer_signals[HIGHLIGHT_UPDATED],
	                            G_TYPE_FROM_CLASS (klass),
	                            _ctk_source_marshal_VOID__BOXED_BOXEDv);

	/**
	 * CtkSourceBuffer::source-mark-updated:
	 * @buffer: the buffer that received the signal
	 * @mark: the #CtkSourceMark
	 *
	 * The ::source-mark-updated signal is emitted each time
	 * a mark is added to, moved or removed from the @buffer.
	 */
	buffer_signals[SOURCE_MARK_UPDATED] =
	    g_signal_new ("source-mark-updated",
			   G_OBJECT_CLASS_TYPE (object_class),
			   G_SIGNAL_RUN_LAST,
			   0,
			   NULL, NULL,
			   g_cclosure_marshal_VOID__OBJECT,
			   G_TYPE_NONE,
			   1, CTK_TYPE_TEXT_MARK);
	g_signal_set_va_marshaller (buffer_signals[SOURCE_MARK_UPDATED],
	                            G_TYPE_FROM_CLASS (klass),
	                            g_cclosure_marshal_VOID__OBJECTv);

	/**
	 * CtkSourceBuffer::undo:
	 * @buffer: the buffer that received the signal
	 *
	 * The ::undo signal is emitted to undo the last user action which
	 * modified the buffer.
	 */
	buffer_signals[UNDO] =
	    g_signal_new ("undo",
			  G_OBJECT_CLASS_TYPE (object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (CtkSourceBufferClass, undo),
			  NULL, NULL,
			  g_cclosure_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);
	g_signal_set_va_marshaller (buffer_signals[UNDO],
	                            G_TYPE_FROM_CLASS (klass),
	                            g_cclosure_marshal_VOID__VOIDv);

	/**
	 * CtkSourceBuffer::redo:
	 * @buffer: the buffer that received the signal
	 *
	 * The ::redo signal is emitted to redo the last undo operation.
	 */
	buffer_signals[REDO] =
	    g_signal_new ("redo",
			  G_OBJECT_CLASS_TYPE (object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (CtkSourceBufferClass, redo),
			  NULL, NULL,
			  g_cclosure_marshal_VOID__VOID,
			  G_TYPE_NONE, 0);
	g_signal_set_va_marshaller (buffer_signals[REDO],
	                            G_TYPE_FROM_CLASS (klass),
	                            g_cclosure_marshal_VOID__VOIDv);

	/**
	 * CtkSourceBuffer::bracket-matched:
	 * @buffer: a #CtkSourceBuffer.
	 * @iter: (nullable): if found, the location of the matching bracket.
	 * @state: state of bracket matching.
	 *
	 * @iter is set to a valid iterator pointing to the matching bracket
	 * if @state is %CTK_SOURCE_BRACKET_MATCH_FOUND. Otherwise @iter is
	 * meaningless.
	 *
	 * The signal is emitted only when the @state changes, typically when
	 * the cursor moves.
	 *
	 * A use-case for this signal is to show messages in a #CtkStatusbar.
	 *
	 * Since: 2.12
	 */
	buffer_signals[BRACKET_MATCHED] =
	    g_signal_new ("bracket-matched",
			  G_OBJECT_CLASS_TYPE (object_class),
			  G_SIGNAL_RUN_LAST,
			  G_STRUCT_OFFSET (CtkSourceBufferClass, bracket_matched),
			  NULL, NULL,
			  _ctk_source_marshal_VOID__BOXED_ENUM,
			  G_TYPE_NONE, 2,
			  CTK_TYPE_TEXT_ITER,
			  CTK_SOURCE_TYPE_BRACKET_MATCH_TYPE);
	g_signal_set_va_marshaller (buffer_signals[BRACKET_MATCHED],
	                            G_TYPE_FROM_CLASS (klass),
	                            _ctk_source_marshal_VOID__BOXED_ENUMv);
}

static void
set_undo_manager (CtkSourceBuffer      *buffer,
                  CtkSourceUndoManager *manager)
{
	if (manager == buffer->priv->undo_manager)
	{
		return;
	}

	if (buffer->priv->undo_manager != NULL)
	{
		g_signal_handlers_disconnect_by_func (buffer->priv->undo_manager,
		                                      G_CALLBACK (ctk_source_buffer_can_undo_handler),
		                                      buffer);

		g_signal_handlers_disconnect_by_func (buffer->priv->undo_manager,
		                                      G_CALLBACK (ctk_source_buffer_can_redo_handler),
		                                      buffer);

		g_object_unref (buffer->priv->undo_manager);
		buffer->priv->undo_manager = NULL;
	}

	if (manager != NULL)
	{
		buffer->priv->undo_manager = g_object_ref (manager);

		g_signal_connect (buffer->priv->undo_manager,
		                  "can-undo-changed",
		                  G_CALLBACK (ctk_source_buffer_can_undo_handler),
		                  buffer);

		g_signal_connect (buffer->priv->undo_manager,
		                  "can-redo-changed",
		                  G_CALLBACK (ctk_source_buffer_can_redo_handler),
		                  buffer);

		/* Notify possible changes in the can-undo/redo state */
		g_object_notify_by_pspec (G_OBJECT (buffer), buffer_properties[PROP_CAN_UNDO]);
		g_object_notify_by_pspec (G_OBJECT (buffer), buffer_properties[PROP_CAN_REDO]);
	}
}

static void
search_context_weak_notify_cb (CtkSourceBuffer *buffer,
			       GObject         *search_context)
{
	buffer->priv->search_contexts = g_list_remove (buffer->priv->search_contexts,
						       search_context);
}

static void
ctk_source_buffer_init (CtkSourceBuffer *buffer)
{
	CtkSourceBufferPrivate *priv = ctk_source_buffer_get_instance_private (buffer);

	buffer->priv = priv;

	priv->highlight_syntax = TRUE;
	priv->highlight_brackets = TRUE;
	priv->bracket_match_state = CTK_SOURCE_BRACKET_MATCH_NONE;
	priv->max_undo_levels = -1;

	priv->source_marks = g_hash_table_new_full (g_str_hash,
						    g_str_equal,
						    (GDestroyNotify)g_free,
						    (GDestroyNotify)g_object_unref);

	priv->all_source_marks = _ctk_source_marks_sequence_new (CTK_TEXT_BUFFER (buffer));

	priv->style_scheme = _ctk_source_style_scheme_get_default ();

	if (priv->style_scheme != NULL)
	{
		g_object_ref (priv->style_scheme);
	}
}

static void
ctk_source_buffer_dispose (GObject *object)
{
	CtkSourceBuffer *buffer = CTK_SOURCE_BUFFER (object);
	GList *l;

	if (buffer->priv->bracket_highlighting_timeout_id != 0)
	{
		g_source_remove (buffer->priv->bracket_highlighting_timeout_id);
		buffer->priv->bracket_highlighting_timeout_id = 0;
	}

	if (buffer->priv->undo_manager != NULL)
	{
		set_undo_manager (buffer, NULL);
	}

	if (buffer->priv->highlight_engine != NULL)
	{
		_ctk_source_engine_attach_buffer (buffer->priv->highlight_engine, NULL);
	}

	g_clear_object (&buffer->priv->highlight_engine);
	g_clear_object (&buffer->priv->language);
	g_clear_object (&buffer->priv->style_scheme);

	for (l = buffer->priv->search_contexts; l != NULL; l = l->next)
	{
		CtkSourceSearchContext *search_context = l->data;

		g_object_weak_unref (G_OBJECT (search_context),
				     (GWeakNotify)search_context_weak_notify_cb,
				     buffer);
	}

	g_list_free (buffer->priv->search_contexts);
	buffer->priv->search_contexts = NULL;

	g_clear_object (&buffer->priv->all_source_marks);

	if (buffer->priv->source_marks != NULL)
	{
		g_hash_table_unref (buffer->priv->source_marks);
		buffer->priv->source_marks = NULL;
	}

	G_OBJECT_CLASS (ctk_source_buffer_parent_class)->dispose (object);
}

static void
ctk_source_buffer_set_property (GObject      *object,
				guint         prop_id,
				const GValue *value,
				GParamSpec   *pspec)
{
	CtkSourceBuffer *buffer;

	g_return_if_fail (CTK_SOURCE_IS_BUFFER (object));

	buffer = CTK_SOURCE_BUFFER (object);

	switch (prop_id)
	{
		case PROP_HIGHLIGHT_SYNTAX:
			ctk_source_buffer_set_highlight_syntax (buffer, g_value_get_boolean (value));
			break;

		case PROP_HIGHLIGHT_MATCHING_BRACKETS:
			ctk_source_buffer_set_highlight_matching_brackets (buffer, g_value_get_boolean (value));
			break;

		case PROP_MAX_UNDO_LEVELS:
			ctk_source_buffer_set_max_undo_levels (buffer, g_value_get_int (value));
			break;

		case PROP_LANGUAGE:
			ctk_source_buffer_set_language (buffer, g_value_get_object (value));
			break;

		case PROP_STYLE_SCHEME:
			ctk_source_buffer_set_style_scheme (buffer, g_value_get_object (value));
			break;

		case PROP_UNDO_MANAGER:
			ctk_source_buffer_set_undo_manager (buffer, g_value_get_object (value));
			break;

		case PROP_IMPLICIT_TRAILING_NEWLINE:
			ctk_source_buffer_set_implicit_trailing_newline (buffer, g_value_get_boolean (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
ctk_source_buffer_get_property (GObject    *object,
				guint       prop_id,
				GValue     *value,
				GParamSpec *pspec)
{
	CtkSourceBuffer *buffer;

	g_return_if_fail (CTK_SOURCE_IS_BUFFER (object));

	buffer = CTK_SOURCE_BUFFER (object);

	switch (prop_id)
	{
		case PROP_HIGHLIGHT_SYNTAX:
			g_value_set_boolean (value, buffer->priv->highlight_syntax);
			break;

		case PROP_HIGHLIGHT_MATCHING_BRACKETS:
			g_value_set_boolean (value, buffer->priv->highlight_brackets);
			break;

		case PROP_MAX_UNDO_LEVELS:
			g_value_set_int (value, buffer->priv->max_undo_levels);
			break;

		case PROP_LANGUAGE:
			g_value_set_object (value, buffer->priv->language);
			break;

		case PROP_STYLE_SCHEME:
			g_value_set_object (value, buffer->priv->style_scheme);
			break;

		case PROP_CAN_UNDO:
			g_value_set_boolean (value, ctk_source_buffer_can_undo (buffer));
			break;

		case PROP_CAN_REDO:
			g_value_set_boolean (value, ctk_source_buffer_can_redo (buffer));
			break;

		case PROP_UNDO_MANAGER:
			g_value_set_object (value, buffer->priv->undo_manager);
			break;

		case PROP_IMPLICIT_TRAILING_NEWLINE:
			g_value_set_boolean (value, buffer->priv->implicit_trailing_newline);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

/**
 * ctk_source_buffer_new:
 * @table: (nullable): a #CtkTextTagTable, or %NULL to create a new one.
 *
 * Creates a new source buffer.
 *
 * Returns: a new source buffer.
 */
CtkSourceBuffer *
ctk_source_buffer_new (CtkTextTagTable *table)
{
	return g_object_new (CTK_SOURCE_TYPE_BUFFER,
			     "tag-table", table,
			     NULL);
}

/**
 * ctk_source_buffer_new_with_language:
 * @language: a #CtkSourceLanguage.
 *
 * Creates a new source buffer using the highlighting patterns in
 * @language.  This is equivalent to creating a new source buffer with
 * a new tag table and then calling ctk_source_buffer_set_language().
 *
 * Returns: a new source buffer which will highlight text
 * according to the highlighting patterns in @language.
 */
CtkSourceBuffer *
ctk_source_buffer_new_with_language (CtkSourceLanguage *language)
{
	g_return_val_if_fail (CTK_SOURCE_IS_LANGUAGE (language), NULL);

	return g_object_new (CTK_SOURCE_TYPE_BUFFER,
			     "tag-table", NULL,
			     "language", language,
			     NULL);
}

static void
ctk_source_buffer_can_undo_handler (CtkSourceUndoManager *manager,
                                    CtkSourceBuffer      *buffer)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));

	g_object_notify_by_pspec (G_OBJECT (buffer), buffer_properties[PROP_CAN_UNDO]);
}

static void
ctk_source_buffer_can_redo_handler (CtkSourceUndoManager *manager,
                                    CtkSourceBuffer      *buffer)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));

	g_object_notify_by_pspec (G_OBJECT (buffer), buffer_properties[PROP_CAN_REDO]);
}

static void
update_bracket_match_style (CtkSourceBuffer *buffer)
{
	CtkSourceStyle *style = NULL;

	if (buffer->priv->bracket_match_tag == NULL)
	{
		return;
	}

	if (buffer->priv->style_scheme != NULL)
	{
		style = _ctk_source_style_scheme_get_matching_brackets_style (buffer->priv->style_scheme);
	}

	ctk_source_style_apply (style, buffer->priv->bracket_match_tag);
}

static CtkTextTag *
get_bracket_match_tag (CtkSourceBuffer *buffer)
{
	if (buffer->priv->bracket_match_tag == NULL)
	{
		buffer->priv->bracket_match_tag =
			ctk_text_buffer_create_tag (CTK_TEXT_BUFFER (buffer),
						    NULL,
						    NULL);
		update_bracket_match_style (buffer);
	}

	return buffer->priv->bracket_match_tag;
}

/* This is private, just used by the print compositor to not print bracket
 * matches. Note that unlike get_bracket_match_tag() it returns NULL
 * if the tag is not set.
 */
CtkTextTag *
_ctk_source_buffer_get_bracket_match_tag (CtkSourceBuffer *buffer)
{
	return buffer->priv->bracket_match_tag;
}

static gunichar
bracket_pair (gunichar  base_char,
	      gint     *direction)
{
	gint dir;
	gunichar pair;

	switch (base_char)
	{
		case '{':
			dir = 1;
			pair = '}';
			break;

		case '(':
			dir = 1;
			pair = ')';
			break;

		case '[':
			dir = 1;
			pair = ']';
			break;

		case '<':
			dir = 1;
			pair = '>';
			break;

		case '}':
			dir = -1;
			pair = '{';
			break;

		case ')':
			dir = -1;
			pair = '(';
			break;

		case ']':
			dir = -1;
			pair = '[';
			break;

		case '>':
			dir = -1;
			pair = '<';
			break;

		default:
			dir = 0;
			pair = 0;
			break;
	}

	if (direction != NULL)
	{
		*direction = dir;
	}

	return pair;
}

/*
 * This function works similar to ctk_text_buffer_remove_tag() except that
 * instead of taking the optimization to make removing tags fast in terms
 * of wall clock time, it tries to avoiding to much time of the screen
 * by minimizing the damage regions. This results in fewer full-redraws
 * when updating the text marks. To see the difference, compare this to
 * ctk_text_buffer_remove_tag() and enable the "show pixel cache" feature
 * the CTK+ inspector.
 */
static void
remove_tag_with_minimal_damage (CtkTextBuffer     *buffer,
                                CtkTextTag        *tag,
                                const CtkTextIter *begin,
                                const CtkTextIter *end)
{
	CtkTextIter tag_begin = *begin;
	CtkTextIter tag_end;

	if (!ctk_text_iter_starts_tag (&tag_begin, tag))
	{
		if (!ctk_text_iter_forward_to_tag_toggle (&tag_begin, tag))
		{
			return;
		}
	}

	while (ctk_text_iter_starts_tag (&tag_begin, tag) &&
	       ctk_text_iter_compare (&tag_begin, end) < 0)
	{
		gint count = 1;

		tag_end = tag_begin;

		/*
		 * We might have found the start of another tag embedded
		 * inside this tag. So keep scanning forward until we have
		 * reached the right number of end tags.
		 */

		while (ctk_text_iter_forward_to_tag_toggle (&tag_end, tag))
		{
			if (ctk_text_iter_starts_tag (&tag_end, tag))
			{
				count++;
			}
			else if (ctk_text_iter_ends_tag (&tag_end, tag))
			{
				if (--count == 0)
				{
					break;
				}
			}
		}

		if (ctk_text_iter_ends_tag (&tag_end, tag))
		{
			ctk_text_buffer_remove_tag (buffer, tag, &tag_begin, &tag_end);

			tag_begin = tag_end;

			/*
			 * Move to the next start tag. It's possible to have an overlapped
			 * end tag, which would be non-ideal, but possible.
			 */
			if (!ctk_text_iter_starts_tag (&tag_begin, tag))
			{
				while (ctk_text_iter_forward_to_tag_toggle (&tag_begin, tag))
				{
					if (ctk_text_iter_starts_tag (&tag_begin, tag))
					{
						break;
					}
				}
			}
		}
	}
}

static void
update_bracket_highlighting (CtkSourceBuffer *source_buffer)
{
	CtkTextBuffer *buffer;
	CtkTextIter insert_iter;
	CtkTextIter bracket;
	CtkTextIter bracket_match;
	CtkSourceBracketMatchType previous_state;

	buffer = CTK_TEXT_BUFFER (source_buffer);

	if (source_buffer->priv->bracket_match_tag != NULL)
	{
		CtkTextIter start;
		CtkTextIter end;

		ctk_text_buffer_get_bounds (buffer, &start, &end);

		remove_tag_with_minimal_damage (CTK_TEXT_BUFFER (source_buffer),
		                                source_buffer->priv->bracket_match_tag,
		                                &start,
		                                &end);
	}

	if (!source_buffer->priv->highlight_brackets)
	{
		if (source_buffer->priv->bracket_match_tag != NULL)
		{
			CtkTextTagTable *table;

			table = ctk_text_buffer_get_tag_table (buffer);
			ctk_text_tag_table_remove (table, source_buffer->priv->bracket_match_tag);
			source_buffer->priv->bracket_match_tag = NULL;
		}

		return;
	}

	ctk_text_buffer_get_iter_at_mark (buffer,
					  &insert_iter,
					  ctk_text_buffer_get_insert (buffer));

	previous_state = source_buffer->priv->bracket_match_state;
	source_buffer->priv->bracket_match_state =
		_ctk_source_buffer_find_bracket_match (source_buffer,
						       &insert_iter,
						       &bracket,
						       &bracket_match);

	if (source_buffer->priv->bracket_match_state == CTK_SOURCE_BRACKET_MATCH_FOUND)
	{
		CtkTextIter next_iter;

		g_signal_emit (source_buffer,
			       buffer_signals[BRACKET_MATCHED],
			       0,
			       &bracket_match,
			       CTK_SOURCE_BRACKET_MATCH_FOUND);

		next_iter = bracket_match;
		ctk_text_iter_forward_char (&next_iter);
		ctk_text_buffer_apply_tag (buffer,
					   get_bracket_match_tag (source_buffer),
					   &bracket_match,
					   &next_iter);

		next_iter = bracket;
		ctk_text_iter_forward_char (&next_iter);
		ctk_text_buffer_apply_tag (buffer,
					   get_bracket_match_tag (source_buffer),
					   &bracket,
					   &next_iter);
		return;
	}

	/* Don't emit the signal at all if chars at previous and current
	 * positions are nonbrackets.
	 */
	if (previous_state != CTK_SOURCE_BRACKET_MATCH_NONE ||
	    source_buffer->priv->bracket_match_state != CTK_SOURCE_BRACKET_MATCH_NONE)
	{
		g_signal_emit (source_buffer,
			       buffer_signals[BRACKET_MATCHED],
			       0,
			       NULL,
			       source_buffer->priv->bracket_match_state);
	}
}

static gboolean
bracket_highlighting_timeout_cb (gpointer user_data)
{
	CtkSourceBuffer *buffer = CTK_SOURCE_BUFFER (user_data);

	update_bracket_highlighting (buffer);

	buffer->priv->bracket_highlighting_timeout_id = 0;
	return G_SOURCE_REMOVE;
}

static void
queue_bracket_highlighting_update (CtkSourceBuffer *buffer)
{
	if (buffer->priv->bracket_highlighting_timeout_id != 0)
	{
		g_source_remove (buffer->priv->bracket_highlighting_timeout_id);
	}

	/* Queue an update to the bracket location instead of doing it
	 * immediately. We are likely going to be servicing a draw deadline
	 * immediately, so blocking to find the match and invalidating
	 * visible regions causes animations to stutter. Instead, give
	 * ourself just a little bit of a delay to catch up.
	 *
	 * The value for this delay was found experimentally, as 25msec
	 * resulted in continuing to see frame stutter, but 50 was not
	 * distinguishable from having matching-brackets disabled.
	 * The animation in ctkscrolledwindow is 200, but that creates
	 * an undesireable delay before the match is shown to the user.
	 * 50msec errors on the side of "immediate", but without the
	 * frame stutter.
	 *
	 * If we had access to a GdkFrameClock, we might consider using
	 * ::update() or ::after-paint() to synchronize this.
	 */
	buffer->priv->bracket_highlighting_timeout_id =
		gdk_threads_add_timeout_full (G_PRIORITY_LOW,
					      UPDATE_BRACKET_DELAY,
					      bracket_highlighting_timeout_cb,
					      buffer,
					      NULL);
}

/* Although this function is not really useful
 * (queue_bracket_highlighting_update() could be called directly), the name
 * cursor_moved() is more meaningful.
 */
static void
cursor_moved (CtkSourceBuffer *buffer)
{
	queue_bracket_highlighting_update (buffer);
}

static void
ctk_source_buffer_real_highlight_updated (CtkSourceBuffer *buffer,
					  CtkTextIter     *start,
					  CtkTextIter     *end)
{
	queue_bracket_highlighting_update (buffer);
}

static void
ctk_source_buffer_content_inserted (CtkTextBuffer *buffer,
				    gint           start_offset,
				    gint           end_offset)
{
	CtkSourceBuffer *source_buffer = CTK_SOURCE_BUFFER (buffer);

	cursor_moved (source_buffer);

	if (source_buffer->priv->highlight_engine != NULL)
	{
		_ctk_source_engine_text_inserted (source_buffer->priv->highlight_engine,
						  start_offset,
						  end_offset);
	}
}

static void
ctk_source_buffer_real_insert_text (CtkTextBuffer *buffer,
				    CtkTextIter   *iter,
				    const gchar   *text,
				    gint           len)
{
	gint start_offset;

	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));
	g_return_if_fail (iter != NULL);
	g_return_if_fail (text != NULL);
	g_return_if_fail (ctk_text_iter_get_buffer (iter) == buffer);

	start_offset = ctk_text_iter_get_offset (iter);

	/* iter is invalidated when
	 * insertion occurs (because the buffer contents change), but the
	 * default signal handler revalidates it to point to the end of the
	 * inserted text.
	 */
	CTK_TEXT_BUFFER_CLASS (ctk_source_buffer_parent_class)->insert_text (buffer, iter, text, len);

	ctk_source_buffer_content_inserted (buffer,
					    start_offset,
					    ctk_text_iter_get_offset (iter));
}

/* insert_pixbuf and insert_child_anchor do nothing except notifying
 * the highlighting engine about the change, because engine's idea
 * of buffer char count must be correct at all times.
 */
static void
ctk_source_buffer_real_insert_pixbuf (CtkTextBuffer *buffer,
				      CtkTextIter   *iter,
				      GdkPixbuf     *pixbuf)
{
	gint start_offset;

	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));
	g_return_if_fail (iter != NULL);
	g_return_if_fail (ctk_text_iter_get_buffer (iter) == buffer);

	start_offset = ctk_text_iter_get_offset (iter);

	/* iter is invalidated when
	 * insertion occurs (because the buffer contents change), but the
	 * default signal handler revalidates it to point to the end of the
	 * inserted text.
	 */
	CTK_TEXT_BUFFER_CLASS (ctk_source_buffer_parent_class)->insert_pixbuf (buffer, iter, pixbuf);

	ctk_source_buffer_content_inserted (buffer,
					    start_offset,
					    ctk_text_iter_get_offset (iter));
}

static void
ctk_source_buffer_real_insert_child_anchor (CtkTextBuffer      *buffer,
					    CtkTextIter        *iter,
					    CtkTextChildAnchor *anchor)
{
	gint start_offset;

	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));
	g_return_if_fail (iter != NULL);
	g_return_if_fail (ctk_text_iter_get_buffer (iter) == buffer);

	start_offset = ctk_text_iter_get_offset (iter);

	/* iter is invalidated when insertion occurs (because the buffer
	 * contents change), but the default signal handler revalidates it to
	 * point to the end of the inserted text.
	 */
	CTK_TEXT_BUFFER_CLASS (ctk_source_buffer_parent_class)->insert_child_anchor (buffer, iter, anchor);

	ctk_source_buffer_content_inserted (buffer,
					    start_offset,
					    ctk_text_iter_get_offset (iter));
}

static void
ctk_source_buffer_real_delete_range (CtkTextBuffer *buffer,
				     CtkTextIter   *start,
				     CtkTextIter   *end)
{
	gint offset, length;
	CtkSourceBuffer *source_buffer = CTK_SOURCE_BUFFER (buffer);

	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));
	g_return_if_fail (start != NULL);
	g_return_if_fail (end != NULL);
	g_return_if_fail (ctk_text_iter_get_buffer (start) == buffer);
	g_return_if_fail (ctk_text_iter_get_buffer (end) == buffer);

	ctk_text_iter_order (start, end);
	offset = ctk_text_iter_get_offset (start);
	length = ctk_text_iter_get_offset (end) - offset;

	CTK_TEXT_BUFFER_CLASS (ctk_source_buffer_parent_class)->delete_range (buffer, start, end);

	cursor_moved (source_buffer);

	/* emit text deleted for engines */
	if (source_buffer->priv->highlight_engine != NULL)
	{
		_ctk_source_engine_text_deleted (source_buffer->priv->highlight_engine,
						 offset, length);
	}
}

static gint
get_bracket_matching_context_class_mask (CtkSourceBuffer *buffer,
					 CtkTextIter     *iter)
{
	gint mask = 0;
	guint i;

	/* This describes a mask of relevant context classes for highlighting
	 * matching brackets.
	 */
	const gchar *cclass_mask_definitions[] = {
		"comment",
		"string",
	};

	for (i = 0; i < G_N_ELEMENTS (cclass_mask_definitions); ++i)
	{
		gboolean has_class;

		has_class = ctk_source_buffer_iter_has_context_class (buffer,
								      iter,
								      cclass_mask_definitions[i]);

		mask |= has_class << i;
	}

	return mask;
}

/* Note that we only look BRACKET_MATCHING_CHARS_LIMIT at most.
 * @pos is moved to the bracket match, if found.
 */
static CtkSourceBracketMatchType
find_bracket_match_real (CtkSourceBuffer *buffer,
			 CtkTextIter     *pos)
{
	CtkTextIter iter;
	gunichar base_char;
	gunichar search_char;
	gint direction;
	gint bracket_count;
	gint char_count;
	gint cclass_mask;
	gboolean found;

	base_char = ctk_text_iter_get_char (pos);
	search_char = bracket_pair (base_char, &direction);

	if (direction == 0)
	{
		return CTK_SOURCE_BRACKET_MATCH_NONE;
	}

	cclass_mask = get_bracket_matching_context_class_mask (buffer, pos);

	iter = *pos;
	bracket_count = 0;
	char_count = 0;
	found = FALSE;

	do
	{
		gunichar cur_char;
		gint cur_mask;

		ctk_text_iter_forward_chars (&iter, direction);
		cur_char = ctk_text_iter_get_char (&iter);
		char_count++;

		cur_mask = get_bracket_matching_context_class_mask (buffer, &iter);

		/* Check if we lost a class, which means we don't look any
		 * further.
		 */
		if ((cclass_mask & cur_mask) != cclass_mask)
		{
			found = FALSE;
			break;
		}

		if (cclass_mask != cur_mask)
		{
			continue;
		}

		if (cur_char == search_char)
		{
			if (bracket_count == 0)
			{
				found = TRUE;
				break;
			}

			bracket_count--;
		}
		else if (cur_char == base_char)
		{
			bracket_count++;
		}
	}
	while (!ctk_text_iter_is_end (&iter) &&
	       !ctk_text_iter_is_start (&iter) &&
	       char_count < BRACKET_MATCHING_CHARS_LIMIT);

	if (found)
	{
		*pos = iter;
		return CTK_SOURCE_BRACKET_MATCH_FOUND;
	}

	if (char_count >= BRACKET_MATCHING_CHARS_LIMIT)
	{
		return CTK_SOURCE_BRACKET_MATCH_OUT_OF_RANGE;
	}

	return CTK_SOURCE_BRACKET_MATCH_NOT_FOUND;
}

/* Note that we take into account both the character following @pos and the one
 * preceding it. If there are brackets on both sides, the one following @pos
 * takes precedence.
 * @bracket and @bracket_match are valid only if CTK_SOURCE_BRACKET_MATCH_FOUND
 * is returned. @bracket is set either to @pos or @pos-1.
 */
CtkSourceBracketMatchType
_ctk_source_buffer_find_bracket_match (CtkSourceBuffer   *buffer,
				       const CtkTextIter *pos,
				       CtkTextIter       *bracket,
				       CtkTextIter       *bracket_match)
{
	CtkSourceBracketMatchType result_right;
	CtkSourceBracketMatchType result_left;
	CtkTextIter prev;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), CTK_SOURCE_BRACKET_MATCH_NONE);
	g_return_val_if_fail (pos != NULL, CTK_SOURCE_BRACKET_MATCH_NONE);
	g_return_val_if_fail (bracket_match != NULL, CTK_SOURCE_BRACKET_MATCH_NONE);

	*bracket_match = *pos;
	result_right = find_bracket_match_real (buffer, bracket_match);

	if (result_right == CTK_SOURCE_BRACKET_MATCH_FOUND)
	{
		if (bracket != NULL)
		{
			*bracket = *pos;
		}

		return CTK_SOURCE_BRACKET_MATCH_FOUND;
	}

	prev = *pos;
	if (!ctk_text_iter_starts_line (&prev) &&
	    ctk_text_iter_backward_cursor_position (&prev))
	{
		*bracket_match = prev;
		result_left = find_bracket_match_real (buffer, bracket_match);
	}
	else
	{
		result_left = CTK_SOURCE_BRACKET_MATCH_NONE;
	}

	if (result_left == CTK_SOURCE_BRACKET_MATCH_FOUND)
	{
		if (bracket != NULL)
		{
			*bracket = prev;
		}

		return CTK_SOURCE_BRACKET_MATCH_FOUND;
	}

	/* If there is a bracket, the expected return value is for the bracket,
	 * not the other character.
	 */
	if (result_right == CTK_SOURCE_BRACKET_MATCH_NONE)
	{
		return result_left;
	}
	if (result_left == CTK_SOURCE_BRACKET_MATCH_NONE)
	{
		return result_right;
	}

	/* There are brackets on both sides, and none was successful. The one on
	 * the right takes precedence.
	 */
	return result_right;
}

/**
 * ctk_source_buffer_can_undo:
 * @buffer: a #CtkSourceBuffer.
 *
 * Determines whether a source buffer can undo the last action.
 *
 * Returns: %TRUE if it's possible to undo the last action.
 */
gboolean
ctk_source_buffer_can_undo (CtkSourceBuffer *buffer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), FALSE);

	return ctk_source_undo_manager_can_undo (buffer->priv->undo_manager);
}

/**
 * ctk_source_buffer_can_redo:
 * @buffer: a #CtkSourceBuffer.
 *
 * Determines whether a source buffer can redo the last action
 * (i.e. if the last operation was an undo).
 *
 * Returns: %TRUE if a redo is possible.
 */
gboolean
ctk_source_buffer_can_redo (CtkSourceBuffer *buffer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), FALSE);

	return ctk_source_undo_manager_can_redo (buffer->priv->undo_manager);
}

/**
 * ctk_source_buffer_undo:
 * @buffer: a #CtkSourceBuffer.
 *
 * Undoes the last user action which modified the buffer.  Use
 * ctk_source_buffer_can_undo() to check whether a call to this
 * function will have any effect.
 *
 * This function emits the #CtkSourceBuffer::undo signal.
 */
void
ctk_source_buffer_undo (CtkSourceBuffer *buffer)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));

	g_signal_emit (buffer, buffer_signals[UNDO], 0);
}

/**
 * ctk_source_buffer_redo:
 * @buffer: a #CtkSourceBuffer.
 *
 * Redoes the last undo operation.  Use ctk_source_buffer_can_redo()
 * to check whether a call to this function will have any effect.
 *
 * This function emits the #CtkSourceBuffer::redo signal.
 */
void
ctk_source_buffer_redo (CtkSourceBuffer *buffer)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));

	g_signal_emit (buffer, buffer_signals[REDO], 0);
}

/**
 * ctk_source_buffer_get_max_undo_levels:
 * @buffer: a #CtkSourceBuffer.
 *
 * Determines the number of undo levels the buffer will track for buffer edits.
 *
 * Returns: the maximum number of possible undo levels or -1 if no limit is set.
 */
gint
ctk_source_buffer_get_max_undo_levels (CtkSourceBuffer *buffer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), 0);

	return buffer->priv->max_undo_levels;
}

/**
 * ctk_source_buffer_set_max_undo_levels:
 * @buffer: a #CtkSourceBuffer.
 * @max_undo_levels: the desired maximum number of undo levels.
 *
 * Sets the number of undo levels for user actions the buffer will
 * track.  If the number of user actions exceeds the limit set by this
 * function, older actions will be discarded.
 *
 * If @max_undo_levels is -1, the undo/redo is unlimited.
 *
 * If @max_undo_levels is 0, the undo/redo is disabled.
 */
void
ctk_source_buffer_set_max_undo_levels (CtkSourceBuffer *buffer,
				       gint             max_undo_levels)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));

	if (buffer->priv->max_undo_levels == max_undo_levels)
	{
		return;
	}

	buffer->priv->max_undo_levels = max_undo_levels;

	if (CTK_SOURCE_IS_UNDO_MANAGER_DEFAULT (buffer->priv->undo_manager))
	{
		ctk_source_undo_manager_default_set_max_undo_levels (CTK_SOURCE_UNDO_MANAGER_DEFAULT (buffer->priv->undo_manager),
		                                                     max_undo_levels);
	}

	g_object_notify_by_pspec (G_OBJECT (buffer), buffer_properties[PROP_MAX_UNDO_LEVELS]);
}

gboolean
_ctk_source_buffer_is_undo_redo_enabled (CtkSourceBuffer *buffer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), FALSE);

	if (buffer->priv->undo_manager == NULL)
	{
		return FALSE;
	}

	/* A custom UndoManager is not forced to follow max_undo_levels. */
	if (!CTK_SOURCE_IS_UNDO_MANAGER_DEFAULT (buffer->priv->undo_manager))
	{
		return TRUE;
	}

	return buffer->priv->max_undo_levels != 0;
}

/**
 * ctk_source_buffer_begin_not_undoable_action:
 * @buffer: a #CtkSourceBuffer.
 *
 * Marks the beginning of a not undoable action on the buffer,
 * disabling the undo manager.  Typically you would call this function
 * before initially setting the contents of the buffer (e.g. when
 * loading a file in a text editor).
 *
 * You may nest ctk_source_buffer_begin_not_undoable_action() /
 * ctk_source_buffer_end_not_undoable_action() blocks.
 */
void
ctk_source_buffer_begin_not_undoable_action (CtkSourceBuffer *buffer)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));

	ctk_source_undo_manager_begin_not_undoable_action (buffer->priv->undo_manager);
}

/**
 * ctk_source_buffer_end_not_undoable_action:
 * @buffer: a #CtkSourceBuffer.
 *
 * Marks the end of a not undoable action on the buffer.  When the
 * last not undoable block is closed through the call to this
 * function, the list of undo actions is cleared and the undo manager
 * is re-enabled.
 */
void
ctk_source_buffer_end_not_undoable_action (CtkSourceBuffer *buffer)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));

	ctk_source_undo_manager_end_not_undoable_action (buffer->priv->undo_manager);
}

/**
 * ctk_source_buffer_get_highlight_matching_brackets:
 * @buffer: a #CtkSourceBuffer.
 *
 * Determines whether bracket match highlighting is activated for the
 * source buffer.
 *
 * Returns: %TRUE if the source buffer will highlight matching
 * brackets.
 */
gboolean
ctk_source_buffer_get_highlight_matching_brackets (CtkSourceBuffer *buffer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), FALSE);

	return buffer->priv->highlight_brackets;
}

/**
 * ctk_source_buffer_set_highlight_matching_brackets:
 * @buffer: a #CtkSourceBuffer.
 * @highlight: %TRUE if you want matching brackets highlighted.
 *
 * Controls the bracket match highlighting function in the buffer.  If
 * activated, when you position your cursor over a bracket character
 * (a parenthesis, a square bracket, etc.) the matching opening or
 * closing bracket character will be highlighted.
 */
void
ctk_source_buffer_set_highlight_matching_brackets (CtkSourceBuffer *buffer,
						   gboolean         highlight)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));

	highlight = highlight != FALSE;

	if (highlight != buffer->priv->highlight_brackets)
	{
		buffer->priv->highlight_brackets = highlight;

		update_bracket_highlighting (buffer);

		g_object_notify_by_pspec (G_OBJECT (buffer), buffer_properties[PROP_HIGHLIGHT_MATCHING_BRACKETS]);
	}
}

/**
 * ctk_source_buffer_get_highlight_syntax:
 * @buffer: a #CtkSourceBuffer.
 *
 * Determines whether syntax highlighting is activated in the source
 * buffer.
 *
 * Returns: %TRUE if syntax highlighting is enabled, %FALSE otherwise.
 */
gboolean
ctk_source_buffer_get_highlight_syntax (CtkSourceBuffer *buffer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), FALSE);

	return buffer->priv->highlight_syntax;
}

/**
 * ctk_source_buffer_set_highlight_syntax:
 * @buffer: a #CtkSourceBuffer.
 * @highlight: %TRUE to enable syntax highlighting, %FALSE to disable it.
 *
 * Controls whether syntax is highlighted in the buffer.
 *
 * If @highlight is %TRUE, the text will be highlighted according to the syntax
 * patterns specified in the #CtkSourceLanguage set with
 * ctk_source_buffer_set_language().
 *
 * If @highlight is %FALSE, syntax highlighting is disabled and all the
 * #CtkTextTag objects that have been added by the syntax highlighting engine
 * are removed from the buffer.
 */
void
ctk_source_buffer_set_highlight_syntax (CtkSourceBuffer *buffer,
					gboolean         highlight)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));

	highlight = highlight != FALSE;

	if (buffer->priv->highlight_syntax != highlight)
	{
		buffer->priv->highlight_syntax = highlight;
		g_object_notify_by_pspec (G_OBJECT (buffer), buffer_properties[PROP_HIGHLIGHT_SYNTAX]);
	}
}

/**
 * ctk_source_buffer_set_language:
 * @buffer: a #CtkSourceBuffer.
 * @language: (nullable): a #CtkSourceLanguage to set, or %NULL.
 *
 * Associates a #CtkSourceLanguage with the buffer.
 *
 * Note that a #CtkSourceLanguage affects not only the syntax highlighting, but
 * also the [context classes][context-classes]. If you want to disable just the
 * syntax highlighting, see ctk_source_buffer_set_highlight_syntax().
 *
 * The buffer holds a reference to @language.
 */
void
ctk_source_buffer_set_language (CtkSourceBuffer   *buffer,
				CtkSourceLanguage *language)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));
	g_return_if_fail (CTK_SOURCE_IS_LANGUAGE (language) || language == NULL);

	if (!g_set_object (&buffer->priv->language, language))
	{
		return;
	}

	if (buffer->priv->highlight_engine != NULL)
	{
		/* disconnect the old engine */
		_ctk_source_engine_attach_buffer (buffer->priv->highlight_engine, NULL);
		g_object_unref (buffer->priv->highlight_engine);
		buffer->priv->highlight_engine = NULL;
	}

	if (language != NULL)
	{
		/* get a new engine */
		buffer->priv->highlight_engine = _ctk_source_language_create_engine (language);

		if (buffer->priv->highlight_engine != NULL)
		{
			_ctk_source_engine_attach_buffer (buffer->priv->highlight_engine,
							  CTK_TEXT_BUFFER (buffer));

			if (buffer->priv->style_scheme != NULL)
			{
				_ctk_source_engine_set_style_scheme (buffer->priv->highlight_engine,
								     buffer->priv->style_scheme);
			}
		}
	}

	g_object_notify_by_pspec (G_OBJECT (buffer), buffer_properties[PROP_LANGUAGE]);
}

/**
 * ctk_source_buffer_get_language:
 * @buffer: a #CtkSourceBuffer.
 *
 * Returns the #CtkSourceLanguage associated with the buffer,
 * see ctk_source_buffer_set_language().  The returned object should not be
 * unreferenced by the user.
 *
 * Returns: (nullable) (transfer none): the #CtkSourceLanguage associated
 * with the buffer, or %NULL.
 */
CtkSourceLanguage *
ctk_source_buffer_get_language (CtkSourceBuffer *buffer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), NULL);

	return buffer->priv->language;
}

/*
 * _ctk_source_buffer_update_syntax_highlight:
 * @buffer: a #CtkSourceBuffer.
 * @start: start of the area to highlight.
 * @end: end of the area to highlight.
 * @synchronous: whether the area should be highlighted synchronously.
 *
 * Asks the buffer to analyze and highlight given area.
 */
void
_ctk_source_buffer_update_syntax_highlight (CtkSourceBuffer   *buffer,
					    const CtkTextIter *start,
					    const CtkTextIter *end,
					    gboolean           synchronous)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));

	if (buffer->priv->highlight_engine != NULL)
	{
		_ctk_source_engine_update_highlight (buffer->priv->highlight_engine,
						     start,
						     end,
						     synchronous);
	}
}

void
_ctk_source_buffer_update_search_highlight (CtkSourceBuffer   *buffer,
					    const CtkTextIter *start,
					    const CtkTextIter *end,
					    gboolean           synchronous)
{
	GList *l;

	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));

	for (l = buffer->priv->search_contexts; l != NULL; l = l->next)
	{
		CtkSourceSearchContext *search_context = l->data;

		_ctk_source_search_context_update_highlight (search_context,
							     start,
							     end,
							     synchronous);
	}
}

/**
 * ctk_source_buffer_ensure_highlight:
 * @buffer: a #CtkSourceBuffer.
 * @start: start of the area to highlight.
 * @end: end of the area to highlight.
 *
 * Forces buffer to analyze and highlight the given area synchronously.
 *
 * <note>
 *   <para>
 *     This is a potentially slow operation and should be used only
 *     when you need to make sure that some text not currently
 *     visible is highlighted, for instance before printing.
 *   </para>
 * </note>
 **/
void
ctk_source_buffer_ensure_highlight (CtkSourceBuffer   *buffer,
				    const CtkTextIter *start,
				    const CtkTextIter *end)
{
	_ctk_source_buffer_update_syntax_highlight (buffer, start, end, TRUE);
	_ctk_source_buffer_update_search_highlight (buffer, start, end, TRUE);
}

/**
 * ctk_source_buffer_set_style_scheme:
 * @buffer: a #CtkSourceBuffer.
 * @scheme: (nullable): a #CtkSourceStyleScheme or %NULL.
 *
 * Sets a #CtkSourceStyleScheme to be used by the buffer and the view.
 *
 * Note that a #CtkSourceStyleScheme affects not only the syntax highlighting,
 * but also other #CtkSourceView features such as highlighting the current line,
 * matching brackets, the line numbers, etc.
 *
 * Instead of setting a %NULL @scheme, it is better to disable syntax
 * highlighting with ctk_source_buffer_set_highlight_syntax(), and setting the
 * #CtkSourceStyleScheme with the "classic" or "tango" ID, because those two
 * style schemes follow more closely the CTK+ theme (for example for the
 * background color).
 *
 * The buffer holds a reference to @scheme.
 */
void
ctk_source_buffer_set_style_scheme (CtkSourceBuffer      *buffer,
				    CtkSourceStyleScheme *scheme)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));
	g_return_if_fail (CTK_SOURCE_IS_STYLE_SCHEME (scheme) || scheme == NULL);

	if (g_set_object (&buffer->priv->style_scheme, scheme))
	{
		update_bracket_match_style (buffer);

		if (buffer->priv->highlight_engine != NULL)
		{
			_ctk_source_engine_set_style_scheme (buffer->priv->highlight_engine, scheme);
		}

		g_object_notify_by_pspec (G_OBJECT (buffer), buffer_properties[PROP_STYLE_SCHEME]);
	}
}

/**
 * ctk_source_buffer_get_style_scheme:
 * @buffer: a #CtkSourceBuffer.
 *
 * Returns the #CtkSourceStyleScheme associated with the buffer,
 * see ctk_source_buffer_set_style_scheme().
 * The returned object should not be unreferenced by the user.
 *
 * Returns: (nullable) (transfer none): the #CtkSourceStyleScheme
 * associated with the buffer, or %NULL.
 */
CtkSourceStyleScheme *
ctk_source_buffer_get_style_scheme (CtkSourceBuffer *buffer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), NULL);

	return buffer->priv->style_scheme;
}

static void
add_source_mark (CtkSourceBuffer *buffer,
		 CtkSourceMark   *mark)
{
	const gchar *category;
	CtkSourceMarksSequence *seq;

	_ctk_source_marks_sequence_add (buffer->priv->all_source_marks,
					CTK_TEXT_MARK (mark));

	category = ctk_source_mark_get_category (mark);
	seq = g_hash_table_lookup (buffer->priv->source_marks, category);

	if (seq == NULL)
	{
		seq = _ctk_source_marks_sequence_new (CTK_TEXT_BUFFER (buffer));

		g_hash_table_insert (buffer->priv->source_marks,
				     g_strdup (category),
				     seq);
	}

	_ctk_source_marks_sequence_add (seq, CTK_TEXT_MARK (mark));
}

static void
ctk_source_buffer_real_mark_set	(CtkTextBuffer     *buffer,
				 const CtkTextIter *location,
				 CtkTextMark       *mark)
{
	if (CTK_SOURCE_IS_MARK (mark))
	{
		add_source_mark (CTK_SOURCE_BUFFER (buffer),
				 CTK_SOURCE_MARK (mark));

		g_signal_emit (buffer, buffer_signals[SOURCE_MARK_UPDATED], 0, mark);
	}
	else if (mark == ctk_text_buffer_get_insert (buffer))
	{
		cursor_moved (CTK_SOURCE_BUFFER (buffer));
	}

	CTK_TEXT_BUFFER_CLASS (ctk_source_buffer_parent_class)->mark_set (buffer, location, mark);
}

static void
ctk_source_buffer_real_mark_deleted (CtkTextBuffer *buffer,
				     CtkTextMark   *mark)
{
	if (CTK_SOURCE_IS_MARK (mark))
	{
		CtkSourceBuffer *source_buffer = CTK_SOURCE_BUFFER (buffer);
		const gchar *category;
		CtkSourceMarksSequence *seq;

		category = ctk_source_mark_get_category (CTK_SOURCE_MARK (mark));
		seq = g_hash_table_lookup (source_buffer->priv->source_marks, category);

		if (_ctk_source_marks_sequence_is_empty (seq))
		{
			g_hash_table_remove (source_buffer->priv->source_marks, category);
		}

		g_signal_emit (buffer, buffer_signals[SOURCE_MARK_UPDATED], 0, mark);
	}

	if (CTK_TEXT_BUFFER_CLASS (ctk_source_buffer_parent_class)->mark_deleted != NULL)
	{
		CTK_TEXT_BUFFER_CLASS (ctk_source_buffer_parent_class)->mark_deleted (buffer, mark);
	}
}

static void
ctk_source_buffer_real_undo (CtkSourceBuffer *buffer)
{
	g_return_if_fail (ctk_source_undo_manager_can_undo (buffer->priv->undo_manager));

	ctk_source_undo_manager_undo (buffer->priv->undo_manager);
}

static void
ctk_source_buffer_real_redo (CtkSourceBuffer *buffer)
{
	g_return_if_fail (ctk_source_undo_manager_can_redo (buffer->priv->undo_manager));

	ctk_source_undo_manager_redo (buffer->priv->undo_manager);
}

/**
 * ctk_source_buffer_create_source_mark:
 * @buffer: a #CtkSourceBuffer.
 * @name: (nullable): the name of the mark, or %NULL.
 * @category: a string defining the mark category.
 * @where: location to place the mark.
 *
 * Creates a source mark in the @buffer of category @category.  A source mark is
 * a #CtkTextMark but organised into categories. Depending on the category
 * a pixbuf can be specified that will be displayed along the line of the mark.
 *
 * Like a #CtkTextMark, a #CtkSourceMark can be anonymous if the
 * passed @name is %NULL.  Also, the buffer owns the marks so you
 * shouldn't unreference it.
 *
 * Marks always have left gravity and are moved to the beginning of
 * the line when the user deletes the line they were in.
 *
 * Typical uses for a source mark are bookmarks, breakpoints, current
 * executing instruction indication in a source file, etc..
 *
 * Returns: (transfer none): a new #CtkSourceMark, owned by the buffer.
 *
 * Since: 2.2
 */
CtkSourceMark *
ctk_source_buffer_create_source_mark (CtkSourceBuffer   *buffer,
				      const gchar       *name,
				      const gchar       *category,
				      const CtkTextIter *where)
{
	CtkSourceMark *mark;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), NULL);
	g_return_val_if_fail (category != NULL, NULL);
	g_return_val_if_fail (where != NULL, NULL);

	mark = ctk_source_mark_new (name, category);
	ctk_text_buffer_add_mark (CTK_TEXT_BUFFER (buffer),
				  CTK_TEXT_MARK (mark),
				  where);

	/* We want to return a borrowed reference and the mark is already
	 * owned by @buffer due to ctk_text_buffer_add_mark(). Therefore
	 * it is safe to unref immediately.
	 */
	g_object_unref (mark);

	return mark;
}

static CtkSourceMarksSequence *
get_marks_sequence (CtkSourceBuffer *buffer,
		    const gchar     *category)
{
	return category == NULL ?
		buffer->priv->all_source_marks :
		g_hash_table_lookup (buffer->priv->source_marks, category);
}

CtkSourceMark *
_ctk_source_buffer_source_mark_next (CtkSourceBuffer *buffer,
				     CtkSourceMark   *mark,
				     const gchar     *category)
{
	CtkSourceMarksSequence *seq;
	CtkTextMark *next_mark;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), NULL);

	seq = get_marks_sequence (buffer, category);

	if (seq == NULL)
	{
		return NULL;
	}

	next_mark = _ctk_source_marks_sequence_next (seq, CTK_TEXT_MARK (mark));

	return next_mark == NULL ? NULL : CTK_SOURCE_MARK (next_mark);
}

CtkSourceMark *
_ctk_source_buffer_source_mark_prev (CtkSourceBuffer *buffer,
				     CtkSourceMark   *mark,
				     const gchar     *category)
{
	CtkSourceMarksSequence *seq;
	CtkTextMark *prev_mark;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), NULL);

	seq = get_marks_sequence (buffer, category);

	if (seq == NULL)
	{
		return NULL;
	}

	prev_mark = _ctk_source_marks_sequence_prev (seq, CTK_TEXT_MARK (mark));

	return prev_mark == NULL ? NULL : CTK_SOURCE_MARK (prev_mark);
}

/**
 * ctk_source_buffer_forward_iter_to_source_mark:
 * @buffer: a #CtkSourceBuffer.
 * @iter: (inout): an iterator.
 * @category: (nullable): category to search for, or %NULL
 *
 * Moves @iter to the position of the next #CtkSourceMark of the given
 * @category. Returns %TRUE if @iter was moved. If @category is NULL, the
 * next source mark can be of any category.
 *
 * Returns: whether @iter was moved.
 *
 * Since: 2.2
 */
gboolean
ctk_source_buffer_forward_iter_to_source_mark (CtkSourceBuffer *buffer,
					       CtkTextIter     *iter,
					       const gchar     *category)
{
	CtkSourceMarksSequence *seq;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);

	seq = get_marks_sequence (buffer, category);

	if (seq == NULL)
	{
		return FALSE;
	}

	return _ctk_source_marks_sequence_forward_iter (seq, iter);
}

/**
 * ctk_source_buffer_backward_iter_to_source_mark:
 * @buffer: a #CtkSourceBuffer.
 * @iter: (inout): an iterator.
 * @category: (nullable): category to search for, or %NULL
 *
 * Moves @iter to the position of the previous #CtkSourceMark of the given
 * category. Returns %TRUE if @iter was moved. If @category is NULL, the
 * previous source mark can be of any category.
 *
 * Returns: whether @iter was moved.
 *
 * Since: 2.2
 */
gboolean
ctk_source_buffer_backward_iter_to_source_mark (CtkSourceBuffer *buffer,
						CtkTextIter     *iter,
						const gchar     *category)
{
	CtkSourceMarksSequence *seq;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);

	seq = get_marks_sequence (buffer, category);

	if (seq == NULL)
	{
		return FALSE;
	}

	return _ctk_source_marks_sequence_backward_iter (seq, iter);
}

/**
 * ctk_source_buffer_get_source_marks_at_iter:
 * @buffer: a #CtkSourceBuffer.
 * @iter: an iterator.
 * @category: (nullable): category to search for, or %NULL
 *
 * Returns the list of marks of the given category at @iter. If @category
 * is %NULL it returns all marks at @iter.
 *
 * Returns: (element-type CtkSource.Mark) (transfer container):
 * a newly allocated #GSList.
 *
 * Since: 2.2
 */
GSList *
ctk_source_buffer_get_source_marks_at_iter (CtkSourceBuffer *buffer,
					    CtkTextIter     *iter,
					    const gchar     *category)
{
	CtkSourceMarksSequence *seq;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), NULL);
	g_return_val_if_fail (iter != NULL, NULL);

	seq = get_marks_sequence (buffer, category);

	if (seq == NULL)
	{
		return NULL;
	}

	return _ctk_source_marks_sequence_get_marks_at_iter (seq, iter);
}

/**
 * ctk_source_buffer_get_source_marks_at_line:
 * @buffer: a #CtkSourceBuffer.
 * @line: a line number.
 * @category: (nullable): category to search for, or %NULL
 *
 * Returns the list of marks of the given category at @line.
 * If @category is %NULL, all marks at @line are returned.
 *
 * Returns: (element-type CtkSource.Mark) (transfer container):
 * a newly allocated #GSList.
 *
 * Since: 2.2
 */
GSList *
ctk_source_buffer_get_source_marks_at_line (CtkSourceBuffer *buffer,
					    gint             line,
					    const gchar     *category)
{
	CtkSourceMarksSequence *seq;
	CtkTextIter start;
	CtkTextIter end;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), NULL);

	seq = get_marks_sequence (buffer, category);

	if (seq == NULL)
	{
		return NULL;
	}

	ctk_text_buffer_get_iter_at_line (CTK_TEXT_BUFFER (buffer),
					  &start,
					  line);

	end = start;

	if (!ctk_text_iter_ends_line (&end))
	{
		ctk_text_iter_forward_to_line_end (&end);
	}

	return _ctk_source_marks_sequence_get_marks_in_range (seq, &start, &end);
}

/**
 * ctk_source_buffer_remove_source_marks:
 * @buffer: a #CtkSourceBuffer.
 * @start: a #CtkTextIter.
 * @end: a #CtkTextIter.
 * @category: (nullable): category to search for, or %NULL.
 *
 * Remove all marks of @category between @start and @end from the buffer.
 * If @category is NULL, all marks in the range will be removed.
 *
 * Since: 2.2
 */
void
ctk_source_buffer_remove_source_marks (CtkSourceBuffer   *buffer,
				       const CtkTextIter *start,
				       const CtkTextIter *end,
				       const gchar       *category)
{
	CtkSourceMarksSequence *seq;
	GSList *list;
	GSList *l;

 	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));
 	g_return_if_fail (start != NULL);
 	g_return_if_fail (end != NULL);

	seq = get_marks_sequence (buffer, category);

	if (seq == NULL)
	{
		return;
	}

	list = _ctk_source_marks_sequence_get_marks_in_range (seq, start, end);

	for (l = list; l != NULL; l = l->next)
	{
		ctk_text_buffer_delete_mark (CTK_TEXT_BUFFER (buffer), l->data);
	}

	g_slist_free (list);
}

gboolean
_ctk_source_buffer_has_source_marks (CtkSourceBuffer *buffer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), FALSE);

	if (buffer->priv->all_source_marks != NULL)
	{
		return !_ctk_source_marks_sequence_is_empty (buffer->priv->all_source_marks);
	}

	return FALSE;
}

static CtkTextTag *
get_context_class_tag (CtkSourceBuffer *buffer,
		       const gchar     *context_class)
{
	gchar *tag_name;
	CtkTextTagTable *tag_table;
	CtkTextTag *tag;

	tag_name = g_strdup_printf (CONTEXT_CLASSES_PREFIX "%s", context_class);

	tag_table = ctk_text_buffer_get_tag_table (CTK_TEXT_BUFFER (buffer));
	tag = ctk_text_tag_table_lookup (tag_table, tag_name);

	g_free (tag_name);
	return tag;
}

/**
 * ctk_source_buffer_iter_has_context_class:
 * @buffer: a #CtkSourceBuffer.
 * @iter: a #CtkTextIter.
 * @context_class: class to search for.
 *
 * Check if the class @context_class is set on @iter.
 *
 * See the #CtkSourceBuffer description for the list of default context classes.
 *
 * Returns: whether @iter has the context class.
 * Since: 2.10
 */
gboolean
ctk_source_buffer_iter_has_context_class (CtkSourceBuffer   *buffer,
                                          const CtkTextIter *iter,
                                          const gchar       *context_class)
{
	CtkTextTag *tag;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);
	g_return_val_if_fail (context_class != NULL, FALSE);

	tag = get_context_class_tag (buffer, context_class);

	if (tag != NULL)
	{
		return ctk_text_iter_has_tag (iter, tag);
	}

	return FALSE;
}

/**
 * ctk_source_buffer_get_context_classes_at_iter:
 * @buffer: a #CtkSourceBuffer.
 * @iter: a #CtkTextIter.
 *
 * Get all defined context classes at @iter.
 *
 * See the #CtkSourceBuffer description for the list of default context classes.
 *
 * Returns: (array zero-terminated=1) (transfer full): a new %NULL
 * terminated array of context class names.
 * Use g_strfreev() to free the array if it is no longer needed.
 *
 * Since: 2.10
 */
gchar **
ctk_source_buffer_get_context_classes_at_iter (CtkSourceBuffer   *buffer,
                                               const CtkTextIter *iter)
{
	const gsize prefix_len = strlen (CONTEXT_CLASSES_PREFIX);
	GSList *tags;
	GSList *l;
	GPtrArray *ret;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), NULL);
	g_return_val_if_fail (iter != NULL, NULL);

	tags = ctk_text_iter_get_tags (iter);
	ret = g_ptr_array_new ();

	for (l = tags; l != NULL; l = l->next)
	{
		CtkTextTag *tag = l->data;
		gchar *tag_name;

		g_object_get (tag, "name", &tag_name, NULL);

		if (tag_name != NULL &&
		    g_str_has_prefix (tag_name, CONTEXT_CLASSES_PREFIX))
		{
			gchar *context_class_name = g_strdup (tag_name + prefix_len);

			g_ptr_array_add (ret, context_class_name);
		}

		g_free (tag_name);
	}

	g_slist_free (tags);

	g_ptr_array_add (ret, NULL);
	return (gchar **) g_ptr_array_free (ret, FALSE);
}

/**
 * ctk_source_buffer_iter_forward_to_context_class_toggle:
 * @buffer: a #CtkSourceBuffer.
 * @iter: (inout): a #CtkTextIter.
 * @context_class: the context class.
 *
 * Moves forward to the next toggle (on or off) of the context class. If no
 * matching context class toggles are found, returns %FALSE, otherwise %TRUE.
 * Does not return toggles located at @iter, only toggles after @iter. Sets
 * @iter to the location of the toggle, or to the end of the buffer if no
 * toggle is found.
 *
 * See the #CtkSourceBuffer description for the list of default context classes.
 *
 * Returns: whether we found a context class toggle after @iter
 *
 * Since: 2.10
 */
gboolean
ctk_source_buffer_iter_forward_to_context_class_toggle (CtkSourceBuffer *buffer,
                                                        CtkTextIter     *iter,
                                                        const gchar     *context_class)
{
	CtkTextTag *tag;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);
	g_return_val_if_fail (context_class != NULL, FALSE);

	tag = get_context_class_tag (buffer, context_class);

	if (tag != NULL)
	{
		return ctk_text_iter_forward_to_tag_toggle (iter, tag);
	}

	return FALSE;
}

/**
 * ctk_source_buffer_iter_backward_to_context_class_toggle:
 * @buffer: a #CtkSourceBuffer.
 * @iter: (inout): a #CtkTextIter.
 * @context_class: the context class.
 *
 * Moves backward to the next toggle (on or off) of the context class. If no
 * matching context class toggles are found, returns %FALSE, otherwise %TRUE.
 * Does not return toggles located at @iter, only toggles after @iter. Sets
 * @iter to the location of the toggle, or to the end of the buffer if no
 * toggle is found.
 *
 * See the #CtkSourceBuffer description for the list of default context classes.
 *
 * Returns: whether we found a context class toggle before @iter
 *
 * Since: 2.10
 */
gboolean
ctk_source_buffer_iter_backward_to_context_class_toggle (CtkSourceBuffer *buffer,
                                                         CtkTextIter     *iter,
                                                         const gchar     *context_class)
{
	CtkTextTag *tag;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), FALSE);
	g_return_val_if_fail (iter != NULL, FALSE);
	g_return_val_if_fail (context_class != NULL, FALSE);

	tag = get_context_class_tag (buffer, context_class);

	if (tag != NULL)
	{
		return ctk_text_iter_backward_to_tag_toggle (iter, tag);
	}

	return FALSE;
}

/*
 * CtkTextView wastes a lot of time tracking the clipboard content if
 * we do insert/delete operations while there is a selection.
 * These two utilities store the current selection with marks before
 * doing an edit operation and restore it at the end.
 */
void
_ctk_source_buffer_save_and_clear_selection (CtkSourceBuffer *buffer)
{
	CtkTextBuffer *buf;

	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));

	buf = CTK_TEXT_BUFFER (buffer);

	/* Note we cannot use buffer_get_selection_bounds since it
	 * orders the iters while we want to know the position of
	 * each mark.
	 */
	if (ctk_text_buffer_get_has_selection (CTK_TEXT_BUFFER (buffer)))
	{
		CtkTextIter insert_iter;
		CtkTextIter selection_bound_iter;

		g_assert (buffer->priv->tmp_insert_mark == NULL);
		g_assert (buffer->priv->tmp_selection_bound_mark == NULL);

		ctk_text_buffer_get_iter_at_mark (buf, &insert_iter, ctk_text_buffer_get_insert (buf));
		ctk_text_buffer_get_iter_at_mark (buf, &selection_bound_iter, ctk_text_buffer_get_selection_bound (buf));
		buffer->priv->tmp_insert_mark = ctk_text_buffer_create_mark (buf, NULL, &insert_iter, FALSE);
		buffer->priv->tmp_selection_bound_mark = ctk_text_buffer_create_mark (buf, NULL, &selection_bound_iter, FALSE);

		ctk_text_buffer_place_cursor (buf, &insert_iter);
	}
}

void
_ctk_source_buffer_restore_selection (CtkSourceBuffer *buffer)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));

	if (buffer->priv->tmp_insert_mark != NULL &&
	    buffer->priv->tmp_selection_bound_mark != NULL)
	{
		CtkTextBuffer *buf;
		CtkTextIter insert_iter;
		CtkTextIter selection_bound_iter;

		buf = CTK_TEXT_BUFFER (buffer);

		ctk_text_buffer_get_iter_at_mark (buf, &insert_iter, buffer->priv->tmp_insert_mark);
		ctk_text_buffer_get_iter_at_mark (buf, &selection_bound_iter, buffer->priv->tmp_selection_bound_mark);

		ctk_text_buffer_select_range (buf, &insert_iter, &selection_bound_iter);

		ctk_text_buffer_delete_mark (buf, buffer->priv->tmp_insert_mark);
		ctk_text_buffer_delete_mark (buf, buffer->priv->tmp_selection_bound_mark);
		buffer->priv->tmp_insert_mark = NULL;
		buffer->priv->tmp_selection_bound_mark = NULL;
	}
}

static gchar *
do_lower_case (CtkTextBuffer     *buffer,
	       const CtkTextIter *start,
	       const CtkTextIter *end)
{
	gchar *text;
	gchar *new_text;

	text = ctk_text_buffer_get_text (buffer, start, end, TRUE);
	new_text = g_utf8_strdown (text, -1);

	g_free (text);
	return new_text;
}

static gchar *
do_upper_case (CtkTextBuffer     *buffer,
	       const CtkTextIter *start,
	       const CtkTextIter *end)
{
	gchar *text;
	gchar *new_text;

	text = ctk_text_buffer_get_text (buffer, start, end, TRUE);
	new_text = g_utf8_strup (text, -1);

	g_free (text);
	return new_text;
}

static gchar *
do_toggle_case (CtkTextBuffer     *buffer,
		const CtkTextIter *start,
		const CtkTextIter *end)
{
	GString *str;
	CtkTextIter iter_start;

	str = g_string_new (NULL);
	iter_start = *start;

	while (!ctk_text_iter_is_end (&iter_start))
	{
		CtkTextIter iter_end;
		gchar *text;
		gchar *text_down;
		gchar *text_up;

		iter_end = iter_start;
		ctk_text_iter_forward_cursor_position (&iter_end);

		if (ctk_text_iter_compare (end, &iter_end) < 0)
		{
			break;
		}

		text = ctk_text_buffer_get_text (buffer, &iter_start, &iter_end, TRUE);
		text_down = g_utf8_strdown (text, -1);
		text_up = g_utf8_strup (text, -1);

		if (g_strcmp0 (text, text_down) == 0)
		{
			g_string_append (str, text_up);
		}
		else if (g_strcmp0 (text, text_up) == 0)
		{
			g_string_append (str, text_down);
		}
		else
		{
			g_string_append (str, text);
		}

		g_free (text);
		g_free (text_down);
		g_free (text_up);

		iter_start = iter_end;
	}

	return g_string_free (str, FALSE);
}

static gchar *
do_title_case (CtkTextBuffer     *buffer,
	       const CtkTextIter *start,
	       const CtkTextIter *end)
{
	GString *str;
	CtkTextIter iter_start;

	str = g_string_new (NULL);
	iter_start = *start;

	while (!ctk_text_iter_is_end (&iter_start))
	{
		CtkTextIter iter_end;
		gchar *text;

		iter_end = iter_start;
		ctk_text_iter_forward_cursor_position (&iter_end);

		if (ctk_text_iter_compare (end, &iter_end) < 0)
		{
			break;
		}

		text = ctk_text_buffer_get_text (buffer, &iter_start, &iter_end, TRUE);

		if (ctk_text_iter_starts_word (&iter_start))
		{
			gchar *text_normalized;

			text_normalized = g_utf8_normalize (text, -1, G_NORMALIZE_DEFAULT);

			if (g_utf8_strlen (text_normalized, -1) == 1)
			{
				gunichar c;
				gunichar new_c;

				c = ctk_text_iter_get_char (&iter_start);
				new_c = g_unichar_totitle (c);

				g_string_append_unichar (str, new_c);
			}
			else
			{
				gchar *text_up;

				text_up = g_utf8_strup (text, -1);
				g_string_append (str, text_up);

				g_free (text_up);
			}

			g_free (text_normalized);
		}
		else
		{
			gchar *text_down;

			text_down = g_utf8_strdown (text, -1);
			g_string_append (str, text_down);

			g_free (text_down);
		}

		g_free (text);
		iter_start = iter_end;
	}

	return g_string_free (str, FALSE);
}

/**
 * ctk_source_buffer_change_case:
 * @buffer: a #CtkSourceBuffer.
 * @case_type: how to change the case.
 * @start: a #CtkTextIter.
 * @end: a #CtkTextIter.
 *
 * Changes the case of the text between the specified iterators.
 *
 * Since: 3.12
 */
void
ctk_source_buffer_change_case (CtkSourceBuffer         *buffer,
                               CtkSourceChangeCaseType  case_type,
                               CtkTextIter             *start,
                               CtkTextIter             *end)
{
	CtkTextBuffer *text_buffer;
	gchar *new_text;

	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));
	g_return_if_fail (start != NULL);
	g_return_if_fail (end != NULL);

	ctk_text_iter_order (start, end);

	text_buffer = CTK_TEXT_BUFFER (buffer);

	switch (case_type)
	{
		case CTK_SOURCE_CHANGE_CASE_LOWER:
			new_text = do_lower_case (text_buffer, start, end);
			break;

		case CTK_SOURCE_CHANGE_CASE_UPPER:
			new_text = do_upper_case (text_buffer, start, end);
			break;

		case CTK_SOURCE_CHANGE_CASE_TOGGLE:
			new_text = do_toggle_case (text_buffer, start, end);
			break;

		case CTK_SOURCE_CHANGE_CASE_TITLE:
			new_text = do_title_case (text_buffer, start, end);
			break;

		default:
			g_return_if_reached ();
	}

	ctk_text_buffer_begin_user_action (text_buffer);
	ctk_text_buffer_delete (text_buffer, start, end);
	ctk_text_buffer_insert (text_buffer, start, new_text, -1);
	ctk_text_buffer_end_user_action (text_buffer);

	g_free (new_text);
}

/* Move to the end of the line excluding trailing spaces. */
static void
move_to_line_text_end(CtkTextIter *iter)
{
	gint line;

	line = ctk_text_iter_get_line (iter);

	if (!ctk_text_iter_ends_line (iter))
	{
		ctk_text_iter_forward_to_line_end (iter);
	}

	while (ctk_text_iter_backward_char (iter) &&
	       (ctk_text_iter_get_line (iter) == line))
	{
		gunichar ch;

		ch = ctk_text_iter_get_char (iter);
		if (!g_unichar_isspace (ch))
		{
			break;
		}
	}

	ctk_text_iter_forward_char (iter);
}

/**
 * ctk_source_buffer_join_lines:
 * @buffer: a #CtkSourceBuffer.
 * @start: a #CtkTextIter.
 * @end: a #CtkTextIter.
 *
 * Joins the lines of text between the specified iterators.
 *
 * Since: 3.16
 */
void
ctk_source_buffer_join_lines (CtkSourceBuffer *buffer,
                              CtkTextIter     *start,
                              CtkTextIter     *end)
{
	CtkTextBuffer *text_buffer;
	CtkTextMark *end_mark;

	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));
	g_return_if_fail (start != NULL);
	g_return_if_fail (end != NULL);

	ctk_text_iter_order (start, end);

	text_buffer = CTK_TEXT_BUFFER (buffer);
	end_mark = ctk_text_buffer_create_mark (text_buffer, NULL, end, FALSE);

	_ctk_source_buffer_save_and_clear_selection (buffer);
	ctk_text_buffer_begin_user_action (text_buffer);

	move_to_line_text_end (start);
	if (!ctk_text_iter_ends_line (end))
	{
		ctk_text_iter_forward_to_line_end (end);
	}

	while (ctk_text_iter_compare (start, end) < 0)
	{
		CtkTextIter iter;
		gunichar ch;

		iter = *start;

		do
		{
			ch = ctk_text_iter_get_char (&iter);
			if (!g_unichar_isspace (ch))
			{
				break;
			}
		} while (ctk_text_iter_forward_char (&iter) &&
			 ctk_text_iter_compare (&iter, end) < 0);

		if (!ctk_text_iter_is_end (&iter))
		{
			ctk_text_buffer_delete (text_buffer, start, &iter);
			if (!ctk_text_iter_ends_line (start))
			{
				ctk_text_buffer_insert (text_buffer, start, " ", 1);
			}
		}

		move_to_line_text_end (start);

		ctk_text_buffer_get_iter_at_mark (text_buffer, end, end_mark);
	}

	ctk_text_buffer_end_user_action (text_buffer);
	_ctk_source_buffer_restore_selection (buffer);

	ctk_text_buffer_delete_mark (text_buffer, end_mark);
}

static gchar *
get_line_slice (CtkTextBuffer *buf,
		gint           line)
{
	CtkTextIter start, end;

	ctk_text_buffer_get_iter_at_line (buf, &start, line);
	end = start;

	if (!ctk_text_iter_ends_line (&start))
	{
		ctk_text_iter_forward_to_line_end (&end);
	}

	return ctk_text_buffer_get_slice (buf, &start, &end, TRUE);
}

typedef struct {
	gchar *line; /* the original text to re-insert */
	gchar *key;  /* the key to use for the comparison */
} SortLine;

static gint
compare_line (gconstpointer aptr,
              gconstpointer bptr)
{
	const SortLine *a = aptr;
	const SortLine *b = bptr;

	return g_strcmp0 (a->key, b->key);
}

static gint
compare_line_reversed (gconstpointer aptr,
                       gconstpointer bptr)
{
	const SortLine *a = aptr;
	const SortLine *b = bptr;

	return g_strcmp0 (b->key, a->key);
}

/**
 * ctk_source_buffer_sort_lines:
 * @buffer: a #CtkSourceBuffer.
 * @start: a #CtkTextIter.
 * @end: a #CtkTextIter.
 * @flags: #CtkSourceSortFlags specifying how the sort should behave
 * @column: sort considering the text starting at the given column
 *
 * Sort the lines of text between the specified iterators.
 *
 * Since: 3.18
 */
void
ctk_source_buffer_sort_lines (CtkSourceBuffer    *buffer,
                              CtkTextIter        *start,
                              CtkTextIter        *end,
                              CtkSourceSortFlags  flags,
                              gint                column)
{
	CtkTextBuffer *text_buffer;
	gint start_line;
	gint end_line;
	gint num_lines;
	SortLine *lines;
	gchar *last_line = NULL;
	gint i;

	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));
	g_return_if_fail (start != NULL);
	g_return_if_fail (end != NULL);

	text_buffer = CTK_TEXT_BUFFER (buffer);

	ctk_text_iter_order (start, end);

	start_line = ctk_text_iter_get_line (start);
	end_line = ctk_text_iter_get_line (end);

	/* Required for ctk_text_buffer_delete() */
	if (!ctk_text_iter_starts_line (start))
	{
		ctk_text_iter_set_line_offset (start, 0);
	}

	/* if we are at line start our last line is the previus one.
	 * Otherwise the last line is the current one but we try to
	 * move the iter after the line terminator */
	if (ctk_text_iter_starts_line (end))
	{
		end_line = MAX (start_line, end_line - 1);
	}
	else
	{
		ctk_text_iter_forward_line (end);
	}

	if (start_line == end_line)
	{
		return;
	}

	num_lines = end_line - start_line + 1;
	lines = g_new0 (SortLine, num_lines);

	for (i = 0; i < num_lines; i++)
	{
		gchar *line;
		gboolean free_line = FALSE;
		glong length;

		lines[i].line = get_line_slice (text_buffer, start_line + i);

		if ((flags & CTK_SOURCE_SORT_FLAGS_CASE_SENSITIVE) != 0)
		{
			line = lines[i].line;
		}
		else
		{
			line = g_utf8_casefold (lines[i].line, -1);
			free_line = TRUE;
		}

		length = g_utf8_strlen (line, -1);

		if (length < column)
		{
			lines[i].key = NULL;
		}
		else if (column > 0)
		{
			gchar *substring;

			substring = g_utf8_offset_to_pointer (line, column);
			lines[i].key = g_utf8_collate_key (substring, -1);
		}
		else
		{
			lines[i].key = g_utf8_collate_key (line, -1);
		}

		if (free_line)
		{
			g_free (line);
		}
	}

	if ((flags & CTK_SOURCE_SORT_FLAGS_REVERSE_ORDER) != 0)
	{
		qsort (lines, num_lines, sizeof (SortLine), compare_line_reversed);
	}
	else
	{
		qsort (lines, num_lines, sizeof (SortLine), compare_line);
	}

	_ctk_source_buffer_save_and_clear_selection (buffer);
	ctk_text_buffer_begin_user_action (text_buffer);

	ctk_text_buffer_delete (text_buffer, start, end);

	for (i = 0; i < num_lines; i++)
	{
		if ((flags & CTK_SOURCE_SORT_FLAGS_REMOVE_DUPLICATES) != 0 &&
		    g_strcmp0 (last_line, lines[i].line) == 0)
		{
			continue;
		}

		ctk_text_buffer_insert (text_buffer, start, lines[i].line, -1);
		ctk_text_buffer_insert (text_buffer, start, "\n", -1);

		last_line = lines[i].line;
	}

	ctk_text_buffer_end_user_action (text_buffer);
	_ctk_source_buffer_restore_selection (buffer);

	for (i = 0; i < num_lines; i++)
	{
		g_free (lines[i].line);
		g_free (lines[i].key);
	}

	g_free (lines);
}

/**
 * ctk_source_buffer_set_undo_manager:
 * @buffer: a #CtkSourceBuffer.
 * @manager: (nullable): A #CtkSourceUndoManager or %NULL.
 *
 * Set the buffer undo manager. If @manager is %NULL the default undo manager
 * will be set.
 */
void
ctk_source_buffer_set_undo_manager (CtkSourceBuffer      *buffer,
                                    CtkSourceUndoManager *manager)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));
	g_return_if_fail (manager == NULL || CTK_SOURCE_IS_UNDO_MANAGER (manager));

	if (manager == NULL)
	{
		manager = g_object_new (CTK_SOURCE_TYPE_UNDO_MANAGER_DEFAULT,
		                        "buffer", buffer,
		                        "max-undo-levels", buffer->priv->max_undo_levels,
		                        NULL);
	}
	else
	{
		g_object_ref (manager);
	}

	set_undo_manager (buffer, manager);
	g_object_unref (manager);

	g_object_notify_by_pspec (G_OBJECT (buffer), buffer_properties[PROP_UNDO_MANAGER]);
}

/**
 * ctk_source_buffer_get_undo_manager:
 * @buffer: a #CtkSourceBuffer.
 *
 * Returns the #CtkSourceUndoManager associated with the buffer,
 * see ctk_source_buffer_set_undo_manager().  The returned object should not be
 * unreferenced by the user.
 *
 * Returns: (nullable) (transfer none): the #CtkSourceUndoManager associated
 * with the buffer, or %NULL.
 */
CtkSourceUndoManager *
ctk_source_buffer_get_undo_manager (CtkSourceBuffer *buffer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), NULL);

	return buffer->priv->undo_manager;
}

void
_ctk_source_buffer_add_search_context (CtkSourceBuffer        *buffer,
				       CtkSourceSearchContext *search_context)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));
	g_return_if_fail (CTK_SOURCE_IS_SEARCH_CONTEXT (search_context));
	g_return_if_fail (ctk_source_search_context_get_buffer (search_context) == buffer);

	if (g_list_find (buffer->priv->search_contexts, search_context) != NULL)
	{
		return;
	}

	buffer->priv->search_contexts = g_list_prepend (buffer->priv->search_contexts,
							search_context);

	g_object_weak_ref (G_OBJECT (search_context),
			   (GWeakNotify)search_context_weak_notify_cb,
			   buffer);
}

static void
sync_invalid_char_tag (CtkSourceBuffer *buffer,
		       GParamSpec      *pspec,
		       gpointer         data)
{
	CtkSourceStyle *style = NULL;

	if (buffer->priv->style_scheme != NULL)
	{
		style = ctk_source_style_scheme_get_style (buffer->priv->style_scheme, "def:error");
	}

	ctk_source_style_apply (style, buffer->priv->invalid_char_tag);
}

static void
text_tag_set_highest_priority (CtkTextTag    *tag,
			       CtkTextBuffer *buffer)
{
	CtkTextTagTable *table;
	gint n;

	table = ctk_text_buffer_get_tag_table (buffer);
	n = ctk_text_tag_table_get_size (table);
	ctk_text_tag_set_priority (tag, n - 1);
}

void
_ctk_source_buffer_set_as_invalid_character (CtkSourceBuffer   *buffer,
					     const CtkTextIter *start,
					     const CtkTextIter *end)
{
	if (buffer->priv->invalid_char_tag == NULL)
	{
		buffer->priv->invalid_char_tag = ctk_text_buffer_create_tag (CTK_TEXT_BUFFER (buffer),
									     "invalid-char-style",
									     NULL);

		sync_invalid_char_tag (buffer, NULL, NULL);

		g_signal_connect (buffer,
		                  "notify::style-scheme",
		                  G_CALLBACK (sync_invalid_char_tag),
		                  NULL);
	}

	/* Make sure the 'error' tag has the priority over
	 * syntax highlighting tags.
	 */
	text_tag_set_highest_priority (buffer->priv->invalid_char_tag,
	                               CTK_TEXT_BUFFER (buffer));

	ctk_text_buffer_apply_tag (CTK_TEXT_BUFFER (buffer),
	                           buffer->priv->invalid_char_tag,
	                           start,
	                           end);
}

gboolean
_ctk_source_buffer_has_invalid_chars (CtkSourceBuffer *buffer)
{
	CtkTextIter start;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), FALSE);

	if (buffer->priv->invalid_char_tag == NULL)
	{
		return FALSE;
	}

	ctk_text_buffer_get_start_iter (CTK_TEXT_BUFFER (buffer), &start);

	if (ctk_text_iter_starts_tag (&start, buffer->priv->invalid_char_tag) ||
	    ctk_text_iter_forward_to_tag_toggle (&start, buffer->priv->invalid_char_tag))
	{
		return TRUE;
	}

	return FALSE;
}

/**
 * ctk_source_buffer_set_implicit_trailing_newline:
 * @buffer: a #CtkSourceBuffer.
 * @implicit_trailing_newline: the new value.
 *
 * Sets whether the @buffer has an implicit trailing newline.
 *
 * If an explicit trailing newline is present in a #CtkTextBuffer, #CtkTextView
 * shows it as an empty line. This is generally not what the user expects.
 *
 * If @implicit_trailing_newline is %TRUE (the default value):
 *  - when a #CtkSourceFileLoader loads the content of a file into the @buffer,
 *    the trailing newline (if present in the file) is not inserted into the
 *    @buffer.
 *  - when a #CtkSourceFileSaver saves the content of the @buffer into a file, a
 *    trailing newline is added to the file.
 *
 * On the other hand, if @implicit_trailing_newline is %FALSE, the file's
 * content is not modified when loaded into the @buffer, and the @buffer's
 * content is not modified when saved into a file.
 *
 * Since: 3.14
 */
void
ctk_source_buffer_set_implicit_trailing_newline (CtkSourceBuffer *buffer,
						 gboolean         implicit_trailing_newline)
{
	g_return_if_fail (CTK_SOURCE_IS_BUFFER (buffer));

	implicit_trailing_newline = implicit_trailing_newline != FALSE;

	if (buffer->priv->implicit_trailing_newline != implicit_trailing_newline)
	{
		buffer->priv->implicit_trailing_newline = implicit_trailing_newline;
		g_object_notify_by_pspec (G_OBJECT (buffer), buffer_properties[PROP_IMPLICIT_TRAILING_NEWLINE]);
	}
}

/**
 * ctk_source_buffer_get_implicit_trailing_newline:
 * @buffer: a #CtkSourceBuffer.
 *
 * Returns: whether the @buffer has an implicit trailing newline.
 * Since: 3.14
 */
gboolean
ctk_source_buffer_get_implicit_trailing_newline (CtkSourceBuffer *buffer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), TRUE);

	return buffer->priv->implicit_trailing_newline;
}

/**
 * ctk_source_buffer_create_source_tag:
 * @buffer: a #CtkSourceBuffer
 * @tag_name: (nullable): name of the new tag, or %NULL
 * @first_property_name: (nullable): name of first property to set, or %NULL
 * @...: %NULL-terminated list of property names and values
 *
 * In short, this is the same function as ctk_text_buffer_create_tag(), but
 * instead of creating a #CtkTextTag, this function creates a #CtkSourceTag.
 *
 * This function creates a #CtkSourceTag and adds it to the tag table for
 * @buffer.  Equivalent to calling ctk_text_tag_new() and then adding the tag to
 * the buffer’s tag table. The returned tag is owned by the buffer’s tag table,
 * so the ref count will be equal to one.
 *
 * If @tag_name is %NULL, the tag is anonymous.
 *
 * If @tag_name is non-%NULL, a tag called @tag_name must not already
 * exist in the tag table for this buffer.
 *
 * The @first_property_name argument and subsequent arguments are a list
 * of properties to set on the tag, as with g_object_set().
 *
 * Returns: (transfer none): a new #CtkSourceTag.
 * Since: 3.20
 */
CtkTextTag *
ctk_source_buffer_create_source_tag (CtkSourceBuffer *buffer,
				     const gchar     *tag_name,
				     const gchar     *first_property_name,
				     ...)
{
	CtkTextTag *tag;
	CtkTextTagTable *table;
	va_list list;

	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), NULL);

	tag = ctk_source_tag_new (tag_name);

	table = ctk_text_buffer_get_tag_table (CTK_TEXT_BUFFER (buffer));
	if (!ctk_text_tag_table_add (table, tag))
	{
		g_object_unref (tag);
		return NULL;
	}

	if (first_property_name != NULL)
	{
		va_start (list, first_property_name);
		g_object_set_valist (G_OBJECT (tag), first_property_name, list);
		va_end (list);
	}

	g_object_unref (tag);

	return tag;
}

gboolean
_ctk_source_buffer_has_spaces_tag (CtkSourceBuffer *buffer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_BUFFER (buffer), FALSE);

	return buffer->priv->has_draw_spaces_tag;
}
