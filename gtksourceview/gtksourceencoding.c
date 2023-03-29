/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8; coding: utf-8 -*- */
/*
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2002-2005 - Paolo Maggi
 * Copyright (C) 2014, 2015 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * GtkSourceView is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * GtkSourceView is distributed in the hope that it will be useful,
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

#include "ctksourceencoding.h"
#include "ctksourceencoding-private.h"
#include <glib/gi18n-lib.h>

/**
 * SECTION:encoding
 * @Short_description: Character encoding
 * @Title: GtkSourceEncoding
 * @See_also: #GtkSourceFileSaver, #GtkSourceFileLoader
 *
 * The #GtkSourceEncoding boxed type represents a character encoding. It is used
 * for example by #GtkSourceFile. Note that the text in CTK+ widgets is always
 * encoded in UTF-8.
 */

struct _GtkSourceEncoding
{
	gint index;
	const gchar *charset;
	const gchar *name;
};

G_DEFINE_BOXED_TYPE (GtkSourceEncoding, ctk_source_encoding,
                     ctk_source_encoding_copy,
                     ctk_source_encoding_free)

/*
 * The original versions of the following tables are taken from profterm
 *
 * Copyright (C) 2002 Red Hat, Inc.
 */

typedef enum _GtkSourceEncodingIndex
{
	CTK_SOURCE_ENCODING_ISO_8859_1,
	CTK_SOURCE_ENCODING_ISO_8859_2,
	CTK_SOURCE_ENCODING_ISO_8859_3,
	CTK_SOURCE_ENCODING_ISO_8859_4,
	CTK_SOURCE_ENCODING_ISO_8859_5,
	CTK_SOURCE_ENCODING_ISO_8859_6,
	CTK_SOURCE_ENCODING_ISO_8859_7,
	CTK_SOURCE_ENCODING_ISO_8859_8,
	CTK_SOURCE_ENCODING_ISO_8859_9,
	CTK_SOURCE_ENCODING_ISO_8859_10,
	CTK_SOURCE_ENCODING_ISO_8859_13,
	CTK_SOURCE_ENCODING_ISO_8859_14,
	CTK_SOURCE_ENCODING_ISO_8859_15,
	CTK_SOURCE_ENCODING_ISO_8859_16,

	CTK_SOURCE_ENCODING_UTF_7,
	CTK_SOURCE_ENCODING_UTF_16,
	CTK_SOURCE_ENCODING_UTF_16_BE,
	CTK_SOURCE_ENCODING_UTF_16_LE,
	CTK_SOURCE_ENCODING_UTF_32,
	CTK_SOURCE_ENCODING_UCS_2,
	CTK_SOURCE_ENCODING_UCS_4,

	CTK_SOURCE_ENCODING_ARMSCII_8,
	CTK_SOURCE_ENCODING_BIG5,
	CTK_SOURCE_ENCODING_BIG5_HKSCS,
	CTK_SOURCE_ENCODING_CP_866,

	CTK_SOURCE_ENCODING_EUC_JP,
	CTK_SOURCE_ENCODING_EUC_JP_MS,
	CTK_SOURCE_ENCODING_CP932,
	CTK_SOURCE_ENCODING_EUC_KR,
	CTK_SOURCE_ENCODING_EUC_TW,

	CTK_SOURCE_ENCODING_GB18030,
	CTK_SOURCE_ENCODING_GB2312,
	CTK_SOURCE_ENCODING_GBK,
	CTK_SOURCE_ENCODING_GEOSTD8,

	CTK_SOURCE_ENCODING_IBM_850,
	CTK_SOURCE_ENCODING_IBM_852,
	CTK_SOURCE_ENCODING_IBM_855,
	CTK_SOURCE_ENCODING_IBM_857,
	CTK_SOURCE_ENCODING_IBM_862,
	CTK_SOURCE_ENCODING_IBM_864,

	CTK_SOURCE_ENCODING_ISO_2022_JP,
	CTK_SOURCE_ENCODING_ISO_2022_KR,
	CTK_SOURCE_ENCODING_ISO_IR_111,
	CTK_SOURCE_ENCODING_JOHAB,
	CTK_SOURCE_ENCODING_KOI8_R,
	CTK_SOURCE_ENCODING_KOI8__R,
	CTK_SOURCE_ENCODING_KOI8_U,

	CTK_SOURCE_ENCODING_SHIFT_JIS,
	CTK_SOURCE_ENCODING_TCVN,
	CTK_SOURCE_ENCODING_TIS_620,
	CTK_SOURCE_ENCODING_UHC,
	CTK_SOURCE_ENCODING_VISCII,

	CTK_SOURCE_ENCODING_WINDOWS_1250,
	CTK_SOURCE_ENCODING_WINDOWS_1251,
	CTK_SOURCE_ENCODING_WINDOWS_1252,
	CTK_SOURCE_ENCODING_WINDOWS_1253,
	CTK_SOURCE_ENCODING_WINDOWS_1254,
	CTK_SOURCE_ENCODING_WINDOWS_1255,
	CTK_SOURCE_ENCODING_WINDOWS_1256,
	CTK_SOURCE_ENCODING_WINDOWS_1257,
	CTK_SOURCE_ENCODING_WINDOWS_1258,

	CTK_SOURCE_ENCODING_LAST,

	CTK_SOURCE_ENCODING_UTF_8,
	CTK_SOURCE_ENCODING_UNKNOWN
} GtkSourceEncodingIndex;

