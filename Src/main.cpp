#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/face.hpp"
#include <opencv2/core/mat.hpp>
#include <stdio.h>
#include <math.h>
#include <iostream>

using namespace std;
using namespace cv;
using namespace cv::face;

CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
Ptr<Facemark> facemark;

int LEFT_EYE_POINTS[6] = {36, 37, 38, 39, 40, 41};
int RIGHT_EYE_POINTS[6] = {42, 43, 44, 45, 46, 47};
int MOUTH_EDGE_POINTS[6] = {48, 50, 52, 54, 56, 58};                                

struct EyeFrameOutput {  
    bool state;     
    Mat eye_frame;
    Mat eye_frame_processed;
};

Point middlePoint(Point p1, Point p2) 
{
    float x = (float)((p1.x + p2.x) / 2);
    float y = (float)((p1.y + p2.y) / 2);
    Point p = Point(x, y);
    return p;
}

float computeHeadTiltAngle(const vector<Point2f>& landmarks)
{
    Point2f left_eye_center = (landmarks[36] + landmarks[39]) * 0.5;
    Point2f right_eye_center = (landmarks[42] + landmarks[45]) * 0.5;

    float deltaY = right_eye_center.y - left_eye_center.y;
    float deltaX = right_eye_center.x - left_eye_center.x;
    float angle_rad = atan2(deltaY, deltaX);
    float angle_deg = angle_rad * 180.0 / CV_PI;

    return fabs(angle_deg);  // Return absolute angle
}
// Tính tỷ lệ ngáp dựa trên landmarks miệng
float yawningRatio(vector<Point2f> landmarks, int points[])
{
    Point left = Point(landmarks[points[0]].x, landmarks[points[0]].y);
    Point right = Point(landmarks[points[3]].x, landmarks[points[3]].y);
    Point top = middlePoint(landmarks[points[1]], landmarks[points[2]]);
    Point bottom = middlePoint(landmarks[points[5]], landmarks[points[4]]);

    float mouth_width = hypot((left.x - right.x), (left.y - right.y));
    float mouth_height = hypot((top.x - bottom.x), (top.y - bottom.y));
    float ratio = mouth_height / mouth_width;

    try {
        return ratio;
    } catch (exception& e) {
        return 0.0;
    }
}

// Drowsiness estimation based on landmarks
float blinkingRatio(vector<Point2f> landmarks, int points[]) 
{
    Point left = Point(landmarks[points[0]].x, landmarks[points[0]].y);
    Point right = Point(landmarks[points[3]].x, landmarks[points[3]].y);
    Point top = middlePoint(landmarks[points[1]], landmarks[points[2]]);
    Point bottom = middlePoint(landmarks[points[5]], landmarks[points[4]]);

    float eye_width = hypot((left.x - right.x), (left.y - right.y));
    float eye_height = hypot((top.x - bottom.x), (top.y - bottom.y));
    float ratio = eye_width / eye_height;
    
    try {
        return ratio;
    } catch (exception& e) {
        return 0.0;
    }
}

// Drowsiness estimation based on morph. operations and iris extraction
float iris_size(Mat frame) 
{
    Size size = frame.size();
    int height = size.height;
    int width = size.width;

    Mat frame_resized = frame(Range(5, height-5), Range(5, width-5));
    int height_resized = height-10;
    int width_resized = width-10;

    float n_pixels = height_resized * width_resized;
    float n_blacks = n_pixels - cv::countNonZero(frame_resized);

    return (n_blacks / n_pixels);
}

Mat iris_correction(Mat frame_eye) {
    int leftmost = frame_eye.cols;
    int rightmost = 0;
    int top = frame_eye.rows;
    int bottom = 0;

    for (int y = 0; y < frame_eye.rows; y++) {
        for (int x = 0; x < frame_eye.cols; x++) {
            if (frame_eye.at<uchar>(cv::Point2i(x,y)) == 0) {
                if (x < leftmost) {
                    leftmost = x;
                }
                if (y < top) {
                    top = y;
                }
                if (x > rightmost) {
                    rightmost = x;
                }
                if (y > bottom) {
                    bottom = y;
                }
            }
        }
    }

    int hdist = rightmost - leftmost;
    int vdist = bottom - top;
    double hv_ratio = hdist > 0 && vdist > 0 ? (double)hdist / (double)vdist : 0.0;

    Mat frame_eye_new;
    if (hv_ratio > 2.3) {
        frame_eye_new = cv::Mat(frame_eye.rows, frame_eye.cols, CV_8UC1, Scalar::all(255));
        cv::threshold(frame_eye_new, frame_eye_new, 240, 255.0, THRESH_BINARY);
    } else {
        frame_eye_new = frame_eye;
    }

    return frame_eye_new;
}

