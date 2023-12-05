#include "PylonGrab.h"

namespace Vision {
    PylonGrab::PylonGrab(QThread::Priority _priority) {
        threadPriority = _priority;
        threadRunning = false;
        threadStreaming = false;
        threadStatus = CamThreadStatus::stopping;
        deviceConnected = false;
    }

    PylonGrab::~PylonGrab() {
        threadStatus = CamThreadStatus::disconnecting;
        wait();
    }

    void PylonGrab::run() {
        while(threadRunning) {
            mutex.tryLock(CAMTHREAD_MUTEX_TIMEOUT);
            switch (threadStatus) {
            case CamThreadStatus::connecting:
                cameraStateConnecting();
                break;
            case CamThreadStatus::idle:
                //
                break;
            case CamThreadStatus::preStream:
                cameraStartStreamGrabbing();
                break;
            case CamThreadStatus::streaming:
                cameraReadNewFrame();
                break;
            case CamThreadStatus::stopStream:
                cameraStopStreamGrabbing();
                threadStatus = CamThreadStatus::idle;
                break;
            case CamThreadStatus::singalShot:
                cameraSignleShotGrabbing();
                break;
            case CamThreadStatus::disconnecting:
                cameraStateDisconnecting();
                break;
            case CamThreadStatus::stopping:
                threadRunning = false;
                break;
            }
            mutex.unlock();
        }
    }

    void PylonGrab::cameraStateConnecting() {
        try {
            // get instance of first camera in list
            pylonCamera = new CInstantCamera(CTlFactory::GetInstance().CreateDevice(device));
            // open connect with camera
            pylonCamera->Open();
            // sleep for camera warm-up about 0.1s
            msleep(100);
            // check camera open status
            if(pylonCamera->IsOpen()) {
                deviceConnected = true;
                emit signal_cameraConnected();
                threadStatus = CamThreadStatus::idle;
            }
            else {
                deviceConnected = false;
                emit signal_cameraConnectFail();
                threadStatus = CamThreadStatus::stopping;
            }
        }
        catch (GenICam::GenericException &e) {
            emit signal_errorOccurred(QString::fromUtf8(e.GetDescription()));
        }
    }

    void PylonGrab::cameraStateDisconnecting() {
        if(pylonCamera->IsOpen()) {
//            if(pylonCamera->IsGrabbing()) {
//                pylonCamera->StopGrabbing();
//            }
            cameraStopStreamGrabbing();
            pylonCamera->Close();
            deviceConnected = false;
            emit signal_cameraDisconnected();
        }
        threadStatus = CamThreadStatus::stopping;
    }

    void PylonGrab::cameraStartStreamGrabbing() {
        if(pylonCamera->IsGrabbing()) {
            return;
        }
        try {
            // start grabbing, just read lasted image comming, thread loop by user provide
            pylonCamera->StartGrabbing(Pylon::GrabStrategy_LatestImageOnly, EGrabLoop::GrabLoop_ProvidedByUser);
            threadStatus = CamThreadStatus::streaming;
            threadStreaming = true;
        }
        catch(GenICam::GenericException &e) {
            emit signal_errorOccurred(QString::fromUtf8(e.GetDescription()));
            threadStatus = CamThreadStatus::disconnecting;
        }
    }

    void PylonGrab::cameraStopStreamGrabbing() {
        if(pylonCamera->IsGrabbing()) {
            pylonCamera->StopGrabbing();
            threadStreaming = false;
        }
    }

    void PylonGrab::cameraSignleShotGrabbing() {
        try {
            CPylonImage pylonFrame;
            CImageFormatConverter formatConverter;
            formatConverter.OutputPixelFormat = PixelType_BGR8packed;
            // This smart pointer will receive the grab result data.
            Pylon::CInstantCamera::GrabResultPtr_t ptrGrabResult;
            // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
            pylonCamera->GrabOne(RETRIEVE_FRAME_TIMEOUT, ptrGrabResult, TimeoutHandling_ThrowException);
            // check retrieve result
            if (ptrGrabResult->GrabSucceeded())
            {
                // Convert the grabbed buffer to pylon image
                formatConverter.Convert(pylonFrame, ptrGrabResult);
                // Create an OpenCV image out of pylon image
                cv::Mat tempFrame = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(),
                                            CV_8UC3, (uint8_t *) pylonFrame.GetBuffer());
                emit signal_NewFrameRead(tempFrame.clone());
            }
            else
            {
                emit signal_errorOccurred("Fail to Grab Frame.");
            }
        }
        catch (GenICam::GenericException &e) {
            emit signal_errorOccurred(QString::fromUtf8(e.GetDescription()));
        }
        threadStatus = CamThreadStatus::idle;
    }

    void PylonGrab::cameraReadNewFrame() {
        try {
            CPylonImage pylonFrame;
            CImageFormatConverter formatConverter;
            formatConverter.OutputPixelFormat = PixelType_BGR8packed;
            // This smart pointer will receive the grab result data.
            Pylon::CInstantCamera::GrabResultPtr_t ptrGrabResult;
            // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
            pylonCamera->RetrieveResult(RETRIEVE_FRAME_TIMEOUT, ptrGrabResult, TimeoutHandling_ThrowException);
            // check retrieve result
            if (ptrGrabResult->GrabSucceeded())
            {
                // Convert the grabbed buffer to pylon image
                formatConverter.Convert(pylonFrame, ptrGrabResult);
                // Create an OpenCV image out of pylon image
                cv::Mat tempFrame = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(),
                                            CV_8UC3, (uint8_t *) pylonFrame.GetBuffer());
                emit signal_NewFrameRead(tempFrame.clone());
            }
            else
            {
                emit signal_errorOccurred("Fail to Grab Frame.");
            }
        }
        catch (GenICam::GenericException &e) {
            emit signal_errorOccurred(QString::fromUtf8(e.GetDescription()));
        }
    }

    /* PUBLIC FUNTIONS */

    void PylonGrab::setDeviceInfo(CDeviceInfo _device) {
        device = _device;
    }

    void PylonGrab::cameraConnect() {
        if(!threadRunning) {
            threadRunning = true;
            mutex.tryLock(CAMTHREAD_MUTEX_TIMEOUT);
            threadStatus = CamThreadStatus::connecting;
            mutex.unlock();
            start();
        }
    }

    void PylonGrab::cameraDisconnect() {
        mutex.tryLock(CAMTHREAD_MUTEX_TIMEOUT);
        threadStatus = CamThreadStatus::disconnecting;
        mutex.unlock();
        wait();
    }

    void PylonGrab::cameraStartStream(){
        mutex.tryLock(CAMTHREAD_MUTEX_TIMEOUT);
        if(threadStatus==CamThreadStatus::idle) {
            threadStatus = CamThreadStatus::preStream;
        }
        mutex.unlock();
    }

    void PylonGrab::cameraStopStream(){
        mutex.tryLock(CAMTHREAD_MUTEX_TIMEOUT);
        threadStatus = CamThreadStatus::stopStream;
        mutex.unlock();
    }

    void PylonGrab::cameraTriggerSingleShot(){
        mutex.tryLock(CAMTHREAD_MUTEX_TIMEOUT);
        if(threadStatus==CamThreadStatus::idle) {
            threadStatus = CamThreadStatus::singalShot;
        }
        mutex.unlock();
    }

    bool PylonGrab::isCameraConnected() {
        return deviceConnected;
    }

    bool PylonGrab::isCameraStreaming() {
        return threadStreaming;
    }

    CDeviceInfo PylonGrab::getDeviceInfo() {
        return device;
    }
}