static const GtkSourceEncoding utf8_encoding =
{
	CTK_SOURCE_ENCODING_UTF_8,
	"UTF-8",
	N_("Unicode")
};

/* Initialized in ctk_source_encoding_lazy_init(). */
static GtkSourceEncoding unknown_encoding =
{
	CTK_SOURCE_ENCODING_UNKNOWN,
	NULL,
	NULL
};

static const GtkSourceEncoding encodings[] =
{
	{ CTK_SOURCE_ENCODING_ISO_8859_1,
	  "ISO-8859-1", N_("Western") },
	{ CTK_SOURCE_ENCODING_ISO_8859_2,
	 "ISO-8859-2", N_("Central European") },
	{ CTK_SOURCE_ENCODING_ISO_8859_3,
	  "ISO-8859-3", N_("South European") },
	{ CTK_SOURCE_ENCODING_ISO_8859_4,
	  "ISO-8859-4", N_("Baltic") },
	{ CTK_SOURCE_ENCODING_ISO_8859_5,
	  "ISO-8859-5", N_("Cyrillic") },
	{ CTK_SOURCE_ENCODING_ISO_8859_6,
	  "ISO-8859-6", N_("Arabic") },
	{ CTK_SOURCE_ENCODING_ISO_8859_7,
	  "ISO-8859-7", N_("Greek") },
	{ CTK_SOURCE_ENCODING_ISO_8859_8,
	  "ISO-8859-8", N_("Hebrew Visual") },
	{ CTK_SOURCE_ENCODING_ISO_8859_9,
	  "ISO-8859-9", N_("Turkish") },
	{ CTK_SOURCE_ENCODING_ISO_8859_10,
	  "ISO-8859-10", N_("Nordic") },
	{ CTK_SOURCE_ENCODING_ISO_8859_13,
	  "ISO-8859-13", N_("Baltic") },
	{ CTK_SOURCE_ENCODING_ISO_8859_14,
	  "ISO-8859-14", N_("Celtic") },
	{ CTK_SOURCE_ENCODING_ISO_8859_15,
	  "ISO-8859-15", N_("Western") },
	{ CTK_SOURCE_ENCODING_ISO_8859_16,
	  "ISO-8859-16", N_("Romanian") },

	{ CTK_SOURCE_ENCODING_UTF_7,
	  "UTF-7", N_("Unicode") },
	{ CTK_SOURCE_ENCODING_UTF_16,
	  "UTF-16", N_("Unicode") },
	{ CTK_SOURCE_ENCODING_UTF_16_BE,
	  "UTF-16BE", N_("Unicode") },
	{ CTK_SOURCE_ENCODING_UTF_16_LE,
	  "UTF-16LE", N_("Unicode") },
	{ CTK_SOURCE_ENCODING_UTF_32,
	  "UTF-32", N_("Unicode") },
	{ CTK_SOURCE_ENCODING_UCS_2,
	  "UCS-2", N_("Unicode") },
	{ CTK_SOURCE_ENCODING_UCS_4,
	  "UCS-4", N_("Unicode") },

	{ CTK_SOURCE_ENCODING_ARMSCII_8,
	  "ARMSCII-8", N_("Armenian") },
	{ CTK_SOURCE_ENCODING_BIG5,
	  "BIG5", N_("Chinese Traditional") },
	{ CTK_SOURCE_ENCODING_BIG5_HKSCS,
	  "BIG5-HKSCS", N_("Chinese Traditional") },
	{ CTK_SOURCE_ENCODING_CP_866,
	  "CP866", N_("Cyrillic/Russian") },

	{ CTK_SOURCE_ENCODING_EUC_JP,
	  "EUC-JP", N_("Japanese") },
	{ CTK_SOURCE_ENCODING_EUC_JP_MS,
	  "EUC-JP-MS", N_("Japanese") },
	{ CTK_SOURCE_ENCODING_CP932,
	  "CP932", N_("Japanese") },

	{ CTK_SOURCE_ENCODING_EUC_KR,
	  "EUC-KR", N_("Korean") },
	{ CTK_SOURCE_ENCODING_EUC_TW,
	  "EUC-TW", N_("Chinese Traditional") },

	{ CTK_SOURCE_ENCODING_GB18030,
	  "GB18030", N_("Chinese Simplified") },
	{ CTK_SOURCE_ENCODING_GB2312,
	  "GB2312", N_("Chinese Simplified") },
	{ CTK_SOURCE_ENCODING_GBK,
	  "GBK", N_("Chinese Simplified") },
	{ CTK_SOURCE_ENCODING_GEOSTD8,
	  "GEORGIAN-ACADEMY", N_("Georgian") }, /* FIXME GEOSTD8 ? */

	{ CTK_SOURCE_ENCODING_IBM_850,
	  "IBM850", N_("Western") },
	{ CTK_SOURCE_ENCODING_IBM_852,
	  "IBM852", N_("Central European") },
	{ CTK_SOURCE_ENCODING_IBM_855,
	  "IBM855", N_("Cyrillic") },
	{ CTK_SOURCE_ENCODING_IBM_857,
	  "IBM857", N_("Turkish") },
	{ CTK_SOURCE_ENCODING_IBM_862,
	  "IBM862", N_("Hebrew") },
	{ CTK_SOURCE_ENCODING_IBM_864,
	  "IBM864", N_("Arabic") },

	{ CTK_SOURCE_ENCODING_ISO_2022_JP,
	  "ISO-2022-JP", N_("Japanese") },
	{ CTK_SOURCE_ENCODING_ISO_2022_KR,
	  "ISO-2022-KR", N_("Korean") },
	{ CTK_SOURCE_ENCODING_ISO_IR_111,
	  "ISO-IR-111", N_("Cyrillic") },
	{ CTK_SOURCE_ENCODING_JOHAB,
	  "JOHAB", N_("Korean") },
	{ CTK_SOURCE_ENCODING_KOI8_R,
	  "KOI8R", N_("Cyrillic") },
	{ CTK_SOURCE_ENCODING_KOI8__R,
	  "KOI8-R", N_("Cyrillic") },
	{ CTK_SOURCE_ENCODING_KOI8_U,
	  "KOI8U", N_("Cyrillic/Ukrainian") },

	{ CTK_SOURCE_ENCODING_SHIFT_JIS,
	  "SHIFT_JIS", N_("Japanese") },
	{ CTK_SOURCE_ENCODING_TCVN,
	  "TCVN", N_("Vietnamese") },
	{ CTK_SOURCE_ENCODING_TIS_620,
	  "TIS-620", N_("Thai") },
	{ CTK_SOURCE_ENCODING_UHC,
	  "UHC", N_("Korean") },
	{ CTK_SOURCE_ENCODING_VISCII,
	  "VISCII", N_("Vietnamese") },

	{ CTK_SOURCE_ENCODING_WINDOWS_1250,
	  "WINDOWS-1250", N_("Central European") },
	{ CTK_SOURCE_ENCODING_WINDOWS_1251,
	  "WINDOWS-1251", N_("Cyrillic") },
	{ CTK_SOURCE_ENCODING_WINDOWS_1252,
	  "WINDOWS-1252", N_("Western") },
	{ CTK_SOURCE_ENCODING_WINDOWS_1253,
	  "WINDOWS-1253", N_("Greek") },
	{ CTK_SOURCE_ENCODING_WINDOWS_1254,
	  "WINDOWS-1254", N_("Turkish") },
	{ CTK_SOURCE_ENCODING_WINDOWS_1255,
	  "WINDOWS-1255", N_("Hebrew") },
	{ CTK_SOURCE_ENCODING_WINDOWS_1256,
	  "WINDOWS-1256", N_("Arabic") },
	{ CTK_SOURCE_ENCODING_WINDOWS_1257,
	  "WINDOWS-1257", N_("Baltic") },
	{ CTK_SOURCE_ENCODING_WINDOWS_1258,
	  "WINDOWS-1258", N_("Vietnamese") }
};

