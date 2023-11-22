#include "mainwindow.h"
#include "main.cpp"

void VideoThread::pause() {
    QMutexLocker locker(&mutex);
    video_pause = true;
}

void VideoThread::resume() {
    QMutexLocker locker(&mutex);
    video_pause = false;
    video_pauseCondition.wakeAll();
}

void VideoThread::stop() {
    QMutexLocker locker(&mutex);
    video_stop = true;
}

void VideoThread::run() {   // 읽기 쓰레드 실행
    qDebug("VideoThread : start");

    cv::Mat frame;

    // 웹캠 영상 읽기
    cv::VideoCapture cap(0, cv::CAP_V4L2);

    if (!cap.isOpened()) {
        std::cerr << "웹캠 연결 상태를 확인해주세요." << std::endl;
        // return -1;
    }

    while (!video_stop) {       // stop() 호출 시 종료
        if (video_pause) {
            video_pauseCondition.wait(&mutex);
        }

        cap >> frame; // frame에 영상 담기

        emit Frame_Ready(frame); // frameReady 시그널을 발생시켜 영상 프레임을 전달
        VideoThread::msleep(100);
        std::cout<<"VideoThread: " << QThread::currentThread() << std::endl;

    }

    // 영상 읽기가 끝나면 리소스 해제
    cap.release();
}

void ProcessingThread::setFrame(cv::Mat &frame) {
    if(frame.empty()){
        ProcessingThread::msleep(100);
    }
    else{
    // 프레임 설정
    Temp_frame = frame;

    }
}

void ProcessingThread::run() {
    qDebug("ProcessingThread : start");

    cv::Size s = Temp_frame.size();
    int rows = s.height;
    int cols = s.width;

    if (!(rows > 0 && cols > 0)) {
        std::cout << "frame is empty by thread.cpp ProcessingThread run()" << std::endl;
        emit setFrame(Temp_frame);
        return run();
    }

    cv::Mat subframe;

    int processedFrames = 0;    // 처리한 프레임
    const int framesToProcess = 1;  // 1프레임씩 처리

    // 시간 측정
    cv::TickMeter time;

    // 히스토리 길이, 임계값, 그림자 검출 여부(배경 제거 객체)
    pMOG2 = cv::createBackgroundSubtractorMOG2(1000, 16, true);

    while (!Temp_frame.empty()) {
        // 시간 측정
        time.start();

        if(processedFrames >= framesToProcess){
            std::cout <<"쓰레드 대기" << std::endl;
            ProcessingThread::msleep(100);  // 대기
            processedFrames = 0; // 처리한 프레임 초기화
        }

        if (!is_playing) {
            break;
        }

        // 현재 프레임 증가기
        current_frame++;

        if(is_playing){
            // frame에 MOG2 적용 후 subframe 출력
            pMOG2->apply(Temp_frame, subframe);

            // fgmask(subframe)에서 객체 검출
            int objsize = detectAndDrawObjects(subframe, Temp_frame);

            // 프레임, subframe, fps의 객체 크기 총합을 시그널로 전달
            emit ProcessingResult(Temp_frame, subframe);
            emit ObjSizeResult(objsize);

            processedFrames++;
            std::cout<<"ProcessThread: " << QThread::currentThread() << std::endl;
        }

        // 1루프 시간 출력
        time.stop();
        std::cout << "1 loop_time: " << time.getAvgTimeMilli() << std::endl;
        time.reset();
    }

}
