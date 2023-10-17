#include "mainwindow.h"
#include <QApplication>
#include <QString>
#include <QDebug>
#include <opencv2/opencv.hpp>

cv::Mat frame;
cv::Mat subframe;
// 예제 비디오 파일
cv::VideoCapture cap("/home/cdm/Desktop/qt/qt_test/RealTimeGraph/vtest.avi");
// MOG2 알고리즘
cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2;

bool selecting_roi = false;
cv::Rect roi_rect;

// 현재 프레임
int current_frame = 0;

// 프레임 및 객체크기 보관 큐 전역변수 지정
extern QQueue<int> queue;

// 영상 출력 여부
bool is_playing = true;

// 객체 벡터
std::vector<cv::Rect> detected_objects;

// 트랙바 이동
void onTrackbarSlide(int pos, void* userdata) {
    current_frame = pos;
    cap.set(cv::CAP_PROP_POS_FRAMES, current_frame);
}

// fgmask(subframe) = 전경 마스크 영상
void detectAndDrawObjects(cv::Mat& subframe, cv::Mat& frame) {
    // 각 프레임의 객체 사이즈의 총합
    int sum_object_size = 0;

    // 객체 벡터 초기화
    detected_objects.clear();

    // 객체 외곽선 찾기(여러개의 외곽선을 동시에 추출: 벡터<벡터>)
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(subframe, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 외곽 기반 객체 검출
    for (const auto& contour : contours) {
        /////////////// 수정을 통한 인식률 향상(신뢰도)
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

    if(queue.isEmpty())
        std::cout << "queue is empty by main.cpp" << std::endl;
    else{
        queue.enqueue(current_frame);   // 현재 프레임
        queue.enqueue(sum_object_size); // 현재 프레임의 객체 총합
    }

    // subframe에서 127(그림자) 이하인 부분을 지움
    cv::threshold(subframe, subframe, 127, 255, cv::THRESH_BINARY);
}



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    if (!cap.isOpened()) {
        std::cerr << "웹캠 연결 상태를 확인해주세요." << std::endl;
        return -1;
    }

    int count_frame = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

    cv::namedWindow("frame");
    cv::createTrackbar("Frame", "frame", &current_frame, count_frame - 1, onTrackbarSlide);

    // 히스토리 길이, 임계값, 그림자 검출 여부(배경 제거 객체)
    pMOG2 = cv::createBackgroundSubtractorMOG2(500, 16, true);

    while (true) {
        // 'p'를 통해 실시간 영상 멈춤
        char key = cv::waitKey(100);
        if (key == 'p' || key == 'P') {
            is_playing = !is_playing;
        }
        if (key == 27) {
            is_playing = false;
            break;
        }

        if (is_playing) {
            cap >> frame;

            // 큐 초기화
            queue.clear();
            // 현재 프레임 증가
            current_frame++;

            if (frame.empty()) {
                std::cout << "비디오 파일 끝." << std::endl;
                is_playing = false;
                break;
            }
            // frame에 MOG2적용 후 subframe 출력
            pMOG2->apply(frame, subframe);

            // fgmask(subframe)에서 객체 검출
            detectAndDrawObjects(subframe, frame);

            // 그래프 업데이트
            w.updateGraph();

        }

        cv::imshow("frame", frame);
        cv::imshow("subframe", subframe);

    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}
