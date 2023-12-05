#ifndef PYLONGRAB_H
#define PYLONGRAB_H

#include <QThread>
#include <QObject>
#include <QMutex>
#include <QMessageBox>

#include <pylon/PylonIncludes.h>
#include <opencv2/core.hpp>

using namespace Pylon;

namespace Vision {

#define CAMTHREAD_MUTEX_TIMEOUT 500
#define RETRIEVE_FRAME_TIMEOUT  1000

    class PylonGrab : public QThread
    {
        Q_OBJECT
    public:
        enum CamThreadStatus {
            connecting = 0,
            idle,
            preStream,
            streaming,
            stopStream,
            singalShot,
            disconnecting,
            stopping
        };

        PylonGrab(QThread::Priority _priority = QThread::NormalPriority);
        ~PylonGrab();

        void setDeviceInfo(CDeviceInfo _device);
        void cameraConnect();
        void cameraDisconnect();
        void cameraStartStream();
        void cameraStopStream();
        void cameraTriggerSingleShot();
        bool isCameraConnected();
        bool isCameraStreaming();
        CDeviceInfo getDeviceInfo();

    signals:
        void signal_cameraConnected();
        void signal_cameraConnectFail();
        void signal_cameraDisconnected();
        void signal_errorOccurred(QString error);
        void signal_NewFrameRead(cv::Mat image);

    public slots:

    private slots:

    private:
        void run() override;
        void cameraStateConnecting();
        void cameraStateDisconnecting();
        void cameraStartStreamGrabbing();
        void cameraStopStreamGrabbing();
        void cameraSignleShotGrabbing();
        void cameraReadNewFrame();

        Priority threadPriority;
        bool threadRunning;
        CamThreadStatus threadStatus;
        bool threadStreaming;

        // this variable auto init pylon runtime when object construct
        // and terminate pylon runtime when object destruct
        PylonAutoInitTerm pylonRuntimeAutoInit;

        CInstantCamera *pylonCamera;
        CDeviceInfo device;
        bool deviceConnected;

        QMutex mutex;
    };
}
#endif // PYLONGRAB_H
