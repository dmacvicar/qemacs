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
#include <QMap>

extern "C" {
#include "qe.h"
}

struct QETimer {
    QTimer *timer;
};

static QMap<int, QSocketNotifier *> gReadNotifiers;
static QMap<int, QSocketNotifier *> gWriteNotifiers;

void set_read_handler(int fd, void (*cb)(void *opaque), void *opaque)
{
    if (gReadNotifiers.contains(fd)) {
        QSocketNotifier *notifier = gReadNotifiers.take(fd);
        notifier->setEnabled(false);
        delete notifier;
    }

    if (cb == nullptr) {
        return;
    }

    QSocketNotifier *notifier = new QSocketNotifier(fd, QSocketNotifier::Read);
    QObject::connect(notifier, &QSocketNotifier::activated,
                     std::bind(cb, opaque) );
    gReadNotifiers.insert(fd, notifier);
}

void set_write_handler(int fd, void (*cb)(void *opaque), void *opaque)
{
    if (gWriteNotifiers.contains(fd)) {
        QSocketNotifier *notifier = gWriteNotifiers.take(fd);
        notifier->setEnabled(false);
        delete notifier;
    }

    if (cb == nullptr) {
        return;
    }

    QSocketNotifier *notifier = new QSocketNotifier(fd, QSocketNotifier::Write);
    QObject::connect(notifier, &QSocketNotifier::activated,
                     std::bind(cb, opaque) );
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

    foreach (QSocketNotifier *notifier, gReadNotifiers) {
        delete notifier;
    }
    foreach (QSocketNotifier *notifier, gWriteNotifiers) {
        delete notifier;
    }
}
