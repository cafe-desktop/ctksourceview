/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*-
 *
 * This file is part of CtkSourceView
 *
 * Copyright (C) 2009 - Jesse van den Kieboom
 * Copyright (C) 2013 - Sébastien Wilmet
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ctksourcecompletionwordsbuffer.h"
#include "ctksourcecompletionwordsutils.h"
#include "ctksourceview/ctksourceregion.h"

/* Timeout in seconds */
#define INITIATE_SCAN_TIMEOUT 5

/* Timeout in milliseconds */
#define BATCH_SCAN_TIMEOUT 10

typedef struct
{
	CtkSourceCompletionWordsProposal *proposal;
	guint use_count;
} ProposalCache;

struct _CtkSourceCompletionWordsBufferPrivate
{
	CtkSourceCompletionWordsLibrary *library;
	CtkTextBuffer *buffer;

	CtkSourceRegion *scan_region;
	gulong batch_scan_id;
	gulong initiate_scan_id;

	guint scan_batch_size;
	guint minimum_word_size;

	GHashTable *words;
};

G_DEFINE_TYPE_WITH_PRIVATE (CtkSourceCompletionWordsBuffer, ctk_source_completion_words_buffer, G_TYPE_OBJECT)

static ProposalCache *
proposal_cache_new (CtkSourceCompletionWordsProposal *proposal)
{
	ProposalCache *cache = g_slice_new (ProposalCache);
	cache->proposal = g_object_ref (proposal);
	cache->use_count = 1;

	return cache;
}

static void
proposal_cache_free (ProposalCache *cache)
{
	g_object_unref (cache->proposal);
	g_slice_free (ProposalCache, cache);
}

static void
remove_proposal_cache (const gchar                    *key,
		       ProposalCache                  *cache,
		       CtkSourceCompletionWordsBuffer *buffer)
{
	guint i;

	for (i = 0; i < cache->use_count; ++i)
	{
		ctk_source_completion_words_library_remove_word (buffer->priv->library,
		                                                 cache->proposal);
	}
}

static void
remove_all_words (CtkSourceCompletionWordsBuffer *buffer)
{
	g_hash_table_foreach (buffer->priv->words,
	                      (GHFunc)remove_proposal_cache,
	                      buffer);

	g_hash_table_remove_all (buffer->priv->words);
}

static void
ctk_source_completion_words_buffer_dispose (GObject *object)
{
	CtkSourceCompletionWordsBuffer *buffer =
		CTK_SOURCE_COMPLETION_WORDS_BUFFER (object);

	if (buffer->priv->words != NULL)
	{
		remove_all_words (buffer);

		g_hash_table_destroy (buffer->priv->words);
		buffer->priv->words = NULL;
	}

	if (buffer->priv->batch_scan_id != 0)
	{
		g_source_remove (buffer->priv->batch_scan_id);
		buffer->priv->batch_scan_id = 0;
	}

	if (buffer->priv->initiate_scan_id != 0)
	{
		g_source_remove (buffer->priv->initiate_scan_id);
		buffer->priv->initiate_scan_id = 0;
	}

	g_clear_object (&buffer->priv->scan_region);
	g_clear_object (&buffer->priv->buffer);
	g_clear_object (&buffer->priv->library);

	G_OBJECT_CLASS (ctk_source_completion_words_buffer_parent_class)->dispose (object);
}

static void
ctk_source_completion_words_buffer_class_init (CtkSourceCompletionWordsBufferClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = ctk_source_completion_words_buffer_dispose;
}

static void
ctk_source_completion_words_buffer_init (CtkSourceCompletionWordsBuffer *self)
{
	self->priv = ctk_source_completion_words_buffer_get_instance_private (self);

	self->priv->scan_batch_size = 20;
	self->priv->minimum_word_size = 3;

	self->priv->words = g_hash_table_new_full (g_str_hash,
	                                           g_str_equal,
	                                           (GDestroyNotify)g_free,
	                                           (GDestroyNotify)proposal_cache_free);
}

