/*
 * Qt driver for QEmacs
 * Copyright (c) 2014 Duncan Mac-Vicar P.
 * Copyright (c) 2013 Francois Revol.
 * Copyright (c) 2002 Fabrice Bellard.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */

#ifdef __cplusplus
extern "C" {
#endif
#include "qe.h"
#ifdef __cplusplus
}
#endif

#include <iostream>

#include <QDebug>
#include <QObject>
#include <QMainWindow>
#include <QFont>
#include <QFontMetrics>
#include <QBitmap>
#include <QPainter>
#include <QKeyEvent>

#include "qt.h"

static int force_tty = 0;
static int font_xsize;
static int font_ptsize;

class QEView;
class QEWindow;
class QEApplication;
class QEUIThread;

QEView::QEView(QEUIContext *ctx, QWidget *parent)
    : QWidget(parent),
      _ctx(ctx),
      _repaints(0)
{
//setAttribute(Qt::WA_OpaquePaintEvent);
}

QEView::~QEView()
{
}

void QEView::keyPressEvent (QKeyEvent * event)
{
    qDebug() << Q_FUNC_INFO;

    QEKeyEvent ev;
    ev.type = QE_KEY_EVENT;

    bool ctrl = event->modifiers() && Qt::ControlModifier;
    bool shift = event->modifiers() && Qt::ShiftModifier;
    bool meta = event->modifiers() && Qt::MetaModifier;

    switch (event->key()) {
    case Qt::Key_Control:
    case Qt::Key_Meta:
      ev.key = KEY_DEFAULT;
      break;
    // in the same order as qe.h
    case Qt::Key_Tab:
        ev.key = shift ? KEY_SHIFT_TAB : KEY_TAB;
        break;
    case Qt::Key_Return:
        ev.key = KEY_RET;
        break;
    case Qt::Key_Escape:
        ev.key = KEY_ESC;
        break;
    case Qt::Key_Space:
        ev.key = KEY_SPC;
        break;
    //case Qt::Key_????:
        //ev.key = KEY_DEL;
        //break;
    case Qt::Key_Backspace:
        ev.key = meta ? KEY_META(KEY_DEL) : KEY_DEL;
        break;
    case Qt::Key_Up:
        ev.key = ctrl ? KEY_CTRL_UP : KEY_UP;
        break;
    case Qt::Key_Down:
        ev.key = ctrl ? KEY_CTRL_DOWN : KEY_DOWN;
        break;
    case Qt::Key_Right:
        ev.key = ctrl ? KEY_CTRL_RIGHT: KEY_RIGHT;
        break;
    case Qt::Key_Left:
        ev.key = ctrl ? KEY_CTRL_LEFT : KEY_LEFT;
        break;
    case Qt::Key_End:
        ev.key = ctrl ? KEY_CTRL_END : KEY_END;
        break;
   case Qt::Key_Home:
        ev.key = ctrl ? KEY_CTRL_HOME : KEY_HOME;
        break;
   case Qt::Key_PageUp:
        ev.key = ctrl ? KEY_CTRL_PAGEUP : KEY_PAGEUP;
        break;
   case Qt::Key_PageDown:
        ev.key = ctrl ? KEY_CTRL_PAGEDOWN : KEY_PAGEDOWN;
        break;
   case Qt::Key_Insert:
        ev.key = KEY_INSERT;
        break;
   case Qt::Key_Delete:
        ev.key = KEY_DELETE;
        break;
   case Qt::Key_F1:
        ev.key = KEY_F1;
        break;
   case Qt::Key_F2:
        ev.key = KEY_F2;
        break;
   case Qt::Key_F3:
        ev.key = KEY_F3;
        break;
   case Qt::Key_F4:
        ev.key = KEY_F4;
        break;
   case Qt::Key_F5:
        ev.key = KEY_F5;
        break;
   case Qt::Key_F6:
        ev.key = KEY_F6;
        break;
   case Qt::Key_F7:
        ev.key = KEY_F7;
        break;
   case Qt::Key_F8:
        ev.key = KEY_F8;
        break;
   case Qt::Key_F9:
        ev.key = KEY_F9;
        break;
   case Qt::Key_F10:
        ev.key = KEY_F10;
        break;
   case Qt::Key_F11:
        ev.key = KEY_F11;
        break;
   case Qt::Key_F12:
        ev.key = KEY_F12;
        break;
   case Qt::Key_F13:
        ev.key = KEY_F13;
        break;
   case Qt::Key_F14:
        ev.key = KEY_F14;
        break;
   case Qt::Key_F15:
        ev.key = KEY_F15;
        break;
   case Qt::Key_F16:
        ev.key = KEY_F16;
        break;
   case Qt::Key_F17:
        ev.key = KEY_F17;
        break;
   case Qt::Key_F18:
        ev.key = KEY_F18;
        break;
   case Qt::Key_F19:
        ev.key = KEY_F19;
        break;
   case Qt::Key_F20:
        ev.key = KEY_F20;
        break;
    default:
        if (event->text().isEmpty()) {
            qDebug() << Q_FUNC_INFO << "empty key" << event->nativeScanCode();
            return;
        }

        ev.key = event->text().at(0).toLatin1();
        qDebug() << Q_FUNC_INFO << " other key" << event->nativeScanCode();
    }

    write(_ctx->events_wr, &ev, sizeof(QEEvent));
}

