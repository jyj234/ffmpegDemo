import QtQuick
import QtQuick.Controls
import QtMultimedia
import VideoItem 1.0
Item {
    id: root
    property int videoChannelId: 0
    property string source: "rtsp://admin:admin@192.168.1.86:554/H264?ch=1&subtype=0"
    signal znMediaStatusChanged()
    // property int mediaStatus: videoLoader.item.mediaStatus
    enum MediaStatusType{
        LoadingMedia,
        BufferedMedia
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
        if(videoLoader.sourceComponent)
        {
            videoLoader.sourceComponent = null
        }
        source = ""
    }

    Connections {
        target: videoLoader.item
        function onVideoInfoChangedSig() {
            videoLoader.sourceComponent = null
            videoLoader.sourceComponent = videoItem
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
            anchors.centerIn: parent
            isAudioOutput: true
            channelId: videoChannelId
            videoSource:root.source
            fillMode: VideoItem.PreserveAspectFit
            onMediaStatusChanged: {
                root.mediaStatus = mediaStatus
                root.znMediaStatusChanged()
            }

            Component.onCompleted: {
                start()
            }
            Component.onDestruction: {
                stop()
            }
        }
    }
    Component.onDestruction: {
        videoLoader.sourceComponent = null
    }
}