static void
ctk_source_encoding_lazy_init (void)
{
	static gboolean initialized = FALSE;
	const gchar *locale_charset;

	if (G_LIKELY (initialized))
	{
		return;
	}

	if (g_get_charset (&locale_charset) == FALSE)
	{
		unknown_encoding.charset = g_strdup (locale_charset);
	}

	initialized = TRUE;
}

/**
 * ctk_source_encoding_get_from_charset:
 * @charset: a character set.
 *
 * Gets a #GtkSourceEncoding from a character set such as "UTF-8" or
 * "ISO-8859-1".
 *
 * Returns: (nullable): the corresponding #GtkSourceEncoding, or %NULL
 * if not found.
 * Since: 3.14
 */
const GtkSourceEncoding *
ctk_source_encoding_get_from_charset (const gchar *charset)
{
	gint i;

	g_return_val_if_fail (charset != NULL, NULL);

	if (g_ascii_strcasecmp (charset, "UTF-8") == 0)
	{
		return ctk_source_encoding_get_utf8 ();
	}

	for (i = 0; i < CTK_SOURCE_ENCODING_LAST; i++)
	{
		if (g_ascii_strcasecmp (charset, encodings[i].charset) == 0)
		{
			return &encodings[i];
		}
	}

	ctk_source_encoding_lazy_init ();

	if (unknown_encoding.charset != NULL &&
	    g_ascii_strcasecmp (charset, unknown_encoding.charset) == 0)
	{
		return &unknown_encoding;
	}

	return NULL;
}

