//
// Created by Jefferson Jones on 2019-08-06.
//

#include <jni.h>
#include <stdbool.h>
#include <android/log.h>
#include <codec2.h>
#include <malloc.h>
#include <assert.h>

static const char *TAG = "codec2-jni";

static struct CODEC2 *codec_instances[7];

static int samples_per_frame(int mode) {
  assert(codec_instances[mode] != NULL);
  return codec2_samples_per_frame(codec_instances[mode]);
}

static int bits_per_frame(int mode) {
  assert(codec_instances[mode] != NULL);
  return codec2_bits_per_frame(codec_instances[mode]);
}

static int bytes_per_enc_frame(int mode) {
  int nbits = bits_per_frame(mode);
  return (nbits + 7) / 8;
}

const char *describe_codec_mode(int mode) {
  switch (mode) {
    case CODEC2_MODE_3200:
      return "3.2 Kbps";
    case CODEC2_MODE_2400:
      return "2.4 Kbps";
    case CODEC2_MODE_1600:
      return "1.6 Kbps";
    case CODEC2_MODE_1400:
      return "1.4 Kbps";
    case CODEC2_MODE_1300:
      return "1.3 Kbps";
    case CODEC2_MODE_1200:
      return "1.2 Kbps";
    case CODEC2_MODE_700:
      return "0.7 Kbps";
    case CODEC2_MODE_700B:
      return "0.7 Kbps (rev B)";
    default:
      return "?";
  }
}

static void print_codec_info(int mode, int nsamps, int nbits, int nbytes) {
  __android_log_print(ANDROID_LOG_INFO,
                      TAG,
                      "Rowetel Codec 2 - Bitrate %s",
                      describe_codec_mode(mode));
  __android_log_print(ANDROID_LOG_INFO,
                      TAG,
                      "samples/input frame: %d (%d bytes)",
                      nsamps,
                      nsamps * 2);
  __android_log_print(ANDROID_LOG_INFO,
                      TAG,
                      "bits/encoded frame: %d (packed in %d bytes)",
                      nbits,
                      nbytes);
}

static bool init_codec(int mode) {
  codec_instances[mode] = codec2_create(mode);
  if (codec_instances[mode] == NULL) {
    __android_log_print(ANDROID_LOG_ERROR, TAG, "codec failed to initialize");
    return false;
  }
  print_codec_info(mode, samples_per_frame(mode), bits_per_frame(mode), bytes_per_enc_frame(mode));
  return true;
}

static void destroy_codec(int mode) {
  if (codec_instances[mode] == NULL) return;
  __android_log_print(ANDROID_LOG_INFO, TAG, "destroying codec instance");
  codec2_destroy(codec_instances[mode]);
  codec_instances[mode] = NULL;
}

JNIEXPORT jboolean JNICALL
Java_com_beartooth_kodec2_Codec2_codec2Initialize(JNIEnv *env,
                                                  jobject instance,
                                                  jint mode) {
  if (codec_instances[mode] != NULL) destroy_codec(mode);
  return (jboolean)
  init_codec(mode);
}

JNIEXPORT void JNICALL
Java_com_beartooth_kodec2_Codec2_codec2Release(JNIEnv
*env,
jobject instance,
    jint
mode) {
destroy_codec(mode);
}

JNIEXPORT jint JNICALL
Java_com_beartooth_kodec2_Codec2_codec2GetFrameLengthBytes(JNIEnv *env,
                                                           jobject instance,
                                                           jint mode) {
  return bytes_per_enc_frame(mode);
}

JNIEXPORT jint JNICALL
Java_com_beartooth_kodec2_Codec2_codec2GetFrameLengthSamples(JNIEnv *env,
                                                             jobject instance,
                                                             jint mode) {
  return samples_per_frame(mode);
}

JNIEXPORT jbyteArray JNICALL
Java_com_beartooth_kodec2_Codec2_codec2EncodeFrame(JNIEnv *env,
                                                   jobject instance,
                                                   jint mode,
                                                   jbyteArray samples_) {
  jbyte *samples = (*env)->GetByteArrayElements(env, samples_, NULL);
  // check sample count
  assert((*env)->GetArrayLength(env, samples_) / 2 == samples_per_frame(mode));
  // allocate bitstream buffer
  int nbytes = bytes_per_enc_frame(mode);
  jbyteArray j_bits_ = (*env)->NewByteArray(env, nbytes);
  jbyte *j_bits = (*env)->GetByteArrayElements(env, j_bits_, NULL);
  codec2_encode(codec_instances[mode], (unsigned char *) j_bits, (short *) samples);
  (*env)->ReleaseByteArrayElements(env, samples_, samples, 0);
  (*env)->ReleaseByteArrayElements(env, j_bits_, j_bits, 0);
  return j_bits_;
}

JNIEXPORT jbyteArray JNICALL
Java_com_beartooth_kodec2_Codec2_codec2DecodeFrame(JNIEnv *env,
                                                   jobject instance,
                                                   jint mode,
                                                   jbyteArray bits_) {
  jbyte *bits = (*env)->GetByteArrayElements(env, bits_, NULL);
  // check bitstream size
  assert((*env)->GetArrayLength(env, bits_) == bytes_per_enc_frame(mode));
  //allocate sample buffer
  int nsamps = samples_per_frame(mode);
  jbyteArray j_samples_ = (*env)->NewByteArray(env, nsamps * 2);
  jbyte *j_samples = (*env)->GetByteArrayElements(env, j_samples_, NULL);
  codec2_decode(codec_instances[mode], (short *) j_samples, (const unsigned char *) bits);
  (*env)->ReleaseByteArrayElements(env, bits_, bits, 0);
  (*env)->ReleaseByteArrayElements(env, j_samples_, j_samples, 0);
  return j_samples_;
}