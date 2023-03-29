/*
 * This file is part of ctksourceview
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

#ifndef GTK_SOURCE_H
#define GTK_SOURCE_H

#define GTK_SOURCE_H_INSIDE

#include <ctksourceview/completion-providers/words/ctksourcecompletionwords.h>
#include <ctksourceview/ctksourcetypes.h>
#include <ctksourceview/ctksourcebuffer.h>
#include <ctksourceview/ctksourcecompletioncontext.h>
#include <ctksourceview/ctksourcecompletion.h>
#include <ctksourceview/ctksourcecompletioninfo.h>
#include <ctksourceview/ctksourcecompletionitem.h>
#include <ctksourceview/ctksourcecompletionproposal.h>
#include <ctksourceview/ctksourcecompletionprovider.h>
#include <ctksourceview/ctksourceencoding.h>
#include <ctksourceview/ctksourcefile.h>
#include <ctksourceview/ctksourcefileloader.h>
#include <ctksourceview/ctksourcefilesaver.h>
#include <ctksourceview/ctksourcegutter.h>
#include <ctksourceview/ctksourcegutterrenderer.h>
#include <ctksourceview/ctksourcegutterrenderertext.h>
#include <ctksourceview/ctksourcegutterrendererpixbuf.h>
#include <ctksourceview/ctksourceinit.h>
#include <ctksourceview/ctksourcelanguage.h>
#include <ctksourceview/ctksourcelanguagemanager.h>
#include <ctksourceview/ctksourcemap.h>
#include <ctksourceview/ctksourcemark.h>
#include <ctksourceview/ctksourcemarkattributes.h>
#include <ctksourceview/ctksourceprintcompositor.h>
#include <ctksourceview/ctksourceregion.h>
#include <ctksourceview/ctksourcesearchcontext.h>
#include <ctksourceview/ctksourcesearchsettings.h>
#include <ctksourceview/ctksourcespacedrawer.h>
#include <ctksourceview/ctksourcestyle.h>
#include <ctksourceview/ctksourcestylescheme.h>
#include <ctksourceview/ctksourcestyleschemechooser.h>
#include <ctksourceview/ctksourcestyleschemechooserbutton.h>
#include <ctksourceview/ctksourcestyleschemechooserwidget.h>
#include <ctksourceview/ctksourcestyleschememanager.h>
#include <ctksourceview/ctksourcetag.h>
#include <ctksourceview/ctksourceundomanager.h>
#include <ctksourceview/ctksourceutils.h>
#include <ctksourceview/ctksourceversion.h>
#include <ctksourceview/ctksourceview.h>
#include <ctksourceview/ctksource-enumtypes.h>
#include <ctksourceview/ctksourceautocleanups.h>

#undef GTK_SOURCE_H_INSIDE

#endif /* GTK_SOURCE_H */