/**
 * ctk_source_encoding_get_all:
 *
 * Gets all encodings.
 *
 * Returns: (transfer container) (element-type GtkSource.Encoding): a list of
 * all #GtkSourceEncoding's. Free with g_slist_free().
 * Since: 3.14
 */
GSList *
ctk_source_encoding_get_all (void)
{
	GSList *all = NULL;
	gint i;

	for (i = CTK_SOURCE_ENCODING_LAST - 1; i >= 0; i--)
	{
		all = g_slist_prepend (all, (gpointer) &encodings[i]);
	}

	all = g_slist_prepend (all, (gpointer) &utf8_encoding);

	return all;
}

/**
 * ctk_source_encoding_get_utf8:
 *
 * Returns: the UTF-8 encoding.
 * Since: 3.14
 */
const GtkSourceEncoding *
ctk_source_encoding_get_utf8 (void)
{
	return &utf8_encoding;
}

/**
 * ctk_source_encoding_get_current:
 *
 * Gets the #GtkSourceEncoding for the current locale. See also g_get_charset().
 *
 * Returns: the current locale encoding.
 * Since: 3.14
 */
const GtkSourceEncoding *
ctk_source_encoding_get_current (void)
{
	static gboolean initialized = FALSE;
	static const GtkSourceEncoding *locale_encoding = NULL;

	const gchar *locale_charset;

	ctk_source_encoding_lazy_init ();

	if (G_LIKELY (initialized))
	{
		return locale_encoding;
	}

	if (g_get_charset (&locale_charset))
	{
		locale_encoding = &utf8_encoding;
	}
	else
	{
		locale_encoding = ctk_source_encoding_get_from_charset (locale_charset);
	}

	if (locale_encoding == NULL)
	{
		locale_encoding = &unknown_encoding;
	}

	initialized = TRUE;

	return locale_encoding;
}

