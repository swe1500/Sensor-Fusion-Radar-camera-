#ifndef SSD_H
#define SSD_H

#include <caffe/caffe.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>
#include <iomanip>
#include <iosfwd>
#include <memory>
#include <utility>

using namespace std;
using namespace caffe;

class Detector{
private:
    shared_ptr<Net<float> > net_;
    cv::Size input_geometry_;
    unsigned int num_channels_;
    cv::Mat mean_;

public:
    Detector(const string& model_file,
             const string& weights_file,
             const string& mean_file,
             const string& mean_value);

    std::vector<std::vector<float> > Detect(const cv::Mat& img);

    void WrapInputLayer(std::vector<cv::Mat>* input_channels);

    void Preprocess(const cv::Mat& img,
                    std::vector<cv::Mat>* input_channels);

    void SetMean(const string& mean_file, const string& mean_value);

}; //Detector

const string label[21] ={"background","aeroplane","bicycle","bird","boat","bottle","bus","car","cat","chair","cow",
                         "diningtable","dog","horse","motorbike","person","pottedplant","sheep","sofa","train","tvmonitor"};
#endif // SSD_H