void QEView::slotResize(const QSize &size)
{
    qDebug() << Q_FUNC_INFO << updatesEnabled()<< size;
    QImage tmp(size, QImage::Format_ARGB32);
    _ctx->image.swap(tmp);
    resize(size);
    // update all the widget in future repaint
    _clip.setRect(0, 0, size.width(), size.height());
}

void QEView::slotSetCursor(int x, int y, int w, int h)
{
    _cursor.setRect(x, y, w, h);
}

void QEView::slotDrawText(const QFont &font, int x, int y, const QString &text, const QColor &color, bool xorMode)
{
    QPainter painter(&_ctx->image);
    painter.setPen(color);

    if (xorMode) {
        painter.setCompositionMode(QPainter::CompositionMode_Xor);
    }

    painter.drawText(x, y, text);
    _repaints++;
}

void QEView::slotFillRectangle(int x, int y, int w, int h, const QColor &color, bool xorMode)
{
    qDebug() << Q_FUNC_INFO;
    QPainter painter(&_ctx->image);
    if (xorMode) {
        painter.setCompositionMode(QPainter::CompositionMode_Xor);
    }
    else
    painter.fillRect(x, y, w, h, color);
    _repaints++;
}

void QEView::slotFlush()
{
    qDebug() << Q_FUNC_INFO << "updates enabled: " << updatesEnabled();

    //Q_ASSERT(!_clip.isEmpty());
    //qDebug() << Q_FUNC_INFO << rect() << _clip;
    //Q_ASSERT(!rect().intersected(_clip).isEmpty());

    /// XXX clipping disabled for now
    //update(_clip);
    update(rect());

    _ctx->app->processEvents();
}

void QEView::slotSetClip(int x, int y, int w, int h)
{
    qDebug() << Q_FUNC_INFO << x << y << w << h;
    _clip.setRect(x, y, w, h);
}

void QEView::paintEvent(QPaintEvent *event)
{
    qDebug() << Q_FUNC_INFO << event->rect();
    QPainter painter(this);
    painter.drawImage(event->rect().x(),
                      event->rect().y(),
                      _ctx->image,
                      event->rect().x(),
                      event->rect().y(),
                      event->rect().x() + event->rect().width(),
                      event->rect().y() + event->rect().height());

    QImage cursorImg = _ctx->image.copy(_cursor);
    cursorImg.invertPixels();
    painter.drawImage(QPoint(_cursor.x(), _cursor.y()), cursorImg);
}