/**
 * ctk_source_encoding_to_string:
 * @enc: a #GtkSourceEncoding.
 *
 * Returns: a string representation. Free with g_free() when no longer needed.
 * Since: 3.14
 */
gchar *
ctk_source_encoding_to_string (const GtkSourceEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	ctk_source_encoding_lazy_init ();

	g_return_val_if_fail (enc->charset != NULL, NULL);

	if (enc->name != NULL)
	{
	    	return g_strdup_printf ("%s (%s)", _(enc->name), enc->charset);
	}
	else if (g_ascii_strcasecmp (enc->charset, "ANSI_X3.4-1968") == 0)
	{
		return g_strdup_printf ("US-ASCII (%s)", enc->charset);
	}
	else
	{
		return g_strdup (enc->charset);
	}
}

/**
 * ctk_source_encoding_get_charset:
 * @enc: a #GtkSourceEncoding.
 *
 * Gets the character set of the #GtkSourceEncoding, such as "UTF-8" or
 * "ISO-8859-1".
 *
 * Returns: the character set of the #GtkSourceEncoding.
 * Since: 3.14
 */
const gchar *
ctk_source_encoding_get_charset (const GtkSourceEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	ctk_source_encoding_lazy_init ();

	g_return_val_if_fail (enc->charset != NULL, NULL);

	return enc->charset;
}

/**
 * ctk_source_encoding_get_name:
 * @enc: a #GtkSourceEncoding.
 *
 * Gets the name of the #GtkSourceEncoding such as "Unicode" or "Western".
 *
 * Returns: the name of the #GtkSourceEncoding.
 * Since: 3.14
 */
const gchar *
ctk_source_encoding_get_name (const GtkSourceEncoding* enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	ctk_source_encoding_lazy_init ();

	return (enc->name == NULL) ? _("Unknown") : _(enc->name);
}

static GSList *
strv_to_list (const gchar * const *enc_str)
{
	GSList *res = NULL;
	gchar **p;

	for (p = (gchar **)enc_str; p != NULL && *p != NULL; p++)
	{
		const gchar *charset = *p;
		const GtkSourceEncoding *enc;

		if (g_str_equal (charset, "CURRENT"))
		{
			g_get_charset (&charset);
		}

		g_return_val_if_fail (charset != NULL, NULL);
		enc = ctk_source_encoding_get_from_charset (charset);

		if (enc != NULL &&
		    g_slist_find (res, enc) == NULL)
		{
			res = g_slist_prepend (res, (gpointer)enc);
		}
	}

	return g_slist_reverse (res);
}

static GSList *
remove_duplicates_keep_first (GSList *list)
{
	GSList *new_list = NULL;
	GSList *l;

	for (l = list; l != NULL; l = l->next)
	{
		gpointer cur_encoding = l->data;

		if (g_slist_find (new_list, cur_encoding) == NULL)
		{
			new_list = g_slist_prepend (new_list, cur_encoding);
		}
	}

	new_list = g_slist_reverse (new_list);

	g_slist_free (list);
	return new_list;
}

static GSList *
remove_duplicates_keep_last (GSList *list)
{
	GSList *new_list = NULL;
	GSList *l;

	list = g_slist_reverse (list);

	for (l = list; l != NULL; l = l->next)
	{
		gpointer cur_encoding = l->data;

		if (g_slist_find (new_list, cur_encoding) == NULL)
		{
			new_list = g_slist_prepend (new_list, cur_encoding);
		}
	}

	g_slist_free (list);
	return new_list;
}

/*
 * _ctk_source_encoding_remove_duplicates:
 * @list: (element-type GtkSource.Encoding): a list of #GtkSourceEncoding's.
 * @removal_type: the #GtkSourceEncodingDuplicates.
 *
 * A convenience function to remove duplicated encodings in a list.
 *
 * Returns: (transfer container) (element-type GtkSource.Encoding): the new
 * start of the #GSList.
 * Since: 3.14
 */
