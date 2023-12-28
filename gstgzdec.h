/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2020 Niels De Graef <niels.degraef@gmail.com>
 * Copyright (C) 2023 Cassiano Kleinert Casagrande <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GST_GZDEC_H__
#define __GST_GZDEC_H__

#include <gst/gst.h>
#include <stdint.h>

G_BEGIN_DECLS

#define GST_TYPE_GZDEC (gst_gzdec_get_type())
G_DECLARE_FINAL_TYPE (Gstgzdec, gst_gzdec,
    GST, GZDEC, GstElement)

// ZLIB suggests a different value but we used 2048 in order to test the behavior when multiple
// inflates calls per buffer are needed.
#define CHUNK 2048

struct _Gstgzdec
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  gboolean silent;
  uint8_t inchunk[CHUNK], outchunk[CHUNK];
  z_stream zstr;
};

G_END_DECLS

GST_DEBUG_CATEGORY_STATIC (gst_gzdec_debug);
#define GST_CAT_DEFAULT gst_gzdec_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

#define gst_gzdec_parent_class parent_class
G_DEFINE_TYPE (Gstgzdec, gst_gzdec, GST_TYPE_ELEMENT);

GST_ELEMENT_REGISTER_DEFINE (gzdec, "gzdec", GST_RANK_NONE,
    GST_TYPE_GZDEC);

static void gst_gzdec_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_gzdec_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

static gboolean gst_gzdec_sink_event (GstPad * pad,
    GstObject * parent, GstEvent * event);
static GstFlowReturn gst_gzdec_chain (GstPad * pad,
    GstObject * parent, GstBuffer * buf);



#endif /* __GST_GZDEC_H__ */