void QEView::closeEvent(QCloseEvent * event)
{
    QEEvent ev;
    // cancel pending operation
    ev.key_event.type = QE_KEY_EVENT;
    ev.key_event.key = KEY_CTRL('g');
    write(_ctx->events_wr, &ev, sizeof(QEEvent));

    // simulate C-x C-c
    ev.key_event.type = QE_KEY_EVENT;
    ev.key_event.key = KEY_CTRL('x');
    write(_ctx->events_wr, &ev, sizeof(QEEvent));

    ev.key_event.type = QE_KEY_EVENT;
    ev.key_event.key = KEY_CTRL('c');
    write(_ctx->events_wr, &ev, sizeof(QEEvent));
}


QEApplication::QEApplication(int &argc, char **argv)
        : QApplication(argc, argv)
{
}

void *qt_thread(void *userdata) {
    QEUIContext *ctx = (QEUIContext *) userdata;
    int argc = 0;
    char *argv[] = {};
    QEApplication app(argc, argv);

    ctx->app = &app;
    qDebug() << "app created";
    ctx->init();
    //ctx->resize(QSize(xsize, ysize));
    return (void * ) ctx->app->exec();
}

static int qt_probe(void)
{
    if (force_tty)
        return 0;
    return 2;
}

QEUIContext::QEUIContext()
{
    app = 0L;
    view = 0L;
}

void QEUIContext::init()
{
    view = new QEView(this);
    view->show();
}

static void qt_handle_event(void *opaque);

static void _qt_process_events_timer(void *opaque)
{
    qe_add_timer(0, opaque, _qt_process_events_timer);
    QApplication *app = (QApplication *)(opaque);
    app->processEvents(QEventLoop::WaitForMoreEvents);
}

static QEFont *qt_open_font(QEditScreen *s, int style, int size);

static int qt_init(QEditScreen *s, int w, int h)
{
    //int argc = 0;
    //char *argv[] = {};
    QEFont *font;
    QEStyleDef default_style;

    //QEApplication *app = new QEApplication(argc, argv);
    QEUIContext *ctx;
    // here init the application
    //ctx = qe_mallocz(WindowState);
    ctx = new QEUIContext();

    if (ctx == NULL) {
        return -1;
    }

    pthread_create(&ctx->uiThread, NULL, qt_thread, ctx);
    while (!ctx->app) {
        qDebug() << "wait for UI thread...";
    }

    while (!ctx->view) {
        qDebug() << "wait for view in thread...";
    }

    int xsize, ysize, font_ysize;

    s->priv_data = ctx;
    s->media = CSS_MEDIA_SCREEN;
    s->bitmap_format = QEBITMAP_FORMAT_RGBA32;

    int event_pipe[2];
    if (pipe(event_pipe) < 0)
        return -1;

    fcntl(event_pipe[0], F_SETFD, FD_CLOEXEC);
    fcntl(event_pipe[1], F_SETFD, FD_CLOEXEC);

    ctx->events_rd = event_pipe[0];
    ctx->events_wr  = event_pipe[1];
    set_read_handler(event_pipe[0], qt_handle_event, s);

    //ctx->app = app;
    //qDebug() << "app created";

    /* At this point, we should be able to ask for metrics */
    if (font_ptsize)
        qe_styles[0].font_size = font_ptsize;
    get_style(NULL, &default_style, 0);
    font = qt_open_font(s, default_style.font_style,
                          default_style.font_size);

    if (!font) {
        fprintf(stderr, "Could not open default font\n");
        exit(1);
    }

    QFont *qt_font = (QFont *)font->priv_data;
    QFontMetrics fm(*qt_font);

    font_xsize = fm.width("n");
    font_ysize = fm.height();

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

    // initialize the double buffer
    QSize size(xsize, ysize);
    QImage tmp(size, QImage::Format_ARGB32);
    ctx->image.swap(tmp);

    ctx->view->slotResize(size);
    ctx->app->processEvents();

    //qe_add_timer(0, ctx->app, _qt_process_events_timer);
    return 2;
}

