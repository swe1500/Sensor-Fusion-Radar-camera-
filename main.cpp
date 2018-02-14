#include "ssd.h"
#include "rsvreader.h"

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace caffe;

struct Data{
    int ID;
    float dist, angle, vel;
};


class CanReceiver{
private:
    int    listenfd, connfd;
    struct sockaddr_in     servaddr;
    char   buff[16];
    int    n;

public:
    CanReceiver();
    void ReceiveMessage(vector<Data> &RadarDataTable, Data& temp);
    ~CanReceiver();
};


CanReceiver::CanReceiver(){

    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
    printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
    exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(6666);

    if( bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
    printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
    exit(0);
    }

    if( listen(listenfd, 10) == -1){
    printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
    exit(0);
    }

    printf("======waiting for client's request======\n");
    if( (connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1){
        printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
    }
}

void CanReceiver::ReceiveMessage(vector<Data> &RadarDataTable, Data& temp){
    RadarDataTable.clear();
    if(temp.ID == 1) RadarDataTable.push_back(temp);
    while(1){
    Data data;
    n = read(connfd,&buff,sizeof(buff));
    memcpy(&data.ID, buff, 4);
    memcpy(&data.dist, buff+4, 4);
    memcpy(&data.angle, buff+8, 4);
    memcpy(&data.vel, buff+12, 4);

    RadarDataTable.push_back(data);
    if(data.ID == 1 && RadarDataTable.size() > 0){
            temp = data;
            RadarDataTable.pop_back();
            return;
        }
    }
}

CanReceiver::~CanReceiver(){

    close(connfd);
    close(listenfd);
}


int main(){

        const string& model_file = "/home/cookie/ssd/ssd/deploy.prototxt";
        const string& weights_file = "/home/cookie/ssd/ssd/VGG_VOC0712_SSD_300x300_iter_120000.caffemodel";
        Detector detector(model_file, weights_file,"","104,117,123");
        cv::VideoCapture capture("/home/cookie/Fusion/DSCF0570.m4v");
        //cv::VideoCapture capture(0);
            if(!capture.isOpened())
                return 1;
        cv::Mat frame,Orgframe;
        cv::Mat imageUncharged;

        CanReceiver can;
        vector<Data> RadarDataTable;
        Data temp;

        capture.read(frame);

        while(capture.read(frame)){
            frame.copyTo(imageUncharged);
            std::vector<vector<float> > detections = detector.Detect(frame);
            can.ReceiveMessage(RadarDataTable, temp);


            /*********** Multiple Objects detection ***************/



            for (unsigned int i = 0; i < detections.size(); ++i) {
                const vector<float>& d = detections[i];
                CHECK_EQ(d.size(), 7);
                const float score = d[2];
                if (score>0.15 && static_cast<int>(d[1]) == 7){

                CvPoint x1(d[3] * frame.cols,d[4] * frame.rows),x2(d[5] * frame.cols,d[6] * frame.rows);
                stringstream stream;

                stream << fixed << setprecision(2) << d[2];
                string tag = label[static_cast<int>(d[1])];
                tag = tag + " " + stream.str();

                //printf("score: %f x1.x: %d x1.y: %d x2.x: %d x2.y: %d\n", score, x1.x, x1.y, x2.x, x2.y);
                for(unsigned int i = 0; i < RadarDataTable.size(); i++){
                    if((((float)(x1.x - frame.cols / 2)*35.0 / (float)frame.cols) <= RadarDataTable[i].angle) &&
                       (((float)(x2.x - frame.cols / 2)*35.0 / (float)frame.cols) >= RadarDataTable[i].angle )){

                        //printf("ID: %d Dist: %f Angle: %f Vel: %f\n",RadarDataTable[i].ID, RadarDataTable[i].dist, RadarDataTable[i].angle, RadarDataTable[i].vel);
                        stringstream radar;
                        radar << fixed << setprecision(2) << RadarDataTable[i].dist<<" "<<RadarDataTable[i].angle<<" "<<RadarDataTable[i].vel;
                        string data = radar.str();
                        cv::putText(frame, tag, CvPoint(d[3] * frame.cols,d[4] * frame.rows-10), 2, 1, cv::Scalar(0,155,133));
                        cv::rectangle(frame, x1, x2, cv::Scalar(0,155,133), 4, 1, 0);
                        cv::putText(frame,data,CvPoint(d[3] * frame.cols,d[4] * frame.rows+20),2,1,cv::Scalar(0,155,133));
                        break;
                    }
                    else{
                        cv::putText(frame, tag, CvPoint(d[3] * frame.cols,d[4] * frame.rows-10), 2, 1, cv::Scalar(240,30,0));
                        cv::rectangle(frame, x1, x2, cv::Scalar(240,30,0), 4, 1, 0);
                    }
                }
              }
            }
            cv::imshow("im", frame);
            cv::waitKey(1);

        }
        capture.release();
        return 0;


}


