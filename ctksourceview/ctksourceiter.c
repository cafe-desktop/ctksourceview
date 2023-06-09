/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2014, 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "ctksourceiter.h"

/* CtkTextIter functions. Contains forward/backward functions for word
 * movements, with custom word boundaries that are used for word selection
 * (double-click) and cursor movements (Ctrl+left, Ctrl+right, etc).  The
 * initial idea was to use those word boundaries directly in CTK+, for all text
 * widgets. But in the end only the CtkTextView::extend-selection signal has
 * been added to be able to customize the boundaries for double- and
 * triple-click (the ::move-cursor and ::delete-from-cursor signals were already
 * present to customize boundaries for cursor movements). The CTK+ developers
 * didn't want to change the word boundaries for text widgets. More information:
 * https://mail.gnome.org/archives/ctk-devel-list/2014-September/msg00019.html
 * https://bugzilla.gnome.org/show_bug.cgi?id=111503
 */

/* TODO: the following Pango bug is now mostly done, see if the code here can be
 * simplified.
 * https://bugzilla.gnome.org/show_bug.cgi?id=97545
 * "Make pango_default_break follow Unicode TR #29"
 */

/* Go to the end of the next or current "full word". A full word is a group of
 * non-blank chars.
 * In other words, this function is the same as the 'E' Vim command.
 *
 * Examples ('|' is the iter position):
 * "|---- abcd"   -> "----| abcd"
 * "|  ---- abcd" -> "  ----| abcd"
 * "--|-- abcd"   -> "----| abcd"
 * "---- a|bcd"   -> "---- abcd|"
 */
void
_ctk_source_iter_forward_full_word_end (CtkTextIter *iter)
{
	CtkTextIter pos;
	gboolean non_blank_found = FALSE;

	/* It would be better to use ctk_text_iter_forward_visible_char(), but
	 * it doesn't exist. So move by cursor position instead, it should be
	 * equivalent here.
	 */

	pos = *iter;

	while (g_unichar_isspace (ctk_text_iter_get_char (&pos)))
	{
		if (!ctk_text_iter_forward_visible_cursor_position (&pos))
		{
			break;
		}
	}

	while (!ctk_text_iter_is_end (&pos) &&
	       !g_unichar_isspace (ctk_text_iter_get_char (&pos)))
	{
		non_blank_found = TRUE;

		if (!ctk_text_iter_forward_visible_cursor_position (&pos))
		{
			break;
		}
	}

	if (non_blank_found)
	{
		*iter = pos;
	}
}

/* Symmetric of iter_forward_full_word_end(). */
void
_ctk_source_iter_backward_full_word_start (CtkTextIter *iter)
{
	CtkTextIter pos;
	CtkTextIter prev;
	gboolean non_blank_found = FALSE;

	pos = *iter;

	while (!ctk_text_iter_is_start (&pos))
	{
		prev = pos;

		if (!ctk_text_iter_backward_visible_cursor_position (&prev))
		{
			break;
		}

		if (!g_unichar_isspace (ctk_text_iter_get_char (&prev)))
		{
			break;
		}

		pos = prev;
	}

	while (!ctk_text_iter_is_start (&pos))
	{
		prev = pos;

		if (!ctk_text_iter_backward_visible_cursor_position (&prev))
		{
			break;
		}

		if (g_unichar_isspace (ctk_text_iter_get_char (&prev)))
		{
			break;
		}

		non_blank_found = TRUE;
		pos = prev;
	}

	if (non_blank_found)
	{
		*iter = pos;
	}
}

gboolean
_ctk_source_iter_starts_full_word (const CtkTextIter *iter)
{
	CtkTextIter prev = *iter;

	if (ctk_text_iter_is_end (iter))
	{
		return FALSE;
	}

	if (!ctk_text_iter_backward_visible_cursor_position (&prev))
	{
		return !g_unichar_isspace (ctk_text_iter_get_char (iter));
	}

	return (g_unichar_isspace (ctk_text_iter_get_char (&prev)) &&
		!g_unichar_isspace (ctk_text_iter_get_char (iter)));
}

