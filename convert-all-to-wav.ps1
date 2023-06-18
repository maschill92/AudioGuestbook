Get-ChildItem . -Filter *.pcm | ForEach-Object {
    ffmpeg -f s16le -ar 44.1k -ac 1 -i $_.FullName ".\wavs\$($_.Name).wav"
}