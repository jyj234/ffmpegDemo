import QtQuick
import QtQuick.Controls
import QtMultimedia
import VideoItem 1.0
Item {
    id: root
    property int videoChannelId: 0
    property string source: ""
    signal znMediaStatusChanged()
    property int mediaStatus: videoLoader.sourceComponent === null ? 0 : videoLoader.item.mediaStatus
    property bool isAudio: false
    property int timeoutDuration: 3000000
    enum MediaStatusType{
        LoadingMedia,
        BufferedMedia
    }
    function click()
    {
        if(videoLoader.sourceComponent === videoItem)
            videoLoader.item.myrotate()
    }

    function play()
    {
        if(videoLoader.sourceComponent === videoItem)
            return;
        if(source !== "")
        {
            videoLoader.sourceComponent = videoItem

        }
    }
    function stop()
    {
        console.log("before mediaPlayer stop")
        videoLoader.sourceComponent = null
        source = ""
        console.log("after mediaPlayer stop")
    }
    Timer{
        //当格式改变时，延迟一定时间后再启动，防止ffmpeg找不到编码器（当h264到h265时会发生这种情况）
        //推测是刚换过编码格式时的数据有异常
        id: timer
        interval: 2000
        repeat: false
        onTriggered: {
            videoLoader.sourceComponent = videoItem
        }
    }

    Connections {
        target: videoLoader.item
        function onVideoInfoChangedSig() {
            videoLoader.sourceComponent = null
            timer.start()
        }
        function onVideoErrorSig(){
            videoLoader.sourceComponent = null
        }
    }
    Loader{
        id: videoLoader
        visible: true
        anchors.fill: parent
    }
    Component{
        id: videoItem
        VideoItem{
            id: videoout
            anchors.centerIn: parent
            // isAudioOutput: true
            isAudioOutput:isAudio
            channelId: videoChannelId
            timeoutDuration:root.timeoutDuration
            videoSource:root.source
            fillMode: VideoItem.PreserveAspectFit
            onMediaStatusChanged: {
                root.mediaStatus = mediaStatus
                root.znMediaStatusChanged()
            }
            MouseArea{
                property real pressedX : 0
                property real pressedY : 0

                anchors.fill: parent
                cursorShape:Qt.OpenHandCursor
                onWheel: {
                    if (wheel.angleDelta.y > 0) {
                        videoout.resize(1)
                    } else {
                        videoout.resize(-1)
                    }
                }
                onPressed: {
                    cursorShape = Qt.ClosedHandCursor
                    pressedX = mouseX;
                    pressedY = mouseY;
                }
                onReleased: {
                    cursorShape = Qt.OpenHandCursor
                }
                onMouseXChanged: {
                    if(!pressed)
                        return;
                    if(mouseX < 0 || mouseY < 0 || mouseX > width || mouseY > height)
                        return;
                    videoout.drag((mouseX - pressedX)/width,(mouseY - pressedY)/height)
                }
                onMouseYChanged: {
                    if(!pressed)
                        return;
                    if(mouseX < 0 || mouseY < 0 || mouseX > width || mouseY > height)
                        return;
                    videoout.drag((mouseX - pressedX)/width,(mouseY - pressedY)/height)
                }
            }
            Component.onCompleted: {
                start()
            }
            Component.onDestruction: {
                console.log("before destory mediaplayer")
                stop()
            }
        }
    }
    Component.onDestruction: {
        videoLoader.sourceComponent = null
    }
}