GSList *
_ctk_source_encoding_remove_duplicates (GSList                      *list,
					GtkSourceEncodingDuplicates  removal_type)
{
	switch (removal_type)
	{
		case CTK_SOURCE_ENCODING_DUPLICATES_KEEP_FIRST:
			return remove_duplicates_keep_first (list);

		case CTK_SOURCE_ENCODING_DUPLICATES_KEEP_LAST:
			return remove_duplicates_keep_last (list);

		default:
			break;
	}

	g_return_val_if_reached (list);
}

/**
 * ctk_source_encoding_get_default_candidates:
 *
 * Gets the list of default candidate encodings to try when loading a file. See
 * ctk_source_file_loader_set_candidate_encodings().
 *
 * This function returns a different list depending on the current locale (i.e.
 * language, country and default encoding). The UTF-8 encoding and the current
 * locale encoding are guaranteed to be present in the returned list.
 *
 * Returns: (transfer container) (element-type GtkSource.Encoding): the list of
 * default candidate encodings. Free with g_slist_free().
 * Since: 3.18
 */
GSList *
ctk_source_encoding_get_default_candidates (void)
{
	const gchar *encodings_str;
	const gchar *encodings_str_translated;
	GVariant *encodings_variant;
	const gchar **encodings_strv;
	GSList *encodings_list;
	GError *error = NULL;

	/* Translators: This is the sorted list of encodings used by
	 * GtkSourceView for automatic detection of the file encoding. You may
	 * want to customize it adding encodings that are common in your
	 * country, for instance the GB18030 encoding for the Chinese
	 * translation. You may also want to remove the ISO-8859-15 encoding
	 * (covering English and most Western European languages) if you think
	 * people in your country will rarely use it.  "CURRENT" is a magic
	 * value used by GtkSourceView and it represents the encoding for the
	 * current locale, so please don't translate the "CURRENT" term.  Only
	 * recognized encodings are used. See
	 * https://gitlab.gnome.org/GNOME/ctksourceview/blob/master/ctksourceview/ctksourceencoding.c#L142
	 * for a list of supported encodings.
	 * Keep the same format: square brackets, single quotes, commas.
	 */
	encodings_str = N_("['UTF-8', 'CURRENT', 'ISO-8859-15', 'UTF-16']");

	encodings_str_translated = _(encodings_str);

	encodings_variant = g_variant_parse (G_VARIANT_TYPE_STRING_ARRAY,
					     encodings_str_translated,
					     NULL,
					     NULL,
					     &error);

	if (error != NULL)
	{
		const gchar * const *language_names = g_get_language_names ();

		g_warning ("Error while parsing encodings list for locale %s:\n"
			   "Translated list: %s\n"
			   "Error message: %s",
			   language_names[0],
			   encodings_str_translated,
			   error->message);

		g_clear_error (&error);

		encodings_variant = g_variant_parse (G_VARIANT_TYPE_STRING_ARRAY,
						     encodings_str,
						     NULL,
						     NULL,
						     &error);

		g_assert_no_error (error);
	}

	encodings_strv = g_variant_get_strv (encodings_variant, NULL);
	encodings_list = strv_to_list (encodings_strv);
	g_free ((gpointer) encodings_strv);

	/* Ensure that UTF-8 and CURRENT are present. */
	encodings_list = g_slist_prepend (encodings_list,
					  (gpointer) ctk_source_encoding_get_current ());

	encodings_list = g_slist_prepend (encodings_list,
					  (gpointer) &utf8_encoding);

	encodings_list = _ctk_source_encoding_remove_duplicates (encodings_list,
								 CTK_SOURCE_ENCODING_DUPLICATES_KEEP_LAST);

	g_variant_unref (encodings_variant);
	return encodings_list;
}

/**
 * ctk_source_encoding_copy:
 * @enc: a #GtkSourceEncoding.
 *
 * Used by language bindings.
 *
 * Returns: a copy of @enc.
 * Since: 3.14
 */
GtkSourceEncoding *
ctk_source_encoding_copy (const GtkSourceEncoding *enc)
{
	g_return_val_if_fail (enc != NULL, NULL);

	return (GtkSourceEncoding *) enc;
}

/**
 * ctk_source_encoding_free:
 * @enc: a #GtkSourceEncoding.
 *
 * Used by language bindings.
 *
 * Since: 3.14
 */
void
ctk_source_encoding_free (GtkSourceEncoding *enc)
{
	g_return_if_fail (enc != NULL);
}