/* Starts the scanning at @start, and ends at the line end or @end. So at most
 * one line is scanned, and the text after @end will never be scanned.
 */
static GSList *
scan_line (CtkSourceCompletionWordsBuffer *buffer,
	   const CtkTextIter              *start,
	   const CtkTextIter              *end)
{
	CtkTextIter line_end;
	CtkTextIter text_end;
	gchar *text;
	GSList *words;

	if (ctk_text_iter_compare (end, start) <= 0 ||
	    ctk_text_iter_ends_line (start))
	{
		return NULL;
	}

	line_end = *start;
	ctk_text_iter_forward_to_line_end (&line_end);

	if (ctk_text_iter_compare (end, &line_end) < 0)
	{
		text_end = *end;
	}
	else
	{
		text_end = line_end;
	}

	_ctk_source_completion_words_utils_check_scan_region (start, &text_end);

	text = ctk_text_buffer_get_text (buffer->priv->buffer,
					 start,
					 &text_end,
					 FALSE);

	words = _ctk_source_completion_words_utils_scan_words (text, buffer->priv->minimum_word_size);

	g_free (text);
	return words;
}

static void
remove_word (CtkSourceCompletionWordsBuffer *buffer,
	     const gchar                    *word)
{
	ProposalCache *cache = g_hash_table_lookup (buffer->priv->words, word);

	if (cache == NULL)
	{
		g_warning ("Could not find word to remove in buffer (%s), this should not happen!",
		           word);
		return;
	}

	ctk_source_completion_words_library_remove_word (buffer->priv->library,
	                                                 cache->proposal);

	--cache->use_count;

	if (cache->use_count == 0)
	{
		g_hash_table_remove (buffer->priv->words, word);
	}
}

static void
add_words (CtkSourceCompletionWordsBuffer *buffer,
	   GSList                         *words)
{
	GSList *item;

	for (item = words; item != NULL; item = g_slist_next (item))
	{
		CtkSourceCompletionWordsProposal *proposal;
		ProposalCache *cache;

		proposal = ctk_source_completion_words_library_add_word (buffer->priv->library,
		                                                         item->data);

		cache = g_hash_table_lookup (buffer->priv->words,
		                             item->data);

		if (cache != NULL)
		{
			++cache->use_count;
			g_free (item->data);
		}
		else
		{
			/* Hash table takes over ownership of the word string */
			cache = proposal_cache_new (proposal);
			g_hash_table_insert (buffer->priv->words,
			                     item->data,
			                     cache);
		}
	}

	g_slist_free (words);
}

/* Scan words in the region delimited by @start and @end.
 * Maximum @max_lines are scanned.
 * Returns the number of lines scanned.
 * @stop is set to the iter where the scanning has stopped.
 */
static guint
scan_region (CtkSourceCompletionWordsBuffer *buffer,
	     const CtkTextIter              *start,
	     const CtkTextIter              *end,
	     guint                           max_lines,
	     CtkTextIter                    *stop)
{
	CtkTextIter iter = *start;
	guint nb_lines_scanned = 0;

	g_assert (max_lines != 0);

	while (TRUE)
	{
		GSList *words;

		if (ctk_text_iter_compare (end, &iter) < 0)
		{
			*stop = *end;
			break;
		}

		if (nb_lines_scanned >= max_lines)
		{
			*stop = iter;
			break;
		}

		words = scan_line (buffer, &iter, end);

		/* add_words also frees the list */
		add_words (buffer, words);

		nb_lines_scanned++;
		ctk_text_iter_forward_line (&iter);
	}

	return nb_lines_scanned;
}

