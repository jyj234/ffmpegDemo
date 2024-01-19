import QtQuick
import QtQuick.Window
//import FrameProvider 1.0
import QtMultimedia
import QtQuick.Controls
// import ZN_VideoInput 1.0

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
   GridView{
       id: gridView
       anchors.fill: parent
       cellHeight: parent.height / 4
       cellWidth: parent.width / 4
       model: 4
       delegate: ZN_MediaPlayer{
           width: gridView.cellWidth
           height: gridView.cellHeight

       }
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

    }
    Component.onDestruction: {
//        ZN_VideoInput.closeDecoders()
    }

}
