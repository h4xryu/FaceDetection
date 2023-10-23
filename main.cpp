#include "mainwindow.h"
#include <QApplication>
#include <QString>
#include <QDebug>

//cv::Mat frame;
//cv::Mat subframe;
// 예제 비디오 파일
// cv::VideoCapture cap("/home/cdm/Desktop/qt/qt_test/RealTimeGraph/vtest.avi");
//// 웹캠
//cv::VideoCapture cap(0, cv::CAP_V4L2);

// MOG2 알고리즘
cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2;

// 현재 프레임
int current_frame = 0;

// 객체 벡터
std::vector<cv::Rect> detected_objects;

// fgmask(subframe) = 전경 마스크 영상
int detectAndDrawObjects(cv::Mat& subframe, cv::Mat& frame) {
    // 각 프레임의 객체 사이즈의 총합
    int sum_object_size = 0;

    // 객체 벡터 초기화
    detected_objects.clear();

    // 객체 외곽선 찾기(여러개의 외곽선을 동시에 추출: 벡터<벡터>)
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(subframe, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 외곽 기반 객체 검출
    for (const auto& contour : contours) {
        // 객체 박스처리
        cv::Rect object_rect = cv::boundingRect(contour);

        int object_size = object_rect.width * object_rect.height;

        if(object_size > 1000)
            // 객체 크기와 프레임 번호 출력(디버깅)
            std::cout << "[" << current_frame << "] frame 객체 크기 : " << object_size << " (Width: " << object_rect.width << ", Height: " << object_rect.height << ")" << std::endl;

        // 특정 크기 이상의 객체만 고려
        if (object_size > 1000) {
            detected_objects.emplace_back(object_rect);
            cv::rectangle(frame, object_rect, cv::Scalar(0, 0, 255), 2);

            // 각 프레임의 객체 총합
            sum_object_size += object_size;
        }

        // 현재 프레임 번호 출력(디버깅)
        cv::putText(frame, "Frame : " + std::to_string(current_frame), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
    }

    // subframe에서 127(그림자) 이하인 부분을 지움
    cv::threshold(subframe, subframe, 127, 255, cv::THRESH_BINARY);

    return sum_object_size;
}



int main(int argc, char *argv[])
{
    // 메타 데이터 등록 QT에 등록되어 있지 않음
    qRegisterMetaType<cv::Mat>("cv::Mat&");

    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    VideoThread video;
    ProcessingThread processing;

    // 시그널과 슬롯 연결
    QObject::connect(&video, &VideoThread::Frame_Ready, &processing, &ProcessingThread::ProcessFrame);
    QObject::connect(&processing, &ProcessingThread::ProcessingResult, &w, &MainWindow::showFrame);
    QObject::connect(&processing, &ProcessingThread::ObjSizeResult, &w, &MainWindow::updateGraph);

    // 쓰레드 시작
    video.start();
    processing.start();

    video.quit();
    processing.quit();

    return a.exec();
}