static void qt_close(QEditScreen *s)
{
    Q_UNUSED(s);
    qDebug();
    QEUIContext *ctx = (QEUIContext *)s->priv_data;
    Q_UNUSED(ctx);
}

static void qt_flush(QEditScreen *s)
{
    qDebug() << Q_FUNC_INFO;
    QEUIContext *ctx = (QEUIContext *)s->priv_data;
    ctx->view->slotFlush();

}

static int qt_is_user_input_pending(QEditScreen *s)
{
    Q_UNUSED(s);
    qDebug() << Q_FUNC_INFO;

    return 0;
}

static void qt_handle_event(void *opaque)
{
    qDebug() << Q_FUNC_INFO;

    QEditScreen *s = (QEditScreen *)opaque;
    QEUIContext *ctx = (QEUIContext *)s->priv_data;

    QEEvent ev;
    if (read(ctx->events_rd, &ev, sizeof(ev)) < (signed)sizeof(ev))
        return;

    qe_handle_event(&ev);
}

static void qt_fill_rectangle(QEditScreen *s,
                              int x1, int y1, int w, int h, QEColor color)
{
    qDebug() << Q_FUNC_INFO << x1 << y1 << w << h;
    QEUIContext *ctx = (QEUIContext *)s->priv_data;

    bool xorMode = (color == QECOLOR_XOR);

    ctx->view->slotFillRectangle(x1, y1, w, h,
                                 QColor::fromRgba(color), xorMode);

}

static QEFont *qt_open_font(QEditScreen *s, int style, int size)
{
    qDebug() << Q_FUNC_INFO << style << size;

    QEUIContext *ctx = (QEUIContext *)s->priv_data;
    QEFont *font;

    font = qe_mallocz(QEFont);
    if (!font)
        return NULL;

    QFont *f = new QFont();
    f->setPointSize(size);
    f->setStyleStrategy(QFont::ForceIntegerMetrics);

    switch (style & QE_FAMILY_MASK) {
    default:
    case QE_FAMILY_FIXED:
        qDebug() << Q_FUNC_INFO << "Monospace font requested";
        f->setFamily("DejaVu Sans Mono");
        f->setStyleHint(QFont::TypeWriter);
        f->setFixedPitch(true);
        break;
    case QE_FAMILY_SANS:
        qDebug() << Q_FUNC_INFO << "Sans font requested";
        f->setFamily("DejaVu Sans");
        f->setStyleHint(QFont::SansSerif);
        break;
    case QE_FAMILY_SERIF:
        qDebug() << Q_FUNC_INFO << "Serif font requested";
        f->setFamily("DejaVu Serif");
        f->setStyleHint(QFont::Serif);
        break;
    }

    if (style & QE_STYLE_BOLD)
        f->setBold(true);
    if (style & QE_STYLE_ITALIC)
        f->setItalic(true);
    if (style & QE_STYLE_UNDERLINE)
        f->setUnderline(true);
    if (style & QE_STYLE_LINE_THROUGH)
        f->setStrikeOut(true);

    QFontMetrics fm(*f, &ctx->image);
    font->ascent = fm.ascent();
    font->descent = fm.descent();

    font->priv_data = f;
    QFontInfo fi(*f);
    qDebug() << Q_FUNC_INFO << "Resolved" << fi.family() << fi.pointSize() << fi.fixedPitch();
    return font;
}

static void qt_close_font(QEditScreen *s, QEFont **fontp)
{
    QEFont *font = *fontp;

    if (font) {
        QFont *f = (QFont *)font->priv_data;
        delete f;
        /* Clear structure to force crash if font is still used after
         * close_font.
         */
        memset(font, 0, sizeof(*font));
        qe_free(fontp);
    }
}

