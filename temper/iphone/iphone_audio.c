#include <pthread.h>
#import <AudioToolbox/AudioQueue.h>
#import <mach/mach.h> 
#import <CoreFoundation/CoreFoundation.h>

//names for the registered ports
#define ANAME CFSTR("AAA")

pthread_t audio_thread;
CFMessagePortRef aLocal;
CFRunLoopSourceRef aSource;
// callbacks for the runloop sources
CFDataRef ThreadACallBack(CFMessagePortRef local, SInt32 msgid,
 CFDataRef data, void *info);

// jump-off point for the runloops
void *SetupRunLoop(void *source);

#define IPHONE_AUDIO_BUFFERS 6
#define IPHONE_AUDIO_BUFFER_SIZE (((44100 / 60) * 4) * 2)

extern void app_MuteSound(void);
extern void app_DemuteSound(void);
extern void audio_callback(s16 *stream, int length);

typedef struct AQCallbackStruct
{
  AudioQueueRef queue;
  UInt32 frameCount;
  AudioQueueBufferRef mBuffers[IPHONE_AUDIO_BUFFERS];
  AudioStreamBasicDescription mDataFormat;
} AQCallbackStruct;

AQCallbackStruct in;

u32 audio_paused = 1;
u32 audio_exit_thread = 0;

// Do not do either of these two without first locking/unlocking audio
// (see functions below)

void audio_callback(s16 *stream, int length)
{
  u32 sample_length = length / 2;
  u32 _length;
  s16 *stream_base = (s16 *)stream;
  s32 *source;
  s32 current_sample;

  u32 i;

  if(audio_paused)
    return;

  if(config.enable_sound)
  {
    if((audio.buffer_base + sample_length) >= AUDIO_BUFFER_SIZE)
    {
      u32 partial_length = (AUDIO_BUFFER_SIZE - audio.buffer_base) * 2;
      sound_copy(audio.buffer_base, partial_length, normal);
      sound_copy(0, length - partial_length, normal);
      audio.buffer_base = (length - partial_length) / 2;
    }
    else
    {
      sound_copy(audio.buffer_base, length, normal);
      audio.buffer_base += sample_length;
    }
  }
  else
  {
    if((audio.buffer_base + sample_length) >= AUDIO_BUFFER_SIZE)
    {
      u32 partial_length = (AUDIO_BUFFER_SIZE - audio.buffer_base) * 2;
      audio.buffer_base = (length - partial_length) / 2;
    }
    else
    {
      audio.buffer_base += sample_length;
    }
    memset(stream, 0, length);
  }
}

void initialize_audio()
{
  audio_exit_thread = 0;
  audio.output_frequency = 44100;
  audio.playback_buffer_size = IPHONE_AUDIO_BUFFER_SIZE;

  app_DemuteSound();
}

// Do not do either of these two without first locking/unlocking audio
// (see functions below)

void audio_signal_callback()
{
}

void audio_unstall_callback()
{
  audio.buffer_index = AUDIO_BUFFER_SIZE - 1;
  audio.buffer_base = 0;
  audio_signal_callback();
}

void audio_wait_callback()
{
}

void audio_reset_buffer()
{
  audio.buffer_index = 0;
  audio.buffer_base = 0;
  audio_signal_callback();
}

void audio_lock()
{
}

void audio_unlock()
{
}

void audio_pause()
{
  audio_paused = 1;
}

void audio_unpause()
{
  audio_paused = 0;
}

void audio_exit()
{
  audio.buffer_index = AUDIO_BUFFER_SIZE - 1;
  audio.buffer_base = 0;
  audio_exit_thread = 1;
}


static void AQBufferCallback(void *userdata, AudioQueueRef outQ,
 AudioQueueBufferRef outQB)
{
  unsigned char *coreAudioBuffer;
  coreAudioBuffer = (unsigned char*) outQB->mAudioData;
  
  outQB->mAudioDataByteSize = IPHONE_AUDIO_BUFFER_SIZE;
  AudioQueueSetParameter(outQ, kAudioQueueParam_Volume, __audioVolume);
  audio_callback(coreAudioBuffer, IPHONE_AUDIO_BUFFER_SIZE);
  
  AudioQueueEnqueueBuffer(outQ, outQB, 0, NULL);
}

int app_OpenSound()
{
  Float64 sampleRate = 44100.0;
  int i;
  UInt32 bufferBytes;
  Uint32 err;
    
  app_MuteSound();
  
  soundInit = 0;
  
  if(!config.enable_sound)
    return 0;

  in.mDataFormat.mSampleRate = sampleRate;
  in.mDataFormat.mFormatID = kAudioFormatLinearPCM;
  in.mDataFormat.mFormatFlags =
   kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
  in.mDataFormat.mBytesPerPacket = 4;
  in.mDataFormat.mFramesPerPacket = 2;
  in.mDataFormat.mBytesPerFrame = 2;
  in.mDataFormat.mChannelsPerFrame = 1;
  in.mDataFormat.mBitsPerChannel = 16;
    
  // Pre-buffer before we turn on audio
  err = AudioQueueNewOutput(&in.mDataFormat, AQBufferCallback,
   NULL, NULL, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode,
   0, &in.queue);
  
  bufferBytes = IPHONE_AUDIO_BUFFER_SIZE;
  
  for(i = 0; i < IPHONE_AUDIO_BUFFERS; i++)
  {
    err = AudioQueueAllocateBuffer(in.queue, bufferBytes, &in.mBuffers[i]);
    in.mBuffers[i]->mAudioDataByteSize = IPHONE_AUDIO_BUFFER_SIZE;
    // "Prime" by calling the callback once per buffer
    AudioQueueEnqueueBuffer(in.queue, in.mBuffers[i], 0, NULL);
  }
  
  soundInit = 1;
  err = AudioQueueStart(in.queue, NULL);
  
  return 0;
}

void app_CloseSound(void)
{
  if(soundInit == 1)
  {
    AudioQueueDispose(in.queue, true);
    soundInit = 0;
  }
}


void app_MuteSound(void)
{
  if(soundInit == 1)
    app_CloseSound();
}

void* SetupRunLoop(void *source)
{
  CFRunLoopAddSource(CFRunLoopGetCurrent(), *((CFRunLoopSourceRef *)source),
   kCFRunLoopDefaultMode);

  // start the run loop
  CFRunLoopRun();

  pthread_exit(0);

  return NULL;
}

CFDataRef ThreadACallBack(CFMessagePortRef local, SInt32 msgid, CFDataRef d,
 void *info)
{
  app_OpenSound();
  return NULL;
}

void app_DemuteSound()
{
  if(soundInit == 0)
    app_OpenSound();
}
