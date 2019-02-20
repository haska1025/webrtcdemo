//"use strict"

var videoElement = document.querySelector('video');
var audioSelect = document.querySelector("select#audioSource");
var videoSelect = document.querySelector('select#videoSource');
navigator.mediaDevices.enumerateDevices().then(listDevices).then(getStream).catch(handleError);
audioSelect.onchange=getStream;
videoSelect.onchange=getStream;
function listDevices(devInfos) {
    for (var i=0; i < devInfos.length;++i){
        var devInfo = devInfos[i];
        var option = document.createElement('option');
        option.value = devInfo.deviceId;
        if (devInfo.kind === 'audioinput'){
            option.text=devInfo.label || 'microphone' + (audioSelect.length+1);
            audioSelect.appendChild(option);
        } else if(devInfo.kind === 'videoinput'){
            option.text=devInfo.label || 'camera ' + (videoSelect.length+1);
            videoSelect.appendChild(option);
        } else {
            console.log('Found one other kind of source/device: ', devInfo);
        }
    }
}

function getStream() {
    if (window.stream) {
        window.stream.getTracks().forEach(function(track) {
            track.stop();
        });
    }

    var constraints = {
        audio: {
            deviceId: {exact: audioSelect.value}
        },
        video: {
            deviceId: {exact: videoSelect.value}
        }
    };

    navigator.mediaDevices.getUserMedia(constraints).
    then(gotStream).catch(handleError);
}

function gotStream(stream) {
    window.stream = stream; // make stream available to console
    videoElement.srcObject = stream;
    //视频是否显示，和下面这段代码关系不大，不知道这段代码是什么作用
   // videoElement.onloadedmetadata = function(e) {
   //     videoElement.play();
   // };
}

function handleError(error) {
    console.log('Error: ', error);
}
