#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <iostream>
#include <QQueue>
#include <QThread>
#include <QMutex> // 스레드 동기화
#include <opencv2/opencv.hpp>

class QTimer;

static QMutex mutex;
static QWaitCondition waitCondition;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updateGraph(int objsize);  // 그래프 업데이트
    void showFrame(cv::Mat &frame, cv::Mat &subframe); // 화면 출력

private slots:
    void on_pushButton_Start_clicked();
    void on_pushButton_Stop_clicked();

private:
    Ui::MainWindow *ui;
    QTimer* DataTimer;
    void setupGraph();
};

// 영상읽기 전용 쓰레드
// QThread에서 상속받아 사용
class VideoThread: public QThread
{
    Q_OBJECT
signals:
    void Frame_Ready(cv::Mat &frame);        // signal을 선언하고 emit를 통해 시그널 발생
private:
    // start() 호출 전까지는 시작 X, start() 호출 시 run()함수 동작
    QMutex mutex;
    cv::Mat frame;
    bool video_stop;
    bool video_pause;
    QWaitCondition video_pauseCondition;


public:
    void pause();
    void resume();
    void stop();
    void run();
};

// 영상처리 전용 쓰레드
class ProcessingThread: public QThread
{
    Q_OBJECT
//protected:
//    void run() override;
public slots:
    void setFrame( cv::Mat &frame);
//    void ProcessFrame(cv::Mat &frame);
signals:
    void ProcessingResult(cv::Mat &frame, cv::Mat &subframe);
    void ObjSizeResult(int objsize);
private:
    bool processing_stop;
    bool is_playing = true;
    cv::Mat Temp_frame;
public:
    void run();
};

#endif // MAINWINDOW_H
