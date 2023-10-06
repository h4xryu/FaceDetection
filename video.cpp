#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

cv::Mat frame;
cv::Mat subframe;
// 예제 비디오 파일
cv::VideoCapture cap("vtest.avi");
// MOG2 알고리즘
cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2;

bool selecting_roi = false;
cv::Rect roi_rect;

// 현재 프레임
int current_frame = 0;
// 영상 출력 여부
bool is_playing = true;
// 객체 벡터
std::vector<cv::Rect> detected_objects; 

// 트랙바 이동
void onTrackbarSlide(int pos, void* userdata) {
    current_frame = pos;
    cap.set(cv::CAP_PROP_POS_FRAMES, current_frame);
}

void mouse_callback(int event, int x, int y, int flags, void* userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        roi_rect.x = x;
        roi_rect.y = y;
        selecting_roi = true;
    }
    else if (event == cv::EVENT_MOUSEMOVE && selecting_roi) {
        roi_rect.width = x - roi_rect.x;
        roi_rect.height = y - roi_rect.y;
    }
    else if (event == cv::EVENT_LBUTTONUP) {
        roi_rect.width = x - roi_rect.x;
        roi_rect.height = y - roi_rect.y;
        selecting_roi = false;
    }
}

void detectAndDrawObjects(cv::Mat& fgmask, cv::Mat& frame) {
    // 객체 벡터 초기화
    detected_objects.clear();

    // 객체 외곽 찾기
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(fgmask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 외곽 기반 객체 검출
    for (const auto& contour : contours) {
        /////////////// 수정을 통한 인식률 향상(신뢰도)
        // 객체 박스처리
        cv::Rect object_rect = cv::boundingRect(contour);

        // 특정 크기 이상의 객체만 고려
        if (object_rect.width * object_rect.height > 1000) {
            detected_objects.push_back(object_rect);
            cv::rectangle(frame, object_rect, cv::Scalar(0, 0, 255), 2);
        }
    }
}

int main() {
    if (!cap.isOpened()) {
        std::cerr << "웹캠 연결 상태를 확인해주세요." << std::endl;
        return -1;
    }

    int count_frame = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

    cv::namedWindow("frame");
    cv::createTrackbar("Frame", "frame", &current_frame, count_frame - 1, onTrackbarSlide);
    cv::setMouseCallback("frame", mouse_callback);

    // 히스토리 길이, 임계값, 그림자 검출 여부
    pMOG2 = cv::createBackgroundSubtractorMOG2(500, 16);

    while (true) {
        // 'p'를 통해 실시간 영상 멈춤
        char key = cv::waitKey(10);
        if (key == 'p' || key == 'P') {
            is_playing = !is_playing;
        }
        if (key == 27) {
            is_playing = false;
            break;
        }

        if (is_playing) {
            cap >> frame;
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
        }

        cv::imshow("frame", frame);
        cv::imshow("subframe", subframe);
    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}
