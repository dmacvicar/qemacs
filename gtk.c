/*
 * GTK display driver for QEmacs
 * Copyright (c) 2018 Duncan Mac-Vicar P.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define G_LOG_USE_STRUCTURED 1
#include <gtk/gtk.h>
#include "qe.h"
// Screen driver implementation

static int force_tty = 0;
static int font_xsize;
static int font_ptsize;

void set_read_handler(int fd, void (*cb)(void *opaque), void *opaque)
{
    g_debug("set_read_handler");
}

void set_write_handler(int fd, void (*cb)(void *opaque), void *opaque)
{
    g_debug("set_write_handler");
}

/* register a callback which is called when process 'pid'
   terminates. When the callback is set to NULL, it is deleted */
/* XXX: add consistency check ? */
int set_pid_handler(int pid,
                    void (*cb)(void *opaque, int status), void *opaque)
{
    return 0;
}

/*
 * add an explicit call back to avoid recursions
 */
void register_bottom_half(void (*cb)(void *opaque), void *opaque)
{
}

/*
 * remove bottom half
 */
void unregister_bottom_half(void (*cb)(void *opaque), void *opaque)
{
}

QETimer *qe_add_timer(int delay, void *opaque, void (*cb)(void *opaque))
{
    return NULL;
}

void qe_kill_timer(QETimer **tip)
{
}

void url_main_loop(void (*init)(void *opaque), void *opaque)
{
    g_debug("gtk url_main_loop: calling init");
    init(opaque);
    g_debug("gtk url_main_loop: calling gtk_main");
    gtk_main();
    g_debug("gtk url_main_loop quit");
}

/* exit from url loop */
void url_exit(void)
{
    g_debug("gtk: url_exit: calling gtk_main_quit");
    gtk_main_quit();
}

static void qe_gtk_handle_event(void *opaque);

static QEFont *qe_gtk_open_font(QEditScreen *s, int style, int size);
static void qe_gtk_close_font(QEditScreen *s, QEFont **fontp);

struct QEGtkState {
    GtkWidget *window;
    GtkWidget *drawing_area;
    cairo_surface_t *surface;
    cairo_rectangle_t cursor;
    GtkAllocation clip;
};

