
#include <pthread.h>

#include <QAbstractScrollArea>
#include <QApplication>
#include <QThread>
#include <QPicture>

#ifndef QEMACS_QT_H
#define QEMACS_QT_H

class QEUIContext;

class QEView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    QEView(QEUIContext *ctx, QWidget *parent = 0);
    ~QEView();
    void keyPressEvent(QKeyEvent *);
protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void closeEvent (QCloseEvent *);
public slots:
    void slotDrawText(const QFont &, int, int, const QString &, const QColor &, bool);
    void slotFillRectangle(int, int, int, int, const QColor &, bool);
    void slotResize(const QSize &);
    void slotFlush();
    void slotSetClip(int, int, int, int);
private:
    QEUIContext *_ctx;
    // when we draw in the double buffer, we increase this
    int _repaints;
    QRect _clip;
};

class QEApplication : public QApplication
{
    Q_OBJECT
public:
    QEApplication(int &argc, char **argv);
private:

};

// this cant be a QObject. Told you.
class QEUIContext
{
public:
    // we can't use the constructor before the thread
    // is running and the QApplication created
    QEUIContext();

    void init();

    pthread_t uiThread;
    QEApplication *app;
    QFont font;
    QMainWindow *window;
    QEView *view;

    // qemacs hooks end painting here
    // and we replay on paintEvent
    QImage *image;

    int events_rd;
    int events_wr;

    void flush();
};


#endif
