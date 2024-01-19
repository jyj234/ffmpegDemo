import QtQuick
import QtQuick.Controls
import QtMultimedia
import VideoItem 1.0
Item {
    id: root
    property alias myurl: textInput.text
    signal sourceChangedSig()
    function stopDecode()
    {
        videoItem.stopDecoder()
    }

    Popup{
        id: urlInputPopup
        anchors.centerIn: Overlay.overlay
        width: 150
        height: 100
        background: Rectangle{
            radius: 5

            border.width: 1
            border.color: "grey"

        }
        Column{
            id: popupContent
            anchors.fill: parent
            spacing: 10
            TextField{
                width: parent.width
                height:20
                id: textInput
                text: "rtsp://admin:123456@10.12.13.237:554/H264?ch=1&subtype=0"
            }
            Button{
                anchors.horizontalCenter: parent.horizontalCenter
                width: 50
                height: 30
                text: "确认"
                onClicked:  {
                    videoItem.setSource(myurl)
                    // sourceChangedSig()
                    urlInputPopup.close()
                }

            }
        }
    }
    Column{
        id: col
        anchors.fill: parent
        Row{
            Button{
                id: urlPopupBtn
                width: 100
                height: 50
                text: "打开"
                onClicked: {
                    urlInputPopup.open()
                }
            }
            Button{
                width: 100
                height: 50
                text: "关闭"
                onClicked: {
                    videoItem.stopDecoder()
                }
            }
        }
        VideoItem{
            id: videoItem
            width: height * 16 / 9
            height: parent.height - urlPopupBtn.height
            Component.onDestruction: {
                stopDecoder()
            }
        }
    }
}
