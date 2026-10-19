# Fixture video

`camera.m3u8` and `camera.ts` are a two-second, silent H.264 test pattern for
offline HLS playback. They contain no camera footage or third-party media.
The player loops the clip. These committed media assets need no build generator.

Generated with FFmpeg 8.0.1 (libx264):

```sh
ffmpeg -hide_banner -loglevel error -f lavfi -i testsrc2=size=320x180:rate=12 \
  -t 2 -c:v libx264 -profile:v baseline -pix_fmt yuv420p -preset veryslow \
  -crf 32 -g 24 -an -fflags +bitexact -flags:v +bitexact -f hls \
  -hls_flags single_file -hls_time 2 -hls_playlist_type vod \
  -hls_segment_filename resources/debug/camera.ts resources/debug/camera.m3u8
```

The `video_streams.camera` fixture entry uses `qrc:/debug/camera.m3u8`.
`FixtureApi` copies this playlist and its segment into its own temporary directory
because the media demuxer resolves segment paths outside Qt resource I/O.