static gboolean
on_key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    g_debug("gtk: on_key_press_event");
    QEKeyEvent ev;
    ev.type = QE_KEY_EVENT;
    ev.key = KEY_NONE;

    gboolean shift = event->state & GDK_SHIFT_MASK;
    gboolean meta = event->state & GDK_META_MASK;
    gboolean ctrl = event->state & GDK_CONTROL_MASK;

    guint32 key;
    switch (event->keyval)
    {
    case GDK_KEY_Tab:
        ev.key = shift ? KEY_SHIFT_TAB : KEY_TAB;
        break;
    case GDK_KEY_Return:
        ev.key = KEY_RET;
        break;
    case GDK_KEY_Escape:
        ev.key = KEY_ESC;
        break;
    //case GDK_KEY_Delete:
    //    ev.key = KEY_DEL;
    //    break;
    case GDK_KEY_BackSpace:
        ev.key = meta ? KEY_META(KEY_BS) : KEY_BS;
        break;
    case GDK_KEY_Up:
        ev.key = ctrl ? KEY_CTRL_UP : KEY_UP;
        break;
    case GDK_KEY_Down:
        ev.key = ctrl ? KEY_CTRL_DOWN : KEY_DOWN;
        break;
    case GDK_KEY_Right:
        ev.key = ctrl ? KEY_CTRL_RIGHT: KEY_RIGHT;
        break;
    case GDK_KEY_Left:
        ev.key = ctrl ? KEY_CTRL_LEFT : KEY_LEFT;
        break;
    case GDK_KEY_End:
        ev.key = ctrl ? KEY_CTRL_END : KEY_END;
        break;
    case GDK_KEY_Home:
        ev.key = ctrl ? KEY_CTRL_HOME : KEY_HOME;
        break;
    case GDK_KEY_Page_Up:
        ev.key = ctrl ? KEY_CTRL_PAGEUP : KEY_PAGEUP;
        break;
   case GDK_KEY_Page_Down:
        ev.key = ctrl ? KEY_CTRL_PAGEDOWN : KEY_PAGEDOWN;
        break;
   case GDK_KEY_Insert:
        ev.key = KEY_INSERT;
        break;
   case GDK_KEY_Delete:
        ev.key = KEY_DELETE;
        break;
   case GDK_KEY_F1:
        ev.key = KEY_F1;
        break;
   case GDK_KEY_F2:
        ev.key = KEY_F2;
        break;
   case GDK_KEY_F3:
        ev.key = KEY_F3;
        break;
   case GDK_KEY_F4:
        ev.key = KEY_F4;
        break;
   case GDK_KEY_F5:
        ev.key = KEY_F5;
        break;
   case GDK_KEY_F6:
        ev.key = KEY_F6;
        break;
   case GDK_KEY_F7:
        ev.key = KEY_F7;
        break;
   case GDK_KEY_F8:
        ev.key = KEY_F8;
        break;
   case GDK_KEY_F9:
        ev.key = KEY_F9;
        break;
   case GDK_KEY_F10:
        ev.key = KEY_F10;
        break;
   case GDK_KEY_F11:
        ev.key = KEY_F11;
        break;
   case GDK_KEY_F12:
        ev.key = KEY_F12;
        break;
   case GDK_KEY_F13:
        ev.key = KEY_F13;
        break;
   case GDK_KEY_F14:
        ev.key = KEY_F14;
        break;
   case GDK_KEY_F15:
        ev.key = KEY_F15;
        break;
   case GDK_KEY_F16:
        ev.key = KEY_F16;
        break;
   case GDK_KEY_F17:
        ev.key = KEY_F17;
        break;
   case GDK_KEY_F18:
        ev.key = KEY_F18;
        break;
   case GDK_KEY_F19:
        ev.key = KEY_F19;
        break;
   case GDK_KEY_F20:
        ev.key = KEY_F20;
        break;
    default:
        key = gdk_keyval_to_unicode (event->keyval);
        if (meta) {
            ev.key = KEY_META(key);
        } else if (ctrl) {
            ev.key = KEY_CTRL(key);
        } else {
            ev.key = key;
        }
        g_debug("key pressed: %c\n", key);
    }
    qe_handle_event((QEEvent *)&ev);
    return TRUE;
}

gboolean on_draw_event(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
    g_debug("draw!!!");
    struct QEGtkState *state = (struct QEGtkState *) user_data;
    cairo_set_source_surface (cr, state->surface, 0, 0);
    cairo_paint (cr);
    return FALSE;
}

void on_configure_event(GtkWindow *window, GdkEvent *event, gpointer user_data)
{
    g_debug("configure window");
}

gboolean on_configure_area_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    g_debug("configure_area");
    struct QEGtkState *state = (struct QEGtkState *) user_data;

    if (state->surface)
        cairo_surface_destroy (state->surface);

    state->surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                                        CAIRO_CONTENT_COLOR,
                                                        gtk_widget_get_allocated_width (widget),
                                                        gtk_widget_get_allocated_height (widget));

    g_debug("configure_area %d x %d : %d", event->configure.width, event->configure.height, gtk_widget_get_allocated_width (widget));

    qe_state.screen->width = event->configure.width;
    qe_state.screen->height = event->configure.height;

    QEEvent ev;
    ev.expose_event.type = QE_EXPOSE_EVENT;
    qe_handle_event((QEEvent *)&ev);

    return TRUE;
}

static void qe_gtk_text_metrics(QEditScreen *s, QEFont *font, 
                               QECharMetrics *metrics,
                                const unsigned int *str, int len);

