QT       += core gui network serialbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    camera/CalibCamera.cpp \
    camera/ChooseCameraDialog.cpp \
    camera/GeoMatch.cpp \
    camera/GeoModel.cpp \
    camera/ImageCropper.cpp \
    camera/PatternDialog.cpp \
    camera/PylonGrab.cpp \
    dialog/InputFormDialog.cpp \
    dialog/SetColorButton.cpp \
    loadFile/SettingFile.cpp \
    main.cpp \
    mainwindow.cpp \
    plate/FlexibleFeed.cpp \
    robot/CoordinateCvt.cpp \
    robot/HansClient.cpp \
    robot/HansCommand.cpp \
    robot/TimeCounter.cpp \
    setting/SettingHandler.cpp \
    widget/ClickLabel.cpp

HEADERS += \
    camera/CalibCamera.h \
    camera/ChooseCameraDialog.h \
    camera/GeoMatch.h \
    camera/GeoModel.h \
    camera/ImageCropper.h \
    camera/PatternDialog.h \
    camera/PylonGrab.h \
    dialog/InputFormDialog.h \
    dialog/SetColorButton.h \
    loadFile/SettingFile.h \
    mainwindow.h \
    plate/FlexibleFeed.h \
    robot/CoordinateCvt.h \
    robot/HansClient.h \
    robot/HansCommand.h \
    robot/HansDefine.h \
    robot/TimeCounter.h \
    setting/SettingHandler.h \
    widget/ClickLabel.h

FORMS += \
    camera/CalibCamera.ui \
    camera/ChooseCameraDialog.ui \
    camera/PatternDialog.ui \
    mainwindow.ui

TRANSLATIONS += \
    E05-PnP_vi_VN.ts \
    E05-PnP_ja_JP.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L'C:/Program Files/Basler/pylon 7/Development/lib/x64/' -lPylonBase_v7_3
else:win32:CONFIG(debug, debug|release): LIBS += -L'C:/Program Files/Basler/pylon 7/Development/lib/x64/' -lPylonBase_v7_3
else:unix: LIBS += -L'C:/Program Files/Basler/pylon 7/Development/lib/x64/' -lPylonBase_v7_3

INCLUDEPATH += 'C:/Program Files/Basler/pylon 7/Development/include'
DEPENDPATH += 'C:/Program Files/Basler/pylon 7/Development/include'

win32:CONFIG(release, debug|release): LIBS += -LC:/opencv/build/x64/vc16/lib/ -lopencv_world480
else:win32:CONFIG(debug, debug|release): LIBS += -LC:/opencv/build/x64/vc16/lib/ -lopencv_world480d
else:unix: LIBS += -LC:/opencv/build/x64/vc16/lib/ -lopencv_world480

INCLUDEPATH += C:/opencv/build/include
DEPENDPATH += C:/opencv/build/include

RESOURCES += \
    AppResource.qrc

RC_ICONS = icon/gbit.ico
