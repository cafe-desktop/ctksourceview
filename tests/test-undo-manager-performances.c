/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
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

#include <ctksourceview/ctksource.h>

#define NB_LINES 100000

/* Returns the number of undo's. */
static gint
test_undo_redo (CtkSourceBuffer *buffer,
		gint             max_actions)
{
	gint nb_actions;
	gint i;

	for (nb_actions = 0; nb_actions < max_actions; nb_actions++)
	{
		if (ctk_source_buffer_can_undo (buffer))
		{
			ctk_source_buffer_undo (buffer);
		}
		else
		{
			break;
		}
	}

	for (i = 0; i < nb_actions; i++)
	{
		g_assert_true (ctk_source_buffer_can_redo (buffer));
		ctk_source_buffer_redo (buffer);
	}

	return nb_actions;
}

gint
main (gint    argc,
      gchar **argv)
{
	CtkSourceBuffer *source_buffer;
	CtkTextBuffer *text_buffer;
	CtkTextIter iter;
	GTimer *timer;
	gint nb_actions;
	gint i;

	ctk_init (&argc, &argv);

	source_buffer = ctk_source_buffer_new (NULL);
	text_buffer = CTK_TEXT_BUFFER (source_buffer);

	ctk_text_buffer_get_start_iter (text_buffer, &iter);

	for (i = 0; i < NB_LINES; i++)
	{
		ctk_text_buffer_begin_user_action (text_buffer);

		ctk_text_buffer_insert (text_buffer,
					&iter,
					"A line of text to fill the text buffer. Is it long enough?\n",
					-1);

		ctk_text_buffer_end_user_action (text_buffer);
	}

	timer = g_timer_new ();
	nb_actions = test_undo_redo (source_buffer, NB_LINES / 10);
	g_timer_stop (timer);

	g_print ("Undo/Redo %d actions: %lf seconds.\n",
		 nb_actions,
		 g_timer_elapsed (timer, NULL));

	g_timer_start (timer);
	nb_actions = test_undo_redo (source_buffer, NB_LINES);
	g_timer_stop (timer);

	g_print ("Undo/Redo %d actions: %lf seconds.\n",
		 nb_actions,
		 g_timer_elapsed (timer, NULL));

	g_object_unref (source_buffer);
	g_timer_destroy (timer);
	return 0;
}
