package com.beartooth.kodec2

import android.util.Log
import java.io.ByteArrayInputStream
import java.io.ByteArrayOutputStream
import kotlin.system.measureTimeMillis

/**
 * Kotlin JVM API for Codec 2. This class is a lightweight wrapper on the native library.
 *
 * **NOTES**:
 * - You can use this class to manage one or more instance of the codec, but only one instance is
 *   retained for each bitrate mode.
 * - Codec 2 only supports 8KHz 16 bit PCM as input/output formats.
 */
object Codec2 {
  private const val TAG = "Codec2"
  /**
   * Number of bits in an input sample.
   */
  const val INPUT_SAMPLE_BITS = 16
  /**
   * Number of bytes in an input sample.
   */
  const val INPUT_SAMPLE_BYTES = INPUT_SAMPLE_BITS / 8

  init {
    System.loadLibrary("codec2jni")
  }

  /**
   * Initialize a native codec instance for the given bitrate mode.
   * @return true if successful.
   */
  fun init(bitrate: Bitrate) = codec2Initialize(bitrate.ordinal).also { Log.i(TAG, "init($bitrate) = $it") }

  /**
   * Release a native codec instance for the given bitrate mode.
   */
  fun release(bitrate: Bitrate) = codec2Release(bitrate.ordinal).also { Log.i(TAG, "release($bitrate)") }

  /**
   * Get frame sizing for a given bitrate mode.
   */
  fun getFrameSize(bitrate: Bitrate): FrameSize {
    val bytes = codec2GetFrameLengthBytes(bitrate.ordinal)
    val samples = codec2GetFrameLengthSamples(bitrate.ordinal)
    return FrameSize(samples, bytes, samples * INPUT_SAMPLE_BYTES).also { Log.v(TAG, "getFrameSize($bitrate) = $it") }
  }

  /**
   * Encode PCM audio into a voice frame buffer.
   * @param bitrate The codec bitrate mode.
   * @param samples The PCM audio samples. **NOTE:** Partial frames in the input are ignored.
   */
  fun encode(bitrate: Bitrate, samples: ByteArray): ByteArray {
    val frameSize = getFrameSize(bitrate)
    if (samples.size % frameSize.decodedByteCount != 0)
      Log.w(TAG, "encode($bitrate): buffer contains partial frames!")
    val sampleStream = ByteArrayInputStream(samples)
    val encodeStream = ByteArrayOutputStream()
    val tempBuffer = ByteArray(frameSize.decodedByteCount)
    val encodeTime = measureTimeMillis {
      while (sampleStream.available() >= frameSize.decodedByteCount) {
        sampleStream.read(tempBuffer, 0, frameSize.decodedByteCount)
        val frame = codec2EncodeFrame(bitrate.ordinal, tempBuffer)
        encodeStream.write(frame, 0, frame.size)
      }
    }
    Log.d(TAG, "encode($bitrate): ${samples.size}B in $encodeTime ms")
    val frames = encodeStream.toByteArray()
    runCatching {
      sampleStream.close()
      encodeStream.close()
    }
    return frames
  }

  /**
   * Decode PCM audio from a voice frame buffer.
   * @param bitrate The codec bitrate mode.
   * @param frames Encoded voice frames. **NOTE:** Partial frames in the input are ignored.
   */
  fun decode(bitrate: Bitrate, frames: ByteArray): ByteArray {
    val frameSize = getFrameSize(bitrate)
    if (frames.size % frameSize.encodedByteCount != 0)
      Log.w(TAG, "decode($bitrate): buffer contains partial frames! (${frames.size}B)")
    val encodeStream = ByteArrayInputStream(frames)
    val sampleStream = ByteArrayOutputStream()
    val tempBuffer = ByteArray(frameSize.encodedByteCount)
    val decodeTime = measureTimeMillis {
      while (encodeStream.available() >= frameSize.encodedByteCount) {
        encodeStream.read(tempBuffer, 0, frameSize.encodedByteCount)
        val dec = codec2DecodeFrame(bitrate.ordinal, tempBuffer)
        sampleStream.write(dec, 0, dec.size)
      }
    }
    Log.d(TAG, "decode($bitrate): ${frames.size}B in $decodeTime ms")
    val samples = sampleStream.toByteArray()
    runCatching {
      encodeStream.close()
      sampleStream.close()
    }
    return samples
  }


  private external fun codec2Initialize(mode: Int): Boolean
  private external fun codec2Release(mode: Int)
  private external fun codec2GetFrameLengthBytes(mode: Int): Int
  private external fun codec2GetFrameLengthSamples(mode: Int): Int
  private external fun codec2EncodeFrame(mode: Int, samples_: ByteArray): ByteArray
  private external fun codec2DecodeFrame(mode: Int, bits_: ByteArray): ByteArray

  /**
   * Codec 2 bitrates.
   */
  enum class Bitrate {
    BPS_3200,
    BPS_2400,
    BPS_1600,
    BPS_1400,
    BPS_1300,
    BPS_1200,
    BPS_700,
    BPS_700B
  }

  /**
   * Voice frame sizing information.
   * @property samples The number of audio samples in a single voice frame.
   * @property encodedByteCount The number of bytes in an encoded voice frame.
   * @property decodedByteCount The number of bytes in a decoded frame. This can also be thought of
   * as the size of a single input frame.
   */
  data class FrameSize(val samples: Int, val encodedByteCount: Int, val decodedByteCount: Int)

}