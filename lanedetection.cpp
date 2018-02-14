/*******      Traffic Lane Detection      ********/

#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace std;

/** @brief Preprocess the raw image and denoise.

@param imageOrg Original image or video frame.
@param Leftline An output array used to hold the set of the left lines detected by Hough.
@param Rightline An output array used to hold the set of the right lines detected by Hough.
@param l_slope An output array used to hold the set of the slope of the left lines.
@param r_slope An output array used to hold the set of the slope of the right lines.
@param l_b An output array used to hold the set of the y intersect of the left lines.
@param r_b An output array used to hold the set of the y intersect of the right lines.
 */
void preLineHough(cv::Mat imageOrg,
                  vector<cv::Vec4i>*  Leftline,
                  vector<cv::Vec4i>*  Rightline,
                  vector<double>*     l_slope,
                  vector<double>*     r_slope,
                  vector<double>*     l_b,
                  vector<double>*     r_b) {
    /*** Define two spaces for separating the color space ***/
    cv::Mat     dst_Lab,dst_YCrCb;
    cv::Mat     b_plane,Y_plane;

    /*** Define ROI ***/
    int x       = imageOrg.cols * 6/32,
        y       = imageOrg.rows * 5/8,
        width   = imageOrg.cols * 22/32,
        height  = imageOrg.rows * 3/8;

    /*** Convert the image to 'lab' and 'YCrCb' color spaces ***/
    cv::cvtColor(imageOrg, dst_Lab,   CV_RGB2Lab);
    cv::cvtColor(imageOrg, dst_YCrCb, CV_RGB2YCrCb);

    cv::Mat imageROI_Lab(dst_Lab, cv::Rect(x, y, width, height));   // The b plane is good at extracting the yellow line
    cv::Mat imageROI_YCrCb(dst_YCrCb,cv::Rect(x, y, width, height));// The Y plane is good at extracting the White line and get rid of while color in environment


    vector<cv::Mat> Space_Lab,Space_YCrCb;
    cv::split(imageROI_Lab, Space_Lab);                             // Spilt the 3 channel color space into three separate plane
    cv::split(imageROI_YCrCb, Space_YCrCb);                         // However, we only use one of them for each color space.
    Y_plane = Space_YCrCb[0];
    b_plane = Space_Lab[2];


    cv::GaussianBlur(Y_plane, Y_plane, cv::Size(11,11),5,5);        // Apply Gaussian Filter to get rid of pepper and salt noise
    cv::GaussianBlur(b_plane, b_plane, cv::Size(11,11),5,5);        // For both pictures that we derived
    cv::threshold(Y_plane, Y_plane, 200.0, 250.0, cv::THRESH_BINARY);// Implement thresh, results are binary images.
    cv::threshold(b_plane, b_plane, 105.0, 120.0, cv::THRESH_BINARY);
    cv::Canny(b_plane, b_plane,110,95);                              // Implement Canny Edge detection
    cv::Canny(Y_plane, Y_plane,110,95);


    b_plane = cv::max(b_plane, Y_plane);                            // For the images that derived above, b_plane hold the information of yellow line,

    vector<cv::Vec4i> lines;                                        // Y_plane hold the information of white line. So take max out of these two images,
    vector<cv::Vec4i> res;                                          // can combine these two binary images into one.

    cv::HoughLinesP(b_plane,lines, 1, CV_PI/180, 80, 50,5);         // Implement Hough transfrom. Result is a set of detected lines.
    vector<cv::Vec4i>::const_iterator it=lines.begin();

    while (it!=lines.end()) {
        int dx = (*it)[0] - (*it)[2];                               // Reconstruct the line.
        int dy = (*it)[1] - (*it)[3];
        double angle = atan2(dy, dx) * 180 /CV_PI;
        double k_    = (double)dy/ dx;                              // k_ represents the slope of the line.
        double b     = -k_* (*it)[0] + (*it)[1];

        if (abs(angle) <= 10 || dx == 0) {                          // Eliminate the verticial lines and horizontal lines
            ++it;
            continue;
        }

        if ((*it)[1] > (*it)[3] + 50 || (*it)[1] < (*it)[3] - 50) { // Roar filter the lines.
            res.push_back(*it);
            if (k_ < 0) {                                           // Separate left lines and right lines, for next filting step
                Leftline->push_back(*it);
                l_slope->push_back(angle);
                l_b->push_back(b);
            }
            else if (k_ > 0) {
                Rightline->push_back(*it);
                r_slope->push_back(angle);
                r_b->push_back(b);
            }
        }
        ++it;
     }
}              //PrelineHough


/** @brief Remove the outlier lines by comparing slope and y intersection.

@param Lines A set of lines from the right or the left.
@param angle The corresponding set of the angle for the above line set.
@param b_ The corresponding set of the y intersection for the line set.
 */
vector<cv::Vec4i> LineFilter(vector<cv::Vec4i> lines, vector<double> angle, vector<double> b_){

    cv::Scalar meanValue = cv::mean(angle);
    float Slope_average = meanValue.val[0];
    cv::Scalar mean_b = cv::mean(b_);
    float b_average = mean_b.val[0];
    float angle_thres = 10, d_thres = 120;

    for(unsigned int i = 0;i < lines.size();i++){
        if(abs(angle[i] - Slope_average) > angle_thres || abs(b_[i] - b_average) > d_thres)
        {
            lines.erase(lines.begin() + i);
            angle.erase(angle.begin() + i);
            b_.erase(b_.begin() + i);
            i--;
        }
    }
    return lines;
}                 //LineFilter


/** @brief Fit a straight line and restrict the line in ROI range.

@param Lines A set of lines, outlier lines have been removed from this set.
@param ymin The upper range.
@param ymax The bottom range.
 */
vector<float> findLine(vector<cv::Vec4i> Lines, int ymin, int ymax){
    vector<cv::Point> points;
    vector<cv::Vec4i>::const_iterator it = Lines.begin();
    while(it!=Lines.end()){
        points.push_back(cv::Point((*it)[0], (*it)[1]));
        points.push_back(cv::Point((*it)[2], (*it)[3]));
        ++it;
    }
    vector<float> Line;
    cv::fitLine(points,Line,CV_DIST_L2,1,0.01,0.01);
    float k_ = Line[1] / Line[0];
    float   xup = (ymin - Line[3]) / k_ + Line[2];
    float   xdown = (ymax - Line[3]) / k_ + Line[2];

    Line[0] = xdown;
    Line[1] = ymax;
    Line[2] = xup;
    Line[3] = ymin;
    return Line;
}                   // findLine

void LaneDetection(cv::Mat imageOrg, vector<float>* line_L, vector<float>* line_R){
    vector<cv::Vec4i> Leftline, Rightline;
    vector<double>    l_slope, r_slope, l_b, r_b;

    preLineHough(imageOrg, &Leftline, &Rightline, &l_slope, &r_slope, &l_b, &r_b);
    int height  = imageOrg.rows * 3/8;
    Leftline = LineFilter(Leftline, l_slope, l_b);
    Rightline = LineFilter(Rightline, r_slope, r_b);
    if(Leftline.size() != 0)
    *line_L = findLine(Leftline, 30, height-35);
    if(Rightline.size() != 0)
    *line_R = findLine(Rightline, 30, height-45);
}