gboolean
_ctk_source_iter_ends_full_word (const CtkTextIter *iter)
{
	CtkTextIter prev = *iter;

	if (!ctk_text_iter_backward_visible_cursor_position (&prev))
	{
		return FALSE;
	}

	return (!g_unichar_isspace (ctk_text_iter_get_char (&prev)) &&
		(ctk_text_iter_is_end (iter) ||
		 g_unichar_isspace (ctk_text_iter_get_char (iter))));
}

/* Extends the definition of a natural-language word used by Pango. The
 * underscore is added to the possible characters of a natural-language word.
 */
void
_ctk_source_iter_forward_extra_natural_word_end (CtkTextIter *iter)
{
	CtkTextIter next_word_end = *iter;
	CtkTextIter next_underscore_end = *iter;
	CtkTextIter *limit = NULL;
	gboolean found;

	if (ctk_text_iter_forward_visible_word_end (&next_word_end))
	{
		limit = &next_word_end;
	}

	found = ctk_text_iter_forward_search (iter,
					      "_",
					      CTK_TEXT_SEARCH_VISIBLE_ONLY | CTK_TEXT_SEARCH_TEXT_ONLY,
					      NULL,
					      &next_underscore_end,
					      limit);

	if (found)
	{
		*iter = next_underscore_end;
	}
	else
	{
		*iter = next_word_end;
	}

	while (TRUE)
	{
		if (ctk_text_iter_get_char (iter) == '_')
		{
			if (!ctk_text_iter_forward_visible_cursor_position (iter))
			{
				break;
			}
		}
		else if (ctk_text_iter_starts_word (iter))
		{
			if (!ctk_text_iter_forward_visible_word_end (iter))
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

/* Symmetric of iter_forward_extra_natural_word_end(). */
void
_ctk_source_iter_backward_extra_natural_word_start (CtkTextIter *iter)
{
	CtkTextIter prev_word_start = *iter;
	CtkTextIter prev_underscore_start = *iter;
	CtkTextIter *limit = NULL;
	gboolean found;

	if (ctk_text_iter_backward_visible_word_start (&prev_word_start))
	{
		limit = &prev_word_start;
	}

	found = ctk_text_iter_backward_search (iter,
					       "_",
					       CTK_TEXT_SEARCH_VISIBLE_ONLY | CTK_TEXT_SEARCH_TEXT_ONLY,
					       &prev_underscore_start,
					       NULL,
					       limit);

	if (found)
	{
		*iter = prev_underscore_start;
	}
	else
	{
		*iter = prev_word_start;
	}

	while (!ctk_text_iter_is_start (iter))
	{
		CtkTextIter prev = *iter;

		if (!ctk_text_iter_backward_visible_cursor_position (&prev))
		{
			break;
		}

		if (ctk_text_iter_get_char (&prev) == '_')
		{
			*iter = prev;
		}
		else if (ctk_text_iter_ends_word (iter))
		{
			if (!ctk_text_iter_backward_visible_word_start (iter))
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

static gboolean
backward_cursor_position (CtkTextIter *iter,
			  gboolean     visible)
{
	if (visible)
	{
		return ctk_text_iter_backward_visible_cursor_position (iter);
	}

	return ctk_text_iter_backward_cursor_position (iter);
}

gboolean
_ctk_source_iter_starts_extra_natural_word (const CtkTextIter *iter,
					    gboolean           visible)
{
	gboolean starts_word;
	CtkTextIter prev;

	starts_word = ctk_text_iter_starts_word (iter);

	prev = *iter;
	if (!backward_cursor_position (&prev, visible))
	{
		return starts_word || ctk_text_iter_get_char (iter) == '_';
	}

	if (starts_word)
	{
		return ctk_text_iter_get_char (&prev) != '_';
	}

	return (ctk_text_iter_get_char (iter) == '_' &&
		ctk_text_iter_get_char (&prev) != '_' &&
		!ctk_text_iter_ends_word (iter));
}

gboolean
_ctk_source_iter_ends_extra_natural_word (const CtkTextIter *iter,
					  gboolean           visible)
{
	CtkTextIter prev;
	gboolean ends_word;

	prev = *iter;
	if (!backward_cursor_position (&prev, visible))
	{
		return FALSE;
	}

	ends_word = ctk_text_iter_ends_word (iter);

	if (ctk_text_iter_is_end (iter))
	{
		return ends_word || ctk_text_iter_get_char (&prev) == '_';
	}

	if (ends_word)
	{
		return ctk_text_iter_get_char (iter) != '_';
	}

	return (ctk_text_iter_get_char (&prev) == '_' &&
		ctk_text_iter_get_char (iter) != '_' &&
		!ctk_text_iter_starts_word (iter));
}

/* Similar to ctk_text_iter_forward_visible_word_end, but with a custom
 * definition of "word".
 *
 * It is normally the same word boundaries as in Vim. This function is the same
 * as the 'e' command.
 *
 * With the custom word definition, a word can be:
 * - a natural-language word as defined by Pango, plus the underscore. The
 *   underscore is added because it is often used in programming languages.
 * - a group of contiguous non-blank characters.
 */
gboolean
_ctk_source_iter_forward_visible_word_end (CtkTextIter *iter)
{
	CtkTextIter orig = *iter;
	CtkTextIter farthest = *iter;
	CtkTextIter next_word_end = *iter;
	CtkTextIter word_start;

	/* 'farthest' is the farthest position that this function can return. Example:
	 * "|---- aaaa"  ->  "----| aaaa"
	 */
	_ctk_source_iter_forward_full_word_end (&farthest);

	/* Go to the next extra-natural word end. It can be farther than
	 * 'farthest':
	 * "|---- aaaa"  ->  "---- aaaa|"
	 *
	 * Or it can remain at the same place:
	 * "aaaa| ----"  ->  "aaaa| ----"
	 */
	_ctk_source_iter_forward_extra_natural_word_end (&next_word_end);

	if (ctk_text_iter_compare (&farthest, &next_word_end) < 0 ||
	    ctk_text_iter_equal (iter, &next_word_end))
	{
		*iter = farthest;
		goto end;
	}

	/* From 'next_word_end', go to the previous extra-natural word start.
	 *
	 * Example 1:
	 * iter:          "ab|cd"
	 * next_word_end: "abcd|" -> the good one
	 * word_start:    "|abcd"
	 *
	 * Example 2:
	 * iter:          "| abcd()"
	 * next_word_end: " abcd|()" -> the good one
	 * word_start:    " |abcd()"
	 *
	 * Example 3:
	 * iter:          "abcd|()efgh"
	 * next_word_end: "abcd()efgh|"
	 * word_start:    "abcd()|efgh" -> the good one, at the end of the word "()".
	 */
	word_start = next_word_end;
	_ctk_source_iter_backward_extra_natural_word_start (&word_start);

	/* Example 1 */
	if (ctk_text_iter_compare (&word_start, iter) <= 0)
	{
		*iter = next_word_end;
	}

	/* Example 2 */
	else if (_ctk_source_iter_starts_full_word (&word_start))
	{
		*iter = next_word_end;
	}

	/* Example 3 */
	else
	{
		*iter = word_start;
	}

end:
	return !ctk_text_iter_equal (&orig, iter) && !ctk_text_iter_is_end (iter);
}

/* Symmetric of _ctk_source_iter_forward_visible_word_end(). */
gboolean
_ctk_source_iter_backward_visible_word_start (CtkTextIter *iter)
{
	CtkTextIter orig = *iter;
	CtkTextIter farthest = *iter;
	CtkTextIter prev_word_start = *iter;
	CtkTextIter word_end;

	/* 'farthest' is the farthest position that this function can return. Example:
	 * "aaaa ----|"  ->  "aaaa |----"
	 */
	_ctk_source_iter_backward_full_word_start (&farthest);

	/* Go to the previous extra-natural word start. It can be farther than
	 * 'farthest':
	 * "aaaa ----|"  ->  "|aaaa ----"
	 *
	 * Or it can remain at the same place:
	 * "---- |aaaa"  ->  "---- |aaaa"
	 */
	_ctk_source_iter_backward_extra_natural_word_start (&prev_word_start);

	if (ctk_text_iter_compare (&prev_word_start, &farthest) < 0 ||
	    ctk_text_iter_equal (iter, &prev_word_start))
	{
		*iter = farthest;
		goto end;
	}

	/* From 'prev_word_start', go to the next extra-natural word end.
	 *
	 * Example 1:
	 * iter:            "ab|cd"
	 * prev_word_start: "|abcd" -> the good one
	 * word_end:        "abcd|"
	 *
	 * Example 2:
	 * iter:            "()abcd |"
	 * prev_word_start: "()|abcd " -> the good one
	 * word_end:        "()abcd| "
	 *
	 * Example 3:
	 * iter:            "abcd()|"
	 * prev_word_start: "|abcd()"
	 * word_end:        "abcd|()" -> the good one, at the start of the word "()".
	 */
	word_end = prev_word_start;
	_ctk_source_iter_forward_extra_natural_word_end (&word_end);

	/* Example 1 */
	if (ctk_text_iter_compare (iter, &word_end) <= 0)
	{
		*iter = prev_word_start;
	}

	/* Example 2 */
	else if (_ctk_source_iter_ends_full_word (&word_end))
	{
		*iter = prev_word_start;
	}

	/* Example 3 */
	else
	{
		*iter = word_end;
	}

end:
	return !ctk_text_iter_equal (&orig, iter) && !ctk_text_iter_is_end (iter);
}

/* Similar to ctk_text_iter_forward_visible_word_ends(). */
gboolean
_ctk_source_iter_forward_visible_word_ends (CtkTextIter *iter,
					    gint         count)
{
	CtkTextIter orig = *iter;
	gint i;

	if (count < 0)
	{
		return _ctk_source_iter_backward_visible_word_starts (iter, -count);
	}

	for (i = 0; i < count; i++)
	{
		if (!_ctk_source_iter_forward_visible_word_end (iter))
		{
			break;
		}
	}

	return !ctk_text_iter_equal (&orig, iter) && !ctk_text_iter_is_end (iter);
}

/* Similar to ctk_text_iter_backward_visible_word_starts(). */
gboolean
_ctk_source_iter_backward_visible_word_starts (CtkTextIter *iter,
					       gint         count)
{
	CtkTextIter orig = *iter;
	gint i;

	if (count < 0)
	{
		return _ctk_source_iter_forward_visible_word_ends (iter, -count);
	}

	for (i = 0; i < count; i++)
	{
		if (!_ctk_source_iter_backward_visible_word_start (iter))
		{
			break;
		}
	}

	return !ctk_text_iter_equal (&orig, iter) && !ctk_text_iter_is_end (iter);
}

gboolean
_ctk_source_iter_starts_word (const CtkTextIter *iter)
{
	if (_ctk_source_iter_starts_full_word (iter) ||
	    _ctk_source_iter_starts_extra_natural_word (iter, TRUE))
	{
		return TRUE;
	}

	/* Example: "abcd|()", at the start of the word "()". */
	return (!_ctk_source_iter_ends_full_word (iter) &&
		_ctk_source_iter_ends_extra_natural_word (iter, TRUE));
}

gboolean
_ctk_source_iter_ends_word (const CtkTextIter *iter)
{
	if (_ctk_source_iter_ends_full_word (iter) ||
	    _ctk_source_iter_ends_extra_natural_word (iter, TRUE))
	{
		return TRUE;
	}

	/* Example: "abcd()|efgh", at the end of the word "()". */
	return (!_ctk_source_iter_starts_full_word (iter) &&
		_ctk_source_iter_starts_extra_natural_word (iter, TRUE));
}

gboolean
_ctk_source_iter_inside_word (const CtkTextIter *iter)
{
	CtkTextIter prev_word_start;
	CtkTextIter word_end;

	if (_ctk_source_iter_starts_word (iter))
	{
		return TRUE;
	}

	prev_word_start = *iter;
	if (!_ctk_source_iter_backward_visible_word_start (&prev_word_start))
	{
		return FALSE;
	}

	word_end = prev_word_start;
	_ctk_source_iter_forward_visible_word_end (&word_end);

	return (ctk_text_iter_compare (&prev_word_start, iter) <= 0 &&
		ctk_text_iter_compare (iter, &word_end) < 0);
}

/* Used for the CtkTextView::extend-selection signal. */
void
_ctk_source_iter_extend_selection_word (const CtkTextIter *location,
					CtkTextIter       *start,
					CtkTextIter       *end)
{
	/* Exactly the same algorithm as in CTK+, but with our custom word
	 * boundaries.
	 */
	*start = *location;
	*end = *location;

	if (_ctk_source_iter_inside_word (start))
	{
		if (!_ctk_source_iter_starts_word (start))
		{
			_ctk_source_iter_backward_visible_word_start (start);
		}

		if (!_ctk_source_iter_ends_word (end))
		{
			_ctk_source_iter_forward_visible_word_end (end);
		}
	}
	else
	{
		CtkTextIter tmp;

		tmp = *start;
		if (_ctk_source_iter_backward_visible_word_start (&tmp))
		{
			_ctk_source_iter_forward_visible_word_end (&tmp);
		}

		if (ctk_text_iter_get_line (&tmp) == ctk_text_iter_get_line (start))
		{
			*start = tmp;
		}
		else
		{
			ctk_text_iter_set_line_offset (start, 0);
		}

		tmp = *end;
		if (!_ctk_source_iter_forward_visible_word_end (&tmp))
		{
			ctk_text_iter_forward_to_end (&tmp);
		}

		if (_ctk_source_iter_ends_word (&tmp))
		{
			_ctk_source_iter_backward_visible_word_start (&tmp);
		}

		if (ctk_text_iter_get_line (&tmp) == ctk_text_iter_get_line (end))
		{
			*end = tmp;
		}
		else
		{
			ctk_text_iter_forward_to_line_end (end);
		}
	}
}

/* Get the boundary, on @iter's line, between leading spaces (indentation) and
 * the text.
 */
void
_ctk_source_iter_get_leading_spaces_end_boundary (const CtkTextIter *iter,
						  CtkTextIter       *leading_end)
{
	g_return_if_fail (iter != NULL);
	g_return_if_fail (leading_end != NULL);

	*leading_end = *iter;
	ctk_text_iter_set_line_offset (leading_end, 0);

	while (!ctk_text_iter_ends_line (leading_end))
	{
		gunichar ch = ctk_text_iter_get_char (leading_end);

		if (!g_unichar_isspace (ch))
		{
			break;
		}

		ctk_text_iter_forward_char (leading_end);
	}
}

/* Get the boundary, on @iter's line, between the end of the text and trailing
 * spaces.
 */
void
_ctk_source_iter_get_trailing_spaces_start_boundary (const CtkTextIter *iter,
						     CtkTextIter       *trailing_start)
{
	g_return_if_fail (iter != NULL);
	g_return_if_fail (trailing_start != NULL);

	*trailing_start = *iter;
	if (!ctk_text_iter_ends_line (trailing_start))
	{
		ctk_text_iter_forward_to_line_end (trailing_start);
	}

	while (!ctk_text_iter_starts_line (trailing_start))
	{
		CtkTextIter prev;
		gunichar ch;

		prev = *trailing_start;
		ctk_text_iter_backward_char (&prev);

		ch = ctk_text_iter_get_char (&prev);
		if (!g_unichar_isspace (ch))
		{
			break;
		}

		*trailing_start = prev;
	}
}
