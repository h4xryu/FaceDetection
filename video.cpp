#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

cv::Mat frame;

bool selecting_roi = false;
cv::Rect roi_rect; // 선택한 ROI의 좌표를 저장할 변수

/////////////////////////////////////////////////////////////// ROI좌표값이 -로 설정되게 되면 에러 발생

////// ROI 설정 후 객체감지를 한 다음 객체감지 사각형의 좌표가 ROI 시작 좌표 - 끝 좌표 인걸로 추정됨.

// 마우스 이벤트 처리
void mouse_callback(int event, int x, int y, int flags, void* userdata)
{
    if (event == cv::EVENT_LBUTTONDOWN) {
        roi_rect.x = x;
        roi_rect.y = y;
        selecting_roi = true;
        std::cout << "시작" <<x << "," << y << std::endl;   // 디버깅 코드
    }
    else if (event == cv::EVENT_MOUSEMOVE && selecting_roi) {
        roi_rect.width = x - roi_rect.x;
        roi_rect.height = y - roi_rect.y;
        std::cout << "이동"<<roi_rect.width << "," << roi_rect.height << std::endl; // 디버깅 코드
    }
    else if (event == cv::EVENT_LBUTTONUP) {
        roi_rect.width = x - roi_rect.x;
        roi_rect.height = y - roi_rect.y;
        std::cout << "끝"<<roi_rect.width << "," << roi_rect.height << std::endl;   // 디버깅 코드
        selecting_roi = false;
    }
}

// 얼굴 감지 roi영역
std::vector<cv::Rect> face_detectObjects(const cv::Mat& image, const cv::Rect& roi) {
    // cascadeClassifier 오브젝트 선언
    cv::CascadeClassifier face_cascade;

    // 미리 정의된 cascade파일을 읽어오기
    face_cascade.load("front_face.xml");
                                 
    // 오브젝트를 담을 벡터 선언
    std::vector<cv::Rect> face_objects; 
   
    // ROI 내에서 객체 탐지 수행
    cv::Mat roi_image = image(roi);

    // 이미지감지, 객체, 영상축소 비율 1.1, 이웃사각형이 3개이상 생성될 때 최종 검출
    if (!roi_image.empty()) { // ROI가 비어 있지 않을 때만 객체 탐지 수행
        face_cascade.detectMultiScale(roi_image, face_objects, 1.1, 10);
    }
    
    // 감지된 오브젝트 벡터를 리턴
    return face_objects;
}

// 눈 감지
std::vector<cv::Rect> eye_detectObjects(const cv::Mat& image, const cv::Rect& roi) {
    
    cv::CascadeClassifier eye_cascade;

    // 미리 정의된 cascade파일을 읽어오기
    eye_cascade.load("eye.xml");
                                 
    std::vector<cv::Rect> eye_objects; 

    // ROI 내에서 객체 탐지 수행
    cv::Mat roi_image = image(roi);
   
    // 이미지, 객체, 영상축소 비율 1.1, 이웃사각형이 3개이상 생성될 때 최종 검출
    if (!roi_image.empty()) { // ROI가 비어 있지 않을 때만 객체 탐지 수행
        eye_cascade.detectMultiScale(roi_image, eye_objects, 1.1, 50);
    }
    
    return eye_objects;
}

int main() {
    cv::VideoCapture cap(0, cv::CAP_V4L2); // 웹캠 연결, VideoLinux2캡처 백엔드 사용
    if (!cap.isOpened()) {
        std::cerr << "웹캠 연결 상태를 확인해주세요." << std::endl;
        return -1;
    }

    int face_detect_counting = 0;
    int eye_detect_counting = 0;
    int count_frame = 0;
    
    cv::namedWindow("ROI"); // 창 이름을 ROI로 설정
    cv::setMouseCallback("ROI", mouse_callback); // ROI 창에 마우스 콜백 함수 등록

    while (true) {
        count_frame += 1;
        cap >> frame; // 웹캠에서 프레임 읽기

        // ROI 선택 중이면 사각형 그리기
        if (selecting_roi) {
            cv::rectangle(frame, roi_rect, cv::Scalar(0, 255, 0), 2);
        }

        // ROI 선택이 끝나면 객체 탐지
        if (!selecting_roi) {
            std::vector<cv::Rect> faceobjects = face_detectObjects(frame, roi_rect);
            std::vector<cv::Rect> eyeobjects = eye_detectObjects(frame, roi_rect);


            // std::string label = "Face"; // 객체 이름
            // cv::rectangle(faceobjects, cv::Rect(cv::Point(roi_rect.x,roi_rect.y), cv::Point(roi_rect.width, roi_rect.height)),cv::Scalar(0,255,0),2);
            // cv::putText(frame, label, cv::Point(roi_rect.x, roi_rect.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);
            // face_detect_counting =+ 1;

            for (const auto& rect : faceobjects) {
                // 객체 이름 표시
                std::string label = "Face"; // 객체 이름

                int x = roi_rect.x + rect.x;
                int y = roi_rect.y + rect.y;
                int width = roi_rect.width + rect.width;
                int height = roi_rect.height + rect.height;

                cv::rectangle(frame, cv::Rect(cv::Point(x,y), cv::Point(width, height)), cv::Scalar(0, 255, 0), 2); // 초록색 사각형 그리기

                cv::putText(frame, label, cv::Point(x, y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);

                // 객체(얼굴) 감지 성공 카운팅
                face_detect_counting += 1;
            }
        
            ////////////////////////////// 객체감지 결과는 roi이미지내에서의 x,y,width,height값 표시할때는 frame에 대입해서 객체밀림(+roi_rect)
            for (const auto& circle : eyeobjects) {
                // 눈 중심 좌표 계산
                int x = (circle.x + circle.width / 2) + roi_rect.x;
                int y = (circle.y + circle.height / 2) + roi_rect.y;
                int radius = circle.width / 2;

                cv::Point center(x, y);     // 원 그리기 (눈)    
                cv::circle(frame, center, radius, cv::Scalar(0, 0, 255), 2); // 원 그리기 (눈)

                std::string label = "Eye"; // 객체 이름
                cv::putText(frame, label, cv::Point(x, y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);

                // 객체(눈) 감지 성공 카운팅
                eye_detect_counting += 1;
            }
        }
        // 실시간 화면
        cv::imshow("ROI", frame);

        // 아스키코드값(27 == ESC) 입력으로 종료
        if (cv::waitKey(1) == 27) {
            break;
        }

        // 디버깅 코드
        std::cout << "face_detect_counting : " << face_detect_counting << std::endl;
        std::cout << "eye_detect_counting : " << eye_detect_counting << std::endl;
    }

    std::cout << "처리한 총 Frame : " << count_frame << std::endl; 

    // 메모리 해제
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
