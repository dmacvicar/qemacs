/*
 * Unix main loop for QEmacs
 *
 * Copyright (c) 2002, 2003 Fabrice Bellard.
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

#include <functional>

#include <QObject>
#include <QEventLoop>
#include <QTimer>
#include <QSocketNotifier>
#include <QList>

extern "C" {
#include "qe.h"
}

typedef struct URLHandler {
    void *read_opaque;
    void (*read_cb)(void *opaque);
    void *write_opaque;
    void (*write_cb)(void *opaque);
} URLHandler;

typedef struct PidHandler {
    struct PidHandler *next, *prev;
    int pid;
    void (*cb)(void *opaque, int status);
    void *opaque;
} PidHandler;

typedef struct BottomHalfEntry {
    struct BottomHalfEntry *next, *prev;
    void (*cb)(void *opaque);
    void *opaque;
} BottomHalfEntry;

struct QETimer {
    QTimer *timer;
    struct QETimer *next;
};

static fd_set url_rfds, url_wfds;
static int url_fdmax;
static URLHandler url_handlers[256];
static int url_exit_request;
static LIST_HEAD(pid_handlers);
static LIST_HEAD(bottom_halves);
static QETimer *first_timer;

static QList<QETimer *> gTimers;
static QList<QSocketNotifier *> gSocketNotifiers;

static void set_handler(QSocketNotifier::Type type, int fd, void (*cb)(void *opaque), void *opaque)
{
    QSocketNotifier *notifier = new QSocketNotifier(fd, type);
        QObject::connect(notifier, &QSocketNotifier::activated,
                     std::bind(cb, opaque) );
}

void set_read_handler(int fd, void (*cb)(void *opaque), void *opaque)
{
    set_handler(QSocketNotifier::Read, fd, cb, opaque);
}

void set_write_handler(int fd, void (*cb)(void *opaque), void *opaque)
{
    set_handler(QSocketNotifier::Write, fd, cb, opaque);
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

static void _qt_timer_callback(QETimer *ti, void *opaque, void (*cb)(void *opaque))
{
    // delete the Qt timer
    delete ti->timer;
    qe_free(&ti);
    // execute the callback
    cb(opaque);
}

QETimer *qe_add_timer(int delay, void *opaque, void (*cb)(void *opaque))
{
    QETimer *ti;
    ti = qe_mallocz(QETimer);
    if (!ti)
        return NULL;

    QTimer *timer = new QTimer();
    QObject::connect(timer, &QTimer::timeout,
                     std::bind(_qt_timer_callback, ti, opaque, cb));

    timer->setSingleShot(true);
    timer->start(delay);
    ti->timer = timer;

    return ti;
}

void qe_kill_timer(QETimer **tip)
{
    if (*tip) {
        (*tip)->timer->stop();
        delete (*tip)->timer;
        qe_free(tip);
    }
}

void url_main_loop(void (*init)(void *opaque), void *opaque)
{
    init(opaque);
    QEventLoop loop;
    loop.exec();
}

/* exit from url loop */
void url_exit(void)
{
    QEventLoop loop;
    loop.exit();
}
