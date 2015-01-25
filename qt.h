
#include <pthread.h>

#include <QMainWindow>
#include <QAbstractScrollArea>
#include <QApplication>
#include <QThread>
#include <QPicture>

#ifndef QEMACS_QT_H
#define QEMACS_QT_H

class QEQtContext;

class QEQtView : public QWidget
{
    Q_OBJECT
public:
    QEQtView(QEQtContext *ctx, QWidget *parent = 0);
    ~QEQtView();
protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void closeEvent (QCloseEvent *);
    virtual void resizeEvent(QResizeEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);

    void mouseEvent(QMouseEvent *);

public slots:
    void slotDrawText(const QFont &, int, int, const QString &, const QColor &, bool);
    void slotFillRectangle(int, int, int, int, const QColor &, bool);
    void slotResizeDoubleBuffer(const QSize &);
    void slotFlush();
    void slotSetClip(int, int, int, int);
    void slotSetCursor(int, int, int, int);
private:
    QEQtContext *_ctx;
    // when we draw in the double buffer, we increase this
    int _repaints;
    QRect _clip;
    QRect _cursor;
};

class QEQtApplication : public QApplication
{
    Q_OBJECT
public:
    QEQtApplication();
private:

};

// opaque pointer where Qt keeps its application and
// views
class QEQtContext
{
public:
    QEQtContext();

    QEQtApplication *app;
    QEQtView *view;

    QFont font;
    QMainWindow *window;
    // qemacs hooks end painting here
    // and we replay on paintEvent
    QImage image;

    int events_rd;
    int events_wr;

    void flush();
};
#endif

