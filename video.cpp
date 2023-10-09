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

// fgmask(subframe) = 전경 마스크 영상
void detectAndDrawObjects(cv::Mat& subframe, cv::Mat& frame) {
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
        }

        // 현재 프레임 번호 출력(디버깅)
        cv::putText(frame, "Frame : " + std::to_string(current_frame), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);
    }
    // subframe에서 127(그림자) 이하인 부분을 지움
    cv::threshold(subframe, subframe, 127, 255, cv::THRESH_BINARY);
}

// HOG 객체 검출
void detectAndDrawObjectsHOG(cv::Mat& frame, cv::HOGDescriptor& hog) {
    std::vector<cv::Rect> fullbody;
    std::vector<double> weights; // 검출된 객체의 가중치
    hog.detectMultiScale(frame, fullbody, weights, 0, cv::Size(8, 8), cv::Size(32, 32), 1.05, 2);

    for (size_t i = 0; i < fullbody.size(); i++) {
        cv::Rect person = fullbody[i];
        double weight = weights[i];

        // 일정 가중치 이상인 객체만 그리기
        if (weight >= 0) {
            cv::rectangle(frame, person, cv::Scalar(255, 0, 0), 2);
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

    // 히스토리 길이, 임계값, 그림자 검출 여부(배경 제거 객체)
    pMOG2 = cv::createBackgroundSubtractorMOG2(500, 16, true);


    // HOG 객체 검출기 초기화
    cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

    // 전신 검출용 Haar Cascade 분류기 초기화
    cv::CascadeClassifier fullbody_cascade;
    if (!fullbody_cascade.load("haarcascade_fullbody.xml")) {
        std::cerr << "파일을 찾을 수 없습니다." << std::endl;
        return -1;
    }

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

             // 전신 검출
            std::vector<cv::Rect> fullbody;
            fullbody_cascade.detectMultiScale(frame, fullbody, 1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 80));

            for (const auto& person : fullbody) {
                cv::rectangle(frame, person, cv::Scalar(0, 255, 0), 2);
            }

            // HOG 객체 검출
            detectAndDrawObjectsHOG(frame, hog);
            
        }

        cv::imshow("frame", frame);
        cv::imshow("subframe", subframe);
    }

    cap.release();
    cv::destroyAllWindows();

    return 0;
}
