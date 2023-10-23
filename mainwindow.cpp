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
    connect(DataTimer, SIGNAL(timeout()), this, SLOT(updateGraph(int)));
    DataTimer->start(100); // 0.1초마다 호출
}

void MainWindow::updateGraph(int objsize)
{
    extern int current_frame;
    int frameProcessed = current_frame;

    ui->customPlot->graph(0)->addData(frameProcessed, objsize); // objSize 대신 objsize 사용
    ui->customPlot->xAxis->setRange(frameProcessed, 100, Qt::AlignRight); // x축 0~100
    ui->customPlot->xAxis->moveRange(1);
    ui->customPlot->replot();
}

void MainWindow::showFrame(cv::Mat &frame, cv::Mat &subframe) {
    if(frame.empty() && subframe.empty())
    {
        std::cout << "showFrame 연결 X" << std::endl;
        return;
    }
    // Mat 이미지를 QImage로 변환
    QImage img1 = QImage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_BGR888);
    QImage img2(subframe.data, subframe.cols, subframe.rows, subframe.step, QImage::Format_Grayscale8);

    // QImage를 QPixmap으로 변환 (이 부분에서 별도의 QPixmap 버퍼가 필요하지 않습니다)
    QPixmap pixmap1 = QPixmap::fromImage(img1);
    QPixmap pixmap2 = QPixmap::fromImage(img2);

    ui->screen_frame->setPixmap(pixmap1);
    ui->screen_frame->show();

    ui->screen_subframe->setPixmap(pixmap2);
    ui->screen_subframe->show();

}

void MainWindow::on_pushButton_Start_clicked()
{
    // DataTimer->start(100);   // data가 시간일 때때
    is_playing = true;
}

void MainWindow::on_pushButton_Stop_clicked()
{
    // DataTimer->stop();
    is_playing = false;
}

MainWindow::~MainWindow()
{
    delete ui;
    delete DataTimer;
}
