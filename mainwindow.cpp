#include "mainwindow.h"
#include "ui_mainwindow.h"

#ifdef None
#undef None
#endif

#define None 0x00

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialize( void )
{
    try
    {
    	ui->glwidget->initialize();
        this->adjustSize();
    }
    catch (exception& e)
    {
    	std::cout << e.what() << '\n';
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    
}