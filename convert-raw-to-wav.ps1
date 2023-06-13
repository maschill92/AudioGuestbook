ffmpeg -f s16le -ar 44.1k -ac 1 -i input.pcm output.wav


# -f s16le … signed 16-bit little endian samples
# -ar 44.1k … sample rate 44.1kHz
# -ac 2 … 2 channels (stereo)
# -i file.pcm … input file
# file.wav … output file
