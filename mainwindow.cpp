#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"

QQueue<int> queue; // 큐 선언

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    DataTimer(new QTimer(this))
{
    ui->setupUi(this);
    setupGraph();
}

void MainWindow::setupGraph()
{
    // 그래프 설정
    ui->customPlot->addGraph();
    ui->customPlot->xAxis->setLabel("Fps");
    ui->customPlot->yAxis->setLabel("Object Size");
    ui->customPlot->xAxis->setRange(0, 100); // x축 범위
    ui->customPlot->yAxis->setRange(0, 20000); // Y축 범위
    ui->customPlot->axisRect()->setupFullAxesBox();

    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom); // 드래그, 줌 기능

    // 그래프 업데이트
    connect(DataTimer, SIGNAL(timeout()), this, SLOT(updateGraph()));
    DataTimer->start(100); // 0.1초마다 호출
}

void MainWindow::updateGraph()
{
    if(queue.isEmpty())
        std::cout << "queue is empty by mainwindow.cpp" << std::endl;
    else{
    // 진행중인 프레임, 객체 사이즈
    int frameProcessed = queue.dequeue();
    int objectSize = queue.dequeue();

    ui->customPlot->graph(0)->addData(frameProcessed, objectSize);
    ui->customPlot->xAxis->setRange(frameProcessed, 100, Qt::AlignRight); // x축 0~100
    ui->customPlot->xAxis->moveRange(1);
    ui->customPlot->replot();
    }
}

void MainWindow::on_pushButton_Start_clicked()
{
    DataTimer->start(100);
}

void MainWindow::on_pushButton_Stop_clicked()
{
    DataTimer->stop();
}


MainWindow::~MainWindow()
{
    delete ui;
    delete DataTimer;
    // delete file;
    delete in;
}
