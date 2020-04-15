const NodeMediaServer = require('node-media-server');

const config={ 
  rtmp:{ 
    port:1935, 
	chunk_size:60000, 
	gop_cache:true, 
	ping:60, 
	ping_timeout:30 
  }, 
  http:{
    mediaroot: 'd:/media',	  
    port:8000, allow_origin:'*' 
  },
  trans: {
    ffmpeg: 'D:/ffmpeg-20190914-197985c-win64-static/bin/ffmpeg.exe',
    tasks: [
      {
        app: 'live',
        hls: true,
        hlsFlags: '[hls_time=2:hls_list_size=3:hls_flags=delete_segments]',
        dash: true,
        dashFlags: '[f=dash:window_size=3:extra_window_size=5]'
      }
    ]
  }
};

let nms = new NodeMediaServer(config);
nms.run();
