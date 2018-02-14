TARGET = fusion
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
TEMPLATE = app
SOURCES += \
    main.cpp \
    ssd.cpp \
    rsvreader.cpp \
    lanedetection.cpp
INCLUDEPATH += /usr/local/include \
/usr/local/cuda/include \
/usr/local/cuda-8.0/include \
/usr/local/include \
/usr/local/include/opencv \
/usr/local/include/opencv2 \
/home/cookie/caffe/include \
#/home/cookie/caffe/src \
/home/cookie/caffe/build/src \
/usr/local/lib \
/usr/lib/x86_64-linux-gnu \
/opt/OpenBLAS/include \
#/opt/intel/compilers_and_libraries_2016.2.181/linux/mkl/include \
/usr/local/cuda-8.0/include
LIBS += -L/usr/local/lib  -L/home/cookie/caffe/build/lib/ \
/usr/local/lib/*.so \
/home/cookie/caffe/build/lib/*.so \
-lglog -lboost_system -lcaffe
#-lopencv_core -lopencv_highgui -lprotobuf -pthread -lgflags -lboost_filesystem -lm -lleveldb -lsnappy -llmdb -lopencv_imgproc -lboost_thread -lstdc++ -lcblas -latlas

HEADERS += \
    ssd.h \
    rsvreader.h
