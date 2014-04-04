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

class QEView;
class QEWindow;
class QEApplication;
class QEUIThread;

QEView::QEView(QEUIContext *ctx, QWidget *parent)
    : QAbstractScrollArea(parent),
      _ctx(ctx)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
}

QEView::~QEView()
{
}

void QEView::keyPressEvent (QKeyEvent * event)
{
    qDebug() << Q_FUNC_INFO;
    _ctx->app->sendEvent(_ctx->receiver, event);
}

void QEView::slotResize(const QSize &size)
{
    qDebug() << Q_FUNC_INFO << size;
    resize(size);
}

void QEView::slotDrawText(const QFont &font, int x, int y, const QString &text, const QColor &color)
{
    qDebug() << Q_FUNC_INFO;
    QPainter painter(_ctx->picture);
    painter.setPen(color);
    painter.drawText(x, y, text);
}

void QEView::slotFillRectangle(int x, int y, int w, int h, const QColor &color)
{
    qDebug() << Q_FUNC_INFO;
    QPainter painter(_ctx->picture);
    painter.fillRect(x, y, w, h, color);
}

void QEView::paintEvent(QPaintEvent *event)
{
    qDebug() << Q_FUNC_INFO;
    QPainter painter(viewport());
    //painter.drawEllipse(2, 2, 30, 30);
    //_ctx->picture->play(&painter);
    painter.drawPicture(0, 0, *_ctx->picture);
    //QPicture empty;
    //_ctx->picture->swap(empty);
}

QEApplication::QEApplication(int &argc, char **argv)
        : QApplication(argc, argv)
{
}

bool QEUIEventReceiver::event(QEvent *event)
{
    qDebug() << Q_FUNC_INFO;
    return true;
}

void *qt_thread(void *userdata) {
    QEUIContext *ctx = (QEUIContext *) userdata;
    int argc = 0;
    char *argv[] = {};
    QEApplication app(argc, argv);
    ctx->app = &app;
    qDebug() << "app created";
    //ctx->resize(QSize(xsize, ysize));
    ctx->init();
    return (void * ) ctx->app->exec();
}

static int qt_probe(void)
{
    if (force_tty)
        return 0;
    return 2;
}

void QEUIContext::resize(const QSize &size)
{
    QMetaObject::invokeMethod(view, "slotResize", Qt::QueuedConnection, Q_ARG(QSize, size));
}

void QEUIContext::drawText(const QFont &font, int x, int y, const QString &text, const QColor &color)
{
    QMetaObject::invokeMethod(view,
                              "slotDrawText",
                              Qt::QueuedConnection,
                              Q_ARG(QFont, font),
                              Q_ARG(int, x),
                              Q_ARG(int, y),
                              Q_ARG(QString, text),
                              Q_ARG(QColor, color));
}

void QEUIContext::fillRectangle(int x, int y, int w, int h, const QColor &color)
{
    QMetaObject::invokeMethod(view,
                              "slotFillRectangle",
                              Qt::QueuedConnection,
                              Q_ARG(int, x),
                              Q_ARG(int, y),
                              Q_ARG(int, w),
                              Q_ARG(int, h),
                              Q_ARG(QColor, color));
}

QEUIContext::QEUIContext()
{
    app = 0L;
}

void QEUIContext::init()
{
    Q_ASSERT(app);
    view = new QEView(this);
    picture = new QPicture();
    view->show();
}

static int qt_init(QEditScreen *s, int w, int h)
{
    int xsize, ysize, font_ysize;
    QEUIContext *ctx;
    // here init the application
    //ctx = qe_mallocz(WindowState);
    ctx = new QEUIContext();

    if (ctx == NULL) {
        return -1;
    }

    s->priv_data = ctx;
    s->media = CSS_MEDIA_SCREEN;
    s->bitmap_format = QEBITMAP_FORMAT_RGBA32;

    pthread_create(&ctx->uiThread, NULL, qt_thread, ctx);

    while (!ctx->app) {
        qDebug() << "wait for UI thread...";
    }

    ctx->receiver = new QEUIEventReceiver();

    ctx->font = QFont("Sans");
    QFontMetrics fm(ctx->font);
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

    ctx->resize(QSize(600, 600));
    return 2;
}

static void qt_close(QEditScreen *s)
{
    Q_UNUSED(s);
    qDebug();
}

static void qt_flush(QEditScreen *s)
{
    Q_UNUSED(s);
    qDebug() << Q_FUNC_INFO;

}

static int qt_is_user_input_pending(QEditScreen *s)
{
    Q_UNUSED(s);
    qDebug() << Q_FUNC_INFO;

    return 0;
}

static void qt_fill_rectangle(QEditScreen *s,
                              int x1, int y1, int w, int h, QEColor color)
{
    qDebug() << Q_FUNC_INFO << x1 << y1 << w << h;
    QEUIContext *ctx = (QEUIContext *)s->priv_data;
    ctx->fillRectangle(x1, y1, w, h, QColor::fromRgba(color));
}

static QEFont *qt_open_font(QEditScreen *s, int style, int size)
{
     qDebug() << Q_FUNC_INFO;

    QEUIContext *ctx = (QEUIContext *)s->priv_data;
    QEFont *font;

    font = qe_mallocz(QEFont);
    if (!font)
        return NULL;

    QFont *f;

    switch (style & QE_FAMILY_MASK) {
    default:
    case QE_FAMILY_FIXED:
        f = new QFont();
        break;
    case QE_FAMILY_SANS:
    case QE_FAMILY_SERIF:
        f = new QFont("Serif");
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

    QFontMetrics fm(*f);
    font->ascent = fm.ascent();
    font->descent = fm.descent();
    font->priv_data = f;
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
        QFontMetrics fm(*f);
        metrics->font_ascent = fm.ascent();
        metrics->font_descent = fm.descent();
        metrics->width = fm.width(QString::fromUcs4(str, len));
    }
}

static void qt_draw_text(QEditScreen *s, QEFont *font,
                         int x1, int y, const unsigned int *str, int len,
                         QEColor color)
{
    qDebug() << Q_FUNC_INFO;
    QEUIContext *ctx = (QEUIContext *)s->priv_data;
    QFont *f = (QFont *)font->priv_data;
    QString text = QString::fromUcs4(str, len);
    ctx->drawText(*f, x1, y, text, QColor::fromRgba(color));
}

static void qt_set_clip(QEditScreen *s,
                        int x, int y, int w, int h)
{
    Q_UNUSED(s);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(w);
    Q_UNUSED(h);
    qDebug() << Q_FUNC_INFO;
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
    NULL, /* dpy_cursor_at */
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