static void qt_text_metrics(QEditScreen *s, QEFont *font, 
                               QECharMetrics *metrics,
                               const unsigned int *str, int len)
{
    if (font) {
        QFont *f = (QFont *)font->priv_data;
        QEUIContext *ctx = (QEUIContext *)s->priv_data;

        QString text = QString::fromUcs4(str, len);
        QPainter painter(&ctx->image);
        QRectF picRectF(ctx->image.rect());
        QRectF rect = painter.boundingRect(picRectF,
                                           Qt::TextDontClip, text);

        metrics->font_ascent = font->ascent;
        metrics->font_descent = font->descent;
        metrics->width = rect.width();

        //qDebug() << Q_FUNC_INFO << "w: " << rect.width() << text;
    }
}

static void qt_draw_text(QEditScreen *s, QEFont *font,
                         int x1, int y, const unsigned int *str, int len,
                         QEColor color)
{
    QEUIContext *ctx = (QEUIContext *)s->priv_data;
    QFont *f = (QFont *)font->priv_data;
    QString text = QString::fromUcs4(str, len);

    QFontInfo fontInfo(*f);
    //qDebug() << Q_FUNC_INFO << "Draw: " << fontInfo.family() << fontInfo.pointSize();

    bool xorMode = (color == QECOLOR_XOR);

    ctx->view->slotDrawText(*f, x1, y, text,
                            QColor::fromRgba(color), xorMode);
}

static void qt_set_clip(QEditScreen *s,
                        int x, int y, int w, int h)
{
    qDebug() << Q_FUNC_INFO << x << y << w << h;
    QEUIContext *ctx = (QEUIContext *)s->priv_data;
    ctx->view->slotSetClip(x, y, w, h);
}

static void qt_cursor_at(QEditScreen *s, int x1, int y1,
                         int w, int h)
{
    qDebug() << Q_FUNC_INFO << x1 << y1 << w << h;
    QEUIContext *ctx = (QEUIContext *)s->priv_data;
    ctx->view->slotSetCursor(x1, y1, w, h);
}

static int qt_bmp_alloc(QEditScreen *s, QEBitmap *b)
{
    Q_UNUSED(s);
    Q_UNUSED(b);
    qDebug() << Q_FUNC_INFO;
    return 0;
}

static void qt_bmp_free(QEditScreen *s, QEBitmap *b)
{
    Q_UNUSED(s);
    Q_UNUSED(b);
    qDebug() << Q_FUNC_INFO;
    b->priv_data = NULL;
}

static void qt_full_screen(QEditScreen *s, int full_screen)
{
    qDebug() << Q_FUNC_INFO;
    QEUIContext *ctx = (QEUIContext *)s->priv_data;
}

static QEDisplay qt_dpy = {
    "qt",
    qt_probe,
    qt_init,
    qt_close,
    qt_flush,
    qt_is_user_input_pending,
    qt_fill_rectangle,
    qt_open_font,
    qt_close_font,
    qt_text_metrics,
    qt_draw_text,
    qt_set_clip,
    NULL, /* no selection handling */
    NULL, /* no selection handling */
    NULL, /* dpy_invalidate */
    qt_cursor_at,
    qt_bmp_alloc, /* dpy_bmp_alloc */
    qt_bmp_free, /* dpy_bmp_free */
    NULL, /* dpy_bmp_draw */
    NULL, /* dpy_bmp_lock */
    NULL, /* dpy_bmp_unlock */
    qt_full_screen,
    NULL, /* next */
};

static CmdOptionDef cmd_options[] = {
    { "no-windows", "nw", NULL, CMD_OPT_BOOL, "force tty terminal usage",
       { int_ptr: &force_tty }},
    { "font-size", "fs", "ptsize", CMD_OPT_INT | CMD_OPT_ARG, "set default font size",
      { .int_ptr = &font_ptsize }},
    { NULL, NULL, NULL, 0, NULL, { NULL }},
};

static int qt_init(void)
{
    QEmacsState *qs = &qe_state;

    qe_register_cmd_line_options(cmd_options);
    return qe_register_display(&qt_dpy);
}

qe_module_init(qt_init);

#include "qt.moc.cpp"