static gboolean
idle_scan_regions (CtkSourceCompletionWordsBuffer *buffer)
{
	guint nb_remaining_lines = buffer->priv->scan_batch_size;
	CtkSourceRegionIter region_iter;
	CtkTextIter start;
	CtkTextIter stop;

	ctk_text_buffer_get_start_iter (buffer->priv->buffer, &start);
	stop = start;

	ctk_source_region_get_start_region_iter (buffer->priv->scan_region, &region_iter);

	while (nb_remaining_lines > 0 &&
	       !ctk_source_region_iter_is_end (&region_iter))
	{
		CtkTextIter region_start;
		CtkTextIter region_end;

		ctk_source_region_iter_get_subregion (&region_iter,
						      &region_start,
						      &region_end);

		nb_remaining_lines -= scan_region (buffer,
						   &region_start,
						   &region_end,
						   nb_remaining_lines,
						   &stop);

		ctk_source_region_iter_next (&region_iter);
	}

	ctk_source_region_subtract_subregion (buffer->priv->scan_region,
					      &start,
					      &stop);

	if (ctk_source_region_is_empty (buffer->priv->scan_region))
	{
		buffer->priv->batch_scan_id = 0;
		return G_SOURCE_REMOVE;
	}

	return G_SOURCE_CONTINUE;
}

static gboolean
initiate_scan (CtkSourceCompletionWordsBuffer *buffer)
{
	buffer->priv->initiate_scan_id = 0;

	/* Add the batch scanner */
	buffer->priv->batch_scan_id =
		g_timeout_add_full (G_PRIORITY_LOW,
		                    BATCH_SCAN_TIMEOUT,
		                    (GSourceFunc)idle_scan_regions,
		                    buffer,
		                    NULL);

	return G_SOURCE_REMOVE;
}

static void
install_initiate_scan (CtkSourceCompletionWordsBuffer *buffer)
{
	if (buffer->priv->batch_scan_id == 0 &&
	    buffer->priv->initiate_scan_id == 0)
	{
		buffer->priv->initiate_scan_id =
			g_timeout_add_seconds_full (G_PRIORITY_LOW,
			                            INITIATE_SCAN_TIMEOUT,
			                            (GSourceFunc)initiate_scan,
			                            buffer,
			                            NULL);
	}
}

static void
remove_words_in_subregion (CtkSourceCompletionWordsBuffer *buffer,
			   const CtkTextIter              *start,
			   const CtkTextIter              *end)
{
	CtkTextIter iter = *start;

	while (ctk_text_iter_compare (&iter, end) < 0)
	{
		GSList *words = scan_line (buffer, &iter, end);
		GSList *item;

		for (item = words; item != NULL; item = g_slist_next (item))
		{
			remove_word (buffer, item->data);
			g_free (item->data);
		}

		g_slist_free (words);
		ctk_text_iter_forward_line (&iter);
	}
}

static void
remove_words_in_region (CtkSourceCompletionWordsBuffer *buffer,
			CtkSourceRegion                *region)
{
	CtkSourceRegionIter region_iter;

	ctk_source_region_get_start_region_iter (region, &region_iter);

	while (!ctk_source_region_iter_is_end (&region_iter))
	{
		CtkTextIter subregion_start;
		CtkTextIter subregion_end;

		ctk_source_region_iter_get_subregion (&region_iter,
						      &subregion_start,
						      &subregion_end);

		remove_words_in_subregion (buffer,
					   &subregion_start,
					   &subregion_end);

		ctk_source_region_iter_next (&region_iter);
	}
}

static CtkSourceRegion *
compute_remove_region (CtkSourceCompletionWordsBuffer *buffer,
		       const CtkTextIter              *start,
		       const CtkTextIter              *end)
{
	CtkSourceRegion *remove_region = ctk_source_region_new (buffer->priv->buffer);
	CtkSourceRegionIter region_iter;

	ctk_source_region_add_subregion (remove_region, start, end);

	ctk_source_region_get_start_region_iter (buffer->priv->scan_region, &region_iter);

	while (!ctk_source_region_iter_is_end (&region_iter))
	{
		CtkTextIter scan_start;
		CtkTextIter scan_end;

		ctk_source_region_iter_get_subregion (&region_iter,
						      &scan_start,
						      &scan_end);

		ctk_source_region_subtract_subregion (remove_region,
						      &scan_start,
						      &scan_end);

		ctk_source_region_iter_next (&region_iter);
	}

	return remove_region;
}

