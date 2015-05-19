#ifndef __MAINWINDOW__
#define __MAINWINDOW__

#include <GL/glew.h>

//#include <QtGui/QtGui>
#include <QMainWindow>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>
#include <QDebug>
#include <QDoubleSpinBox>

#include "glwidget.hpp"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void initialize( void );

public slots:

private:
    Ui::MainWindow *ui;

protected:
    void resizeEvent(QResizeEvent *event);

};

#endif // MAINWINDOW