static int qe_gtk_init(QEditScreen *s, int w, int h)
{
    g_debug("qe_gtk init");
    int xsize, ysize, font_ysize;
    QEStyleDef default_style;
    struct QEGtkState *state = qe_mallocz(struct QEGtkState);

    gtk_init(NULL, NULL);
    state->surface = NULL;
    state->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title (GTK_WINDOW (state->window), "qemacs");

    state->drawing_area = gtk_drawing_area_new ();
    gtk_container_add (GTK_CONTAINER (state->window), state->drawing_area);

    g_signal_connect (state->window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    
    gtk_widget_add_events (state->window, GDK_KEY_PRESS_MASK);
    gtk_widget_add_events (state->drawing_area, GDK_KEY_PRESS_MASK);
    gtk_widget_add_events (state->drawing_area, GDK_BUTTON_PRESS_MASK);
    gtk_widget_add_events (state->drawing_area, GDK_EXPOSURE_MASK);

    // FIXME why the event comes from the window?
    g_signal_connect (state->window, "key-press-event",
                    G_CALLBACK (on_key_press_event), NULL);
    g_signal_connect (state->window, "configure-event",
                      G_CALLBACK (on_configure_event), state);

    //g_signal_connect (state->drawing_area, "key_press_event",
    //                  G_CALLBACK (on_key_press_event), NULL);
    g_signal_connect(state->drawing_area, "draw",
                     G_CALLBACK(on_draw_event), state);
    g_signal_connect (state->drawing_area, "configure-event",
                      G_CALLBACK (on_configure_area_event), state);

    s->priv_data = state;

    /* At this point, we should be able to ask for metrics */
    if (font_ptsize)
        qe_styles[0].font_size = font_ptsize;
    get_style(NULL, &default_style, 0);

    QEFont *font = qe_gtk_open_font(s, default_style.font_style,
                                    default_style.font_size);
    if (!font) {
        fprintf(stderr, "Could not open default font\n");
        exit(1);
    }
    QECharMetrics metrics;
    unsigned int txt = 'b';
    qe_gtk_text_metrics(s, font, &metrics, &txt, 1);
    font_xsize = metrics.width;
    font_ysize = max(metrics.font_ascent, font->ascent);
    qe_gtk_close_font(s, &font);

    if (w == 0)
        w = 80;
    if (h == 0)
        h = 25;
    xsize = w * font_xsize;
    ysize = h * font_ysize;

    s->width = xsize;
    s->height = ysize;
    s->charset = &charset_utf8;

    s->clip_x1 = 0;
    s->clip_y1 = 0;
    s->clip_x2 = s->width;
    s->clip_y2 = s->height;

    g_debug("activate initial size %d x %d", s->width, s->height);
    gtk_widget_set_size_request (state->drawing_area, xsize, ysize);
    gtk_widget_show_all (state->window);
    g_debug("qe_gtk inited");
    return 2;
}

static void qe_gtk_close(QEditScreen *s)
{
    g_debug("qe_gtk_close");
    struct QEGtkState *state = (struct QEGtkState *) s->priv_data;

    cairo_surface_destroy (state->surface);
}

static void qe_gtk_flush(QEditScreen *s)
{
    g_debug("qe_gtk_flush");
    struct QEGtkState *state = (struct QEGtkState *)s->priv_data;
    // FIXME for some reason only flushing the current clip does not work
    //gtk_widget_queue_draw_area(state->drawing_area, state->clip.x, state->clip.y, state->clip.width, state->clip.height);
    gtk_widget_queue_draw(state->drawing_area);
}

static int qe_gtk_is_user_input_pending(QEditScreen *s)
{
    g_debug("qe_gtk_is_input_pending");
}

static void qe_gtk_fill_rectangle(QEditScreen *s,
                              int x1, int y1, int w, int h, QEColor color)
{
    g_debug("qe_gtk_fill_rectangle %d %d %d %d", x1, y1, w, h);
    struct QEGtkState *state = (struct QEGtkState *)s->priv_data;
    cairo_t *cr;
    cr = cairo_create (state->surface);
    cairo_set_source_rgba(cr, color >> 16 & 255, color >> 8 & 255, color >> 0 & 255, color >> 24 & 255);
    //cairo_rectangle (cr, PANGO_PIXELS(x1), PANGO_PIXELS(y1), PANGO_PIXELS(w), PANGO_PIXELS(h));
    cairo_rectangle (cr, x1, y1, w, h);
    cairo_fill (cr);
    cairo_destroy (cr);
}

static QEFont *qe_gtk_open_font(QEditScreen *s, int style, int size)
{
    g_debug("qe_gtk_open_font: size %d", size);
    struct QEGtkState *state = (struct QEGtkState *)s->priv_data;
    QEFont *font;

    font = qe_mallocz(QEFont);
    if (!font)
        return NULL;

    PangoFontDescription *fdesc = pango_font_description_new ();

    switch (style & QE_FAMILY_MASK) {
    case QE_FAMILY_SANS:
        pango_font_description_set_family_static (fdesc, "sans-serif");
        break;
    case QE_FAMILY_SERIF:
        pango_font_description_set_family_static (fdesc, "serif");
        break;
    case QE_FAMILY_FIXED:
    default:
        pango_font_description_set_family_static (fdesc, "monospace");
        break;
    }
    pango_font_description_set_size(fdesc, size * PANGO_SCALE);

    if (style & QE_STYLE_BOLD)
        pango_font_description_set_weight(fdesc, PANGO_WEIGHT_BOLD);
    if (style & QE_STYLE_ITALIC)
        pango_font_description_set_style(fdesc, PANGO_STYLE_ITALIC);
    //TODO UNDERLINE AND STRIKE TRHOUGH

    //g_debug("font requested (%s)", pango_font_description_to_string(fdesc));
    PangoContext *pc = gtk_widget_get_pango_context (state->drawing_area);
    PangoFontMetrics *metrics = pango_context_get_metrics(pc, fdesc, NULL);

    font->ascent = PANGO_PIXELS(pango_font_metrics_get_ascent (metrics));
    font->descent = PANGO_PIXELS(pango_font_metrics_get_descent (metrics));
    pango_font_metrics_unref (metrics);
    font->priv_data = fdesc;
    return font;
}

static void qe_gtk_close_font(QEditScreen *s, QEFont **fontp)
{
    g_debug("qe_gtk_close_font");
    QEFont *font = *fontp;

    if (font) {
        PangoFontDescription *fdesc = (PangoFontDescription *)font->priv_data;
        pango_font_description_free(fdesc);
        /* Clear structure to force crash if font is still used after
         * close_font.
         */
        memset(font, 0, sizeof(*font));
        qe_free(fontp);
    }
}

// Uses pango to calculate the metrics for a given text and len
// If the text is NULL, but len is > 0, the function uses
// pango_font_metrics_get_approximate_char_width to aproximate
// metrics for the givenlen, useful when setting up the initial
// window size
static void qe_gtk_text_metrics(QEditScreen *s, QEFont *font,
                               QECharMetrics *metrics,
                               const unsigned int *str, int len)
{
    g_debug("qe_gtk_text_metrics");
    struct QEGtkState *state = (struct QEGtkState *)s->priv_data;
    PangoFontDescription *fdesc = (PangoFontDescription *)font->priv_data;
    PangoContext *pc = gtk_widget_get_pango_context (state->drawing_area);

    metrics->font_ascent = font->ascent;
    metrics->font_descent = font->descent;
    PangoLayout *layout = pango_layout_new (pc);
    pango_layout_set_font_description (layout, fdesc);

    glong items_read;
    glong items_written;
    GError *error = NULL;

    gchar *text = g_ucs4_to_utf8(str, len, &items_read, &items_written, &error);
    if (text == NULL) {
        g_error("%s", error->message);
        g_free(error);
    }

    pango_layout_set_height(layout, -1);
    pango_layout_set_text(layout, text, -1);
    int height = 0;
    pango_layout_get_pixel_size(layout, &metrics->width, &height);
    g_debug("  metrics '%s' w %d h %d SCALE: %f", str, metrics->width, height, PANGO_SCALE);
    g_object_unref (layout);

}

static void qe_gtk_draw_text(QEditScreen *s, QEFont *font,
                         int x1, int y, const unsigned int *str, int len,
                         QEColor color)
{
    g_debug("qe_gtk_draw_text %d %d: '%s'", x1, y, str);

    struct QEGtkState *state = (struct QEGtkState *)s->priv_data;
    PangoFontDescription *fdesc = (PangoFontDescription *)font->priv_data;

    cairo_t *cr;
    /* Paint to the surface, where we store our state */
    cr = cairo_create (state->surface);
    PangoContext *pc = gtk_widget_get_pango_context (state->drawing_area);
    PangoLayout *layout = pango_layout_new (pc);
    pango_layout_set_font_description (layout, fdesc);

    glong items_read;
    glong items_written;
    GError *error;
    gchar *text = g_ucs4_to_utf8(str, len, &items_read, &items_written, &error);
    if (text == NULL) {
        g_error("%s", error->message);
        g_free(error);
    }
    pango_layout_set_height(layout, -1);
    pango_layout_set_text(layout, text, -1);

    cairo_set_source_rgba(cr, color >> 16 & 255, color >> 8 & 255, color >> 0 & 255, color >> 24 & 255);
    cairo_move_to (cr, x1, y - font->ascent);
    pango_cairo_update_layout(cr, layout);
    pango_cairo_show_layout (cr, layout);
    cairo_destroy (cr);
    g_object_unref (layout);
    g_free(text);
}

static void qe_gtk_set_clip(QEditScreen *s,
                        int x, int y, int w, int h)
{
    g_debug("qe_gtk_set_clip %d %d %d %d", x, y, w, h);
    struct QEGtkState *state = (struct QEGtkState *)s->priv_data;
    state->clip.x = x;
    state->clip.y = y;
    state->clip.width = w;
    state->clip.height = h;
}

static void qe_gtk_cursor_at(QEditScreen *s, int x1, int y1,
                         int w, int h)
{
    struct QEGtkState *state = (struct QEGtkState *)s->priv_data;
    state->cursor.x - x1;
    state->cursor.y - y1;
    state->cursor.width = w;
    state->cursor.height = h;
}

static int qe_gtk_bmp_alloc(QEditScreen *s, QEBitmap *b)
{
    return 0;
}

static void qe_gtk_bmp_free(QEditScreen *s, QEBitmap *b)
{
}

static void qe_gtk_full_screen(QEditScreen *s, int full_screen)
{
}

static int qe_gtk_probe(void)
{
    if (force_tty)
        return 0;
    g_debug("probed");
    return 2;
}

static QEDisplay qe_gtk_dpy = {
    "gtk",
    qe_gtk_probe,
    qe_gtk_init,
    qe_gtk_close,
    qe_gtk_flush,
    qe_gtk_is_user_input_pending,
    qe_gtk_fill_rectangle,
    qe_gtk_open_font,
    qe_gtk_close_font,
    qe_gtk_text_metrics,
    qe_gtk_draw_text,
    qe_gtk_set_clip,
    NULL, /* no selection handling */
    NULL, /* no selection handling */
    NULL, /* dpy_invalidate */
    qe_gtk_cursor_at,
    qe_gtk_bmp_alloc, /* dpy_bmp_alloc */
    qe_gtk_bmp_free, /* dpy_bmp_free */
    NULL, /* dpy_bmp_draw */
    NULL, /* dpy_bmp_lock */
    NULL, /* dpy_bmp_unlock */
    qe_gtk_full_screen,
    NULL, /* next */
};

static CmdOptionDef cmd_options[] = {
    { "no-windows", "nw", NULL, CMD_OPT_BOOL, "force tty terminal usage",
      { .int_ptr = &force_tty }},
    { "font-size", "fs", "ptsize", CMD_OPT_INT | CMD_OPT_ARG, "set default font size",
      { .int_ptr = &font_ptsize }},
    { NULL, NULL, NULL, 0, NULL, { NULL }},
};

static int qe_gtk_module_init(void)
{
    qe_register_cmd_line_options(cmd_options);
    return qe_register_display(&qe_gtk_dpy);
}

qe_module_init(qe_gtk_module_init);