/* Remove the words between @start and @end that are not in the scan region.
 * @start and @end are adjusted to word boundaries, if they touch or are inside
 * a word.
 */
static void
invalidate_region (CtkSourceCompletionWordsBuffer *buffer,
		   const CtkTextIter              *start,
		   const CtkTextIter              *end)
{
	CtkTextIter start_iter = *start;
	CtkTextIter end_iter = *end;
	CtkSourceRegion *remove_region;

	_ctk_source_completion_words_utils_adjust_region (&start_iter, &end_iter);

	remove_region = compute_remove_region (buffer, &start_iter, &end_iter);

	remove_words_in_region (buffer, remove_region);
	g_clear_object (&remove_region);
}

static void
add_to_scan_region (CtkSourceCompletionWordsBuffer *buffer,
		    const CtkTextIter              *start,
		    const CtkTextIter              *end)
{
	CtkTextIter start_iter = *start;
	CtkTextIter end_iter = *end;

	_ctk_source_completion_words_utils_adjust_region (&start_iter, &end_iter);

	ctk_source_region_add_subregion (buffer->priv->scan_region,
					 &start_iter,
					 &end_iter);

	install_initiate_scan (buffer);
}

static void
on_insert_text_before_cb (CtkTextBuffer                  *textbuffer,
			  CtkTextIter                    *location,
			  const gchar                    *text,
			  gint                            len,
			  CtkSourceCompletionWordsBuffer *buffer)
{
	invalidate_region (buffer, location, location);
}

static void
on_insert_text_after_cb (CtkTextBuffer                  *textbuffer,
			 CtkTextIter                    *location,
			 const gchar                    *text,
			 gint                            len,
			 CtkSourceCompletionWordsBuffer *buffer)
{
	CtkTextIter start_iter = *location;
	gint nb_chars = g_utf8_strlen (text, -1);

	ctk_text_iter_backward_chars (&start_iter, nb_chars);

	/* If add_to_scan_region() is called before the text insertion, the
	 * created CtkSourceRegion can be empty and is thus not added to the
	 * scan region. After the text insertion, we are sure that the
	 * CtkSourceRegion is not empty and that the words will be scanned.
	 */
	add_to_scan_region (buffer, &start_iter, location);
}

static void
on_delete_range_before_cb (CtkTextBuffer                  *text_buffer,
			   CtkTextIter                    *start,
			   CtkTextIter                    *end,
			   CtkSourceCompletionWordsBuffer *buffer)
{
	CtkTextIter start_buf;
	CtkTextIter end_buf;

	ctk_text_buffer_get_bounds (text_buffer, &start_buf, &end_buf);

	/* Special case removing all the text */
	if (ctk_text_iter_equal (start, &start_buf) &&
	    ctk_text_iter_equal (end, &end_buf))
	{
		remove_all_words (buffer);

		g_clear_object (&buffer->priv->scan_region);
		buffer->priv->scan_region = ctk_source_region_new (text_buffer);
	}
	else
	{
		invalidate_region (buffer, start, end);
	}
}

static void
on_delete_range_after_cb (CtkTextBuffer                  *text_buffer,
			  CtkTextIter                    *start,
			  CtkTextIter                    *end,
			  CtkSourceCompletionWordsBuffer *buffer)
{
	/* start = end, but add_to_scan_region() adjusts the iters to word
	 * boundaries if needed. If the adjusted [start,end] region is empty, it
	 * won't be added to the scan region. If we call add_to_scan_region()
	 * when the text is not already deleted, the CtkSourceRegion is not empty
	 * and will be added to the scan region, and when the CtkSourceRegion
	 * becomes empty after the text deletion, the CtkSourceRegion is not
	 * removed from the scan region. Hence two callbacks: before and after
	 * the text deletion.
	 */
	add_to_scan_region (buffer, start, end);
}

