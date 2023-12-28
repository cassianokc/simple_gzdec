/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gst/gst.h>

#include <zlib.h>
#include "gstgzdec.h"

/* GObject vmethod implementations */

/* initialize the gzdec's class */
    static void
gst_gzdec_class_init (GstgzdecClass * klass)
{
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;

    gobject_class = (GObjectClass *) klass;
    gstelement_class = (GstElementClass *) klass;

    gobject_class->set_property = gst_gzdec_set_property;
    gobject_class->get_property = gst_gzdec_get_property;

    g_object_class_install_property (gobject_class, PROP_SILENT,
            g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
                FALSE, G_PARAM_READWRITE));

    gst_element_class_set_details_simple (gstelement_class,
            "gzdec",
            "gzdec",
            "gzdec",
            "Cassiano Kleinert Casagrande <<cassianokleinert@gmail.com>>");

    gst_element_class_add_pad_template (gstelement_class,
            gst_static_pad_template_get (&src_factory));
    gst_element_class_add_pad_template (gstelement_class,
            gst_static_pad_template_get (&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad callback functions
 * initialize instance structure
 */
    static void
gst_gzdec_init (Gstgzdec * filter)
{
    filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
    gst_pad_set_event_function (filter->sinkpad,
            GST_DEBUG_FUNCPTR (gst_gzdec_sink_event));
    gst_pad_set_chain_function (filter->sinkpad,
            GST_DEBUG_FUNCPTR (gst_gzdec_chain));
    GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
    gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

    filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
    GST_PAD_SET_PROXY_CAPS (filter->srcpad);
    gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

    filter->silent = FALSE;

    // zlib setup for inflating
    filter->zstr.zalloc = Z_NULL;
    filter->zstr.zfree = Z_NULL;
    filter->zstr.opaque = Z_NULL;
    filter->zstr.avail_in = 0;
    filter->zstr.next_in = Z_NULL;
    int ret = inflateInit(&filter->zstr);
    if (ret != Z_OK)
        g_print("Failed ZLIB inflateInit call\n");

}

    static void
gst_gzdec_set_property (GObject * object, guint prop_id,
        const GValue * value, GParamSpec * pspec)
{
    Gstgzdec *filter = GST_GZDEC (object);

    switch (prop_id) {
        case PROP_SILENT:
            filter->silent = g_value_get_boolean (value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

    static void
gst_gzdec_get_property (GObject * object, guint prop_id,
        GValue * value, GParamSpec * pspec)
{
    Gstgzdec *filter = GST_GZDEC (object);

    switch (prop_id) {
        case PROP_SILENT:
            g_value_set_boolean (value, filter->silent);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
    static gboolean
gst_gzdec_sink_event (GstPad * pad, GstObject * parent,
        GstEvent * event)
{
    Gstgzdec *filter;
    gboolean ret;

    filter = GST_GZDEC (parent);

    GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT,
            GST_EVENT_TYPE_NAME (event), event);

    switch (GST_EVENT_TYPE (event)) {
        case GST_EVENT_CAPS:
            {
                GstCaps *caps;

                gst_event_parse_caps (event, &caps);
                /* do something with the caps */

                /* and forward */
                ret = gst_pad_event_default (pad, parent, event);
                break;
            }
        default:
            ret = gst_pad_event_default (pad, parent, event);
            break;
    }
    return ret;
}

/* chain function
 * this function does the actual processing
 */
    static GstFlowReturn
gst_gzdec_chain (GstPad * pad, GstObject * parent, GstBuffer * in_buf)
{
    Gstgzdec *filter;
    filter = GST_GZDEC (parent);

    gsize buf_sz = gst_buffer_get_size(in_buf); // total input buffer size
    gsize remaining_sz = buf_sz;                // stores how much of the buffer still needs to be processed
    gsize offset = 0, sz;

    if (!filter->silent) {
        g_print ("Buffer size: %lu\n", buf_sz);
    }

    GstMapInfo in_map;                          // mapping to access input buffer values
    GstBuffer *out_buf = gst_buffer_new();      // buffer that will contain full output
    GstBuffer *tmp_buf;                         // temporary buffer that contains output for each chunk
    uint8_t chunk[CHUNK];
    int ret;

    if (!gst_buffer_map(in_buf, &in_map, GST_MAP_READ)) {
        g_print("Failed mapping buffer\n");
        return GST_FLOW_ERROR;
    }

    do {
        sz = CHUNK > remaining_sz ? remaining_sz : CHUNK;
        if (!filter->silent) {
            g_print ("Remaining size: %lu\n", remaining_sz);
        }
        filter->zstr.next_in = &(in_map.data[offset]);
        filter->zstr.avail_in = sz;
        do {
            filter->zstr.avail_out = CHUNK;
            filter->zstr.next_out = chunk;
            if (!filter->silent) {
                g_print ("Inflating\n");
            }
            ret = inflate(&filter->zstr, Z_NO_FLUSH);
            if (ret != Z_OK && ret != Z_STREAM_END) {
                g_print("Failed inflating: %d\n", ret);
                gst_buffer_unmap(in_buf, &in_map);
                gst_buffer_unref(in_buf);
                inflateEnd(&filter->zstr);
                return GST_FLOW_ERROR;
            }
            tmp_buf = gst_buffer_new_memdup(chunk, CHUNK - filter->zstr.avail_out);
            gst_buffer_copy_into(out_buf, tmp_buf, GST_BUFFER_COPY_MEMORY, 0, -1);
            gst_buffer_unref(tmp_buf);
        } while (filter->zstr.avail_out == 0);           // tests whether input chunk was fully processed
        offset += sz;
        remaining_sz -= sz;
    } while (remaining_sz > 0);                          // tests whether full input buffer was fully processed
    gst_buffer_unmap(in_buf, &in_map);
    gst_buffer_unref(in_buf);
    return gst_pad_push (filter->srcpad, out_buf);
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean gzdec_init (GstPlugin * gzdec)
{
    /* debug category for filtering log messages
     *
     * exchange the string 'Template gzdec' with your description
     */
    GST_DEBUG_CATEGORY_INIT (gst_gzdec_debug, "gzdec",
            0, "gzdec");

    return GST_ELEMENT_REGISTER (gzdec, gzdec);
}

/* PACKAGE: this is usually set by meson depending on some _INIT macro
 * in meson.build and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use meson to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "gzdec"
#endif

/* gstreamer looks for this structure to register gzdecs
 *
 * exchange the string 'Template gzdec' with your gzdec description
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR, GST_VERSION_MINOR, gzdec,
        "gzdec",
        gzdec_init,
        PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