// Morphological operations used for iris extraction
Mat eye_processing(Mat frame_eye_resized, float threshold)
{
    Mat inv_mask;
    inRange(frame_eye_resized, Scalar(0, 0, 0), Scalar(0, 0, 0), inv_mask);
    frame_eye_resized.setTo(Scalar(255, 255, 255), inv_mask);

    // Contouring eye region
    Mat frame_eye_contours;
    cv::bilateralFilter(frame_eye_resized, frame_eye_contours, 10, 20, 5);

    Mat frame_eye_binary;
    cvtColor(frame_eye_contours, frame_eye_binary, COLOR_BGR2GRAY);
    cv::threshold(frame_eye_binary, frame_eye_binary, threshold, 255.0, THRESH_BINARY);

    Mat kernel(5,5, CV_8UC1, Scalar::all(255));
    Mat frame_eye_dilated;
    cv::dilate(frame_eye_binary, frame_eye_dilated, kernel);

    Mat frame_eye_closing;
    cv::erode(frame_eye_dilated, frame_eye_closing, kernel);

    Mat frame_eye_polished = iris_correction(frame_eye_closing);

    return frame_eye_polished;
}

// Calibration of threshold values used in binarization
float find_best_threshold(Mat eye_frame) 
{
    map<int, float> trials;
    float average_iris_size = 0.45;

    for (int i = 5; i < 100; i += 5) 
    {
        Mat frame_eye_binary = eye_processing(eye_frame, i); 
        float iris_result = iris_size(frame_eye_binary);
        trials.insert(pair<int, float>(i, iris_result));
    }

    float closest_distance = 100;
    float closest_threshold = 5;
    for (auto it = trials.begin(); it != trials.end(); ++it)
    {
        float distance = abs(average_iris_size - it->second);
        if (distance < closest_distance) 
        {
            closest_distance = distance;
            closest_threshold = it->first;
        }
    }
    
    return closest_threshold;
}

// Extraction of eye polygon from the image
Mat isolate(Mat frame, vector<Point2f> landmarks, int points[])
{
    Point region[1][20];

    for (int i = 0; i < 6; i++) {
        region[0][i] = Point(landmarks[points[i]].x, landmarks[points[i]].y-1);
    }

    Size size = frame.size();
    int height = size.height;
    int width = size.width;

    cv::Mat black_frame = cv::Mat(height, width, CV_8UC1, Scalar::all(0));
    cv::Mat mask = cv::Mat(height, width, CV_8UC1, Scalar::all(255));

    int npt[] = {6};
    const Point* ppt[1] = {region[0]};
    cv::fillPoly(mask, ppt, npt, 1, cv::Scalar(0, 0, 0));
    cv::bitwise_not(mask, mask);

    Mat frame_eye;
    frame.copyTo(frame_eye, mask);

    int margin = 5;
    int x_vals[] = {region[0][0].x, region[0][1].x, region[0][2].x, region[0][3].x, region[0][4].x, region[0][5].x};
    int y_vals[] = {region[0][0].y, region[0][1].y, region[0][2].y, region[0][3].y, region[0][4].y, region[0][5].y};
    int min_x = *std::min_element(x_vals, x_vals+6) - margin;
    int max_x = *std::max_element(x_vals, x_vals+6) + margin;
    int min_y = *std::min_element(y_vals, y_vals+6) - margin;
    int max_y = *std::max_element(y_vals, y_vals+6) + margin;

    // Ensure valid range
    min_x = max(0, min_x);
    max_x = min(width, max_x);
    min_y = max(0, min_y);
    max_y = min(height, max_y);

    Mat frame_eye_resized = frame_eye(Range(min_y, max_y), Range(min_x, max_x));

    return frame_eye_resized;
}

