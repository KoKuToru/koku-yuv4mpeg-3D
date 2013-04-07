koku-yuv4mpeg-3D
================

Takes stereoscopic 3D video via yuv4mpeg and renders it so that 3D-Monitors show it right.

Example:
     mplayer -vo yuv4mpeg:file=>(./koku-yuv4mpeg-3D LEFT_RIGHT COL) '3d-video.mp4'
     
Possible input modes:
     NONE
     LEFT_RIGHT  =>  On the left-side is the left-eye on the right -side is the right-eye
     TOP_BOTTOM  =>  On the top -side is the left-eye on the bottom-side is the right-eye
     
Possible output modes:
     NONE
     LEFT_RIGHT  =>  On the left-side is the left-eye on the right -side is the right-eye
     TOP_BOTTOM  =>  On the top -side is the left-eye on the bottom-side is the right-eye
     ROW         =>  First horizontal line is the left-eye, second horizontal line is the right-eye
     COL         =>  Not working for now...
