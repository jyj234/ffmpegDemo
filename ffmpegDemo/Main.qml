import QtQuick
import QtQuick.Window
//import FrameProvider 1.0
import QtMultimedia
import QtQuick.Controls
// import ZN_VideoInput 1.0
// import DHStream 1.0
Window {
    property int videoW: 500
    property int videoH: videoW * 9 / 16
    width: Screen.width
    height: Screen.height
    visible: true
    title: qsTr("Hello World")
    // Button{
    //     text: "close"
    //     anchors.bottom: parent.bottom
    //     width: 100
    //     height: 100
    //     z: 1
    //     onClicked: {
    //         for(var i  = 0;i<gridView.model;++i)
    //         {
    //             gridView.itemAt(i).
    //         }
    //     }
    // }
   Component{
       id:gridView
       GridView{
           anchors.fill: parent
           cellHeight: parent.height / 4 + 100
           cellWidth: parent.width / 4
           model: 2
           delegate: ZN_MediaPlayer{
               width: cellWidth
               height: cellHeight
               x:index % 4 * width
               y:index / 4 * height
           }
       }
   }
   Component{
       id: mediaPlayerCom
       Row{
           Button{
               width: 100
               height: 100
               onClicked: {
                   mediaPlayer.click()
               }
           }

           Rectangle{
               border.width: 1
               height:400
               width:400
               ZN_MediaPlayer{
                   id: mediaPlayer
                   isAudio:false
                   // source: "D:\\code\\Qt\\encode\\video\\sei.h265"
                   // source: "rtsp://admin:123456@10.12.5.107:554/H264?ch=1&subtype=0"
                   source: "rtsp://admin:admin@192.168.1.86:554/H264?ch=1&subtype=0"
                   anchors.fill: parent
                   Component.onCompleted: {
                        play()
                   }
               }
           }
       }
   }
    Loader{
        id: loader
        sourceComponent:mediaPlayerCom
    }


   // ZN_MediaPlayer{
   //     width: videoW
   //     height: videoH
   //     id: mediaPlayer1
   //     // onSourceChangedSig: {
   //     //     ZN_VideoInput.setSource(0,url,videoSink)
   //     // }
   // }

//    ZN_MediaPlayer{
//        anchors.left: mediaPlayer1.right
//        anchors.top: mediaPlayer1.top
//        width: videoW
//        height: videoH
//        id: mediaPlayer2
//        onSourceChangedSig: {
//            ZN_VideoInput.setSource(1,url,videoSink)
//        }
//    }
//    ZN_MediaPlayer{
//        anchors.left: mediaPlayer2.right
//        anchors.top: mediaPlayer1.top
//        width: videoW
//        height: videoH
//        id: mediaPlayer3
//        onSourceChangedSig: {
//            ZN_VideoInput.setSource(2,url,videoSink)
//        }
//    }
//    ZN_MediaPlayer{
//        anchors.left: mediaPlayer3.right
//        anchors.top: mediaPlayer1.top
//        width: videoW
//        height: videoH
//        id: mediaPlayer4
//        onSourceChangedSig: {
//            ZN_VideoInput.setSource(3,url,videoSink)
//        }
//    }
    Component.onCompleted: {
        // DHStream.startStream()
    }
    Component.onDestruction: {
        // DHStream.stopStream()
//        ZN_VideoInput.closeDecoders()
    }

}