static void
scan_all_buffer (CtkSourceCompletionWordsBuffer *buffer)
{
	CtkTextIter start;
	CtkTextIter end;

	ctk_text_buffer_get_bounds (buffer->priv->buffer,
	                            &start,
	                            &end);

	ctk_source_region_add_subregion (buffer->priv->scan_region,
					 &start,
					 &end);

	install_initiate_scan (buffer);
}

static void
connect_buffer (CtkSourceCompletionWordsBuffer *buffer)
{
	g_signal_connect_object (buffer->priv->buffer,
				 "insert-text",
				 G_CALLBACK (on_insert_text_before_cb),
				 buffer,
				 0);

	g_signal_connect_object (buffer->priv->buffer,
				 "insert-text",
				 G_CALLBACK (on_insert_text_after_cb),
				 buffer,
				 G_CONNECT_AFTER);

	g_signal_connect_object (buffer->priv->buffer,
				 "delete-range",
				 G_CALLBACK (on_delete_range_before_cb),
				 buffer,
				 0);

	g_signal_connect_object (buffer->priv->buffer,
				 "delete-range",
				 G_CALLBACK (on_delete_range_after_cb),
				 buffer,
				 G_CONNECT_AFTER);

	scan_all_buffer (buffer);
}

static void
on_library_lock (CtkSourceCompletionWordsBuffer *buffer)
{
	if (buffer->priv->batch_scan_id != 0)
	{
		g_source_remove (buffer->priv->batch_scan_id);
		buffer->priv->batch_scan_id = 0;
	}

	if (buffer->priv->initiate_scan_id != 0)
	{
		g_source_remove (buffer->priv->initiate_scan_id);
		buffer->priv->initiate_scan_id = 0;
	}
}

static void
on_library_unlock (CtkSourceCompletionWordsBuffer *buffer)
{
	if (!ctk_source_region_is_empty (buffer->priv->scan_region))
	{
		install_initiate_scan (buffer);
	}
}

CtkSourceCompletionWordsBuffer *
ctk_source_completion_words_buffer_new (CtkSourceCompletionWordsLibrary *library,
					CtkTextBuffer                   *buffer)
{
	CtkSourceCompletionWordsBuffer *ret;

	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_LIBRARY (library), NULL);
	g_return_val_if_fail (CTK_IS_TEXT_BUFFER (buffer), NULL);

	ret = g_object_new (CTK_SOURCE_TYPE_COMPLETION_WORDS_BUFFER, NULL);

	ret->priv->library = g_object_ref (library);
	ret->priv->buffer = g_object_ref (buffer);

	ret->priv->scan_region = ctk_source_region_new (buffer);

	g_signal_connect_object (ret->priv->library,
				 "lock",
				 G_CALLBACK (on_library_lock),
				 ret,
				 G_CONNECT_SWAPPED);

	g_signal_connect_object (ret->priv->library,
				 "unlock",
				 G_CALLBACK (on_library_unlock),
				 ret,
				 G_CONNECT_SWAPPED);

	connect_buffer (ret);

	return ret;
}

CtkTextBuffer *
ctk_source_completion_words_buffer_get_buffer (CtkSourceCompletionWordsBuffer *buffer)
{
	g_return_val_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_BUFFER (buffer), NULL);

	return buffer->priv->buffer;
}

void
ctk_source_completion_words_buffer_set_scan_batch_size (CtkSourceCompletionWordsBuffer *buffer,
							guint                           size)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_BUFFER (buffer));
	g_return_if_fail (size != 0);

	buffer->priv->scan_batch_size = size;
}

void
ctk_source_completion_words_buffer_set_minimum_word_size (CtkSourceCompletionWordsBuffer *buffer,
							  guint                           size)
{
	g_return_if_fail (CTK_SOURCE_IS_COMPLETION_WORDS_BUFFER (buffer));
	g_return_if_fail (size != 0);

	if (buffer->priv->minimum_word_size != size)
	{
		buffer->priv->minimum_word_size = size;
		remove_all_words (buffer);
		scan_all_buffer (buffer);
	}
}
