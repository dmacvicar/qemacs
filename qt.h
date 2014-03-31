
#include <QAbstractScrollArea>
#include <QApplication>
#include <QThread>

#ifndef QEMACS_QT_H
#define QEMACS_QT_H

class WindowState;

class QEView : public QAbstractScrollArea
{
    Q_OBJECT
public:
    QEView(QWidget *parent = 0);
    ~QEView();
};

class QEApplication : public QApplication
{
    Q_OBJECT
public:
    QEApplication(int &argc, char **argv);

private:
    QMainWindow *_window;
    QEView *_view;
};


class QEUIThread : public QThread
{
public:
    QEUIThread(WindowState *ctx);

    virtual void run();
private:
    WindowState *_ctx;
};

#endif
