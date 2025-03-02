# Validation Notes

_Notes for validation as I read through the HLS specification._

## Media Segments

- Playlist media segments are specified by a URI and optionally a byte range.
- The duration of each segment is provided by the EXTINF tag.
- Each segment in the playlist has a unique media sequence number
  - The media sequence number starts at 0 (unless otherwise specified) for the first segment and is incremented by 1 for each subsequent segment.
- Each media segment must carry the continuation of the encoded bitstream from the previous segment.
  - The only exception are the first segment and segments marked by a discontinuity.
- Media segments support the following content formats:
  - MPEG-2 Transport Streams
  - Fragmented MPEG-4
  - Packed Audio
  - WebVTT (Web Video Text Tracks)

## Playlists

- The playlist file must have the .m3u8 or .m3u extension OR have the HTTP Content-Type header set to application/vnd.apple.mpegurl or audio/mpegurl.
- Playlist files must be UTF-8 encoded
  - The UTF-8 BOM must not be present
  - UTF-8 control characters must not be present, except for LF and CR
- Playlist lines are terminated by either a single LF or a CR followed by LF
- A playlist line can be:
  - A comment
  - A tag
  - A URI
  - Blank
