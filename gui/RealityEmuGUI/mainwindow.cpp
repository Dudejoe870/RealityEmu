#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingswindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(settingsPressed()));

    MainWindow::sw = new SettingsWindow();
}

MainWindow::~MainWindow()
{
    delete MainWindow::sw;
    delete ui;
}

void MainWindow::settingsPressed()
{
    MainWindow::sw->show();
}
