#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    DataTimer(new QTimer(this))
{
    ui->setupUi(this);
    setupGraph();
    std::cout << "MainWindow Thread: " <<QThread::currentThread() << std::endl;
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

    std::cout << "setupGraph Thread: " <<QThread::currentThread() << std::endl;
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom); // 드래그, 줌 기능
}

void MainWindow::updateGraph(int objsize)
{
    std::cout << "updateGraph Thread: " <<QThread::currentThread() << std::endl;

    QMutexLocker locker(&mutex); // 동기화를 위한 mutex 사용
    extern int current_frame;
    int frameProcessed = current_frame;

    ui->customPlot->graph(0)->addData(frameProcessed, objsize); // objSize 대신 objsize 사용    
    ui->customPlot->xAxis->setRange(frameProcessed, 100, Qt::AlignRight); // x축 0~100    
    ui->customPlot->xAxis->moveRange(1);    
    ui->customPlot->replot();

}

void MainWindow::showFrame(cv::Mat &frame, cv::Mat &subframe) {

    std::cout << "showFrame Thread: " <<QThread::currentThread() << std::endl;

    if(frame.empty() && subframe.empty())
    {
        std::cout << "showFrame 연결 X" << std::endl;
        return;
    }
    // Mat to QImage
    QImage img1 = QImage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_BGR888);
    QImage img2(subframe.data, subframe.cols, subframe.rows, subframe.step, QImage::Format_Grayscale8);

    // QImage to QPixmap
    QPixmap pixmap1 = QPixmap::fromImage(img1);
    QPixmap pixmap2 = QPixmap::fromImage(img2);

    ui->screen_frame->setPixmap(pixmap1);
    ui->screen_frame->show();

    ui->screen_subframe->setPixmap(pixmap2);
    ui->screen_subframe->show();

}

//void MainWindow::on_pushButton_Start_clicked()
//{
//    // 다른 쓰레드 videoThread, ProcessingThread 재시작
//}

//void MainWindow::on_pushButton_Stop_clicked()
//{
//    // 다른 쓰레드 videoThread, ProcessingThread 중지
//}

MainWindow::~MainWindow()
{
    delete ui;
}