EyeFrameOutput detectFaceEyesAndDisplay(Mat frame, float& yawning_ratio, float& head_tilt_angle)
{
    Mat frame_gray;
    cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    equalizeHist(frame_gray, frame_gray);

    std::vector<Rect> faces;
    face_cascade.detectMultiScale(frame_gray, faces);

    if (faces.empty()) {
        yawning_ratio = 0.0;
        head_tilt_angle = 0.0;
        return EyeFrameOutput{0, Mat(), Mat()};
    }

    Mat faceROI = frame(faces[0]);
    rectangle(frame, faces[0], Scalar(255, 0, 0), 2);

    vector<vector<Point2f>> shapes;

    if (facemark->fit(frame, faces, shapes)) {
        yawning_ratio = yawningRatio(shapes[0], MOUTH_EDGE_POINTS);
        head_tilt_angle = computeHeadTiltAngle(shapes[0]);
    } else {
        yawning_ratio = 0.0;
        head_tilt_angle = 0.0;
    }

    Mat eye_frame = isolate(frame, shapes[0], LEFT_EYE_POINTS);
    float threshold = find_best_threshold(eye_frame);
    Mat eye_frame_processed = eye_processing(eye_frame, threshold);

    if (iris_size(eye_frame_processed) < 0.1) {
        return EyeFrameOutput{1, eye_frame, eye_frame_processed};
    } else {
        return EyeFrameOutput{0, eye_frame, eye_frame_processed};
    }
}

int main(int argc, const char** argv)
{
    String face_cascade_name = samples::findFile("haarcascade_frontalface_alt.xml");
    String facemark_filename = "lbfmodel.yaml";
    facemark = createFacemarkLBF();
    facemark->loadModel(facemark_filename);
    cout << "Loaded facemark LBF model" << endl;

    if (!face_cascade.load(face_cascade_name))
    {
        cout << "--(!)Error loading face cascade\n";
        return -1;
    }

    cv::VideoCapture capture;
    capture.set(cv::CAP_PROP_FRAME_WIDTH, 320);
    capture.set(cv::CAP_PROP_FRAME_HEIGHT, 240);
    capture.open(0);

    if (!capture.isOpened()) {
        cout << "Error opening video stream" << endl;
        return -1;
    }

    Mat frame;
    int eyeClosedFrameCount = 0;
    int yawnFrameCount = 0;
    int headTiltFrameCount = 0;
    int noFaceFrameCount = 0;
    float yawning_ratio = 0.0;
    float head_tilt_angle = 0.0;
    const float YAWN_THRESHOLD = 0.57;
    const float HEAD_TILT_THRESHOLD = 15.0;
    const int EYE_CLOSED_LIMIT = 30;
    const int YAWN_LIMIT = 15;
    const int HEAD_TILT_LIMIT = 30;
    const int NO_FACE_LIMIT = 30;

    while (true) {
        capture >> frame;
        if (frame.empty())
            break;

        EyeFrameOutput eyeOutput = detectFaceEyesAndDisplay(frame, yawning_ratio, head_tilt_angle);

        if (eyeOutput.eye_frame.empty()) {
            noFaceFrameCount++;
            eyeClosedFrameCount = 0;
            yawnFrameCount = 0;
            headTiltFrameCount = 0;

            if (noFaceFrameCount >= NO_FACE_LIMIT) {
                cout << "Alert: Drowsiness (No face detected)" << endl;
            } else {
                cout << "No face detected (" << noFaceFrameCount << " frames)" << endl;
            }
        } else {
            noFaceFrameCount = 0;

            bool isEyeClosed = eyeOutput.state;
            bool isYawning = yawning_ratio > YAWN_THRESHOLD;
            bool isHeadTilted = head_tilt_angle > HEAD_TILT_THRESHOLD;

            if (isEyeClosed) eyeClosedFrameCount++; else eyeClosedFrameCount = 0;
            if (isYawning) yawnFrameCount++; else yawnFrameCount = 0;
            if (isHeadTilted) headTiltFrameCount++; else headTiltFrameCount = 0;

            if (eyeClosedFrameCount >= EYE_CLOSED_LIMIT) {
                cout << "Alert: Drowsiness (Eyes closed for " << eyeClosedFrameCount << " frames)" << endl;
            } else if (yawnFrameCount >= YAWN_LIMIT) {
                cout << "Alert: Drowsiness (Yawning for " << yawnFrameCount << " frames)" << endl;
            } else if (headTiltFrameCount >= HEAD_TILT_LIMIT) {
                cout << "Alert: Drowsiness (Head tilted for " << headTiltFrameCount << " frames)" << endl;
            } else {
                cout << "Normal state" << endl;
            }

            cout << "Eye Ratio: " << iris_size(eyeOutput.eye_frame_processed) 
                 << " | Yawning Ratio: " << yawning_ratio 
                 << " | Head Tilt Angle: " << head_tilt_angle << " degrees" << endl;
        }

        char c = (char)waitKey(1);
        if (c == 27) // ESC
            break;
    }

    capture.release();
    return 0;
}

