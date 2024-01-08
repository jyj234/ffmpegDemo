import QtQuick
import QtQuick.Window
//import FrameProvider 1.0
import QtMultimedia
import QtQuick.Controls
import Decoder 1.0
Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    Button{
        width: 50
        height: 50
        onClicked: {
        }
    }

    VideoOutput{
        anchors.fill: parent
        id: videoOutput

    }



    Component.onCompleted: {
        Decoder.setVideoSink(videoOutput.videoSink)
        Decoder.moveThread()
    }
    Component.onDestruction: {
        Decoder.stopRecord()
        Decoder.quitThread()
    }

}
