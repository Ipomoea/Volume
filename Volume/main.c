//
//  main.c
//  Volume
//
//  Created by Павел on 20.10.16.
//  Copyright © 2016 Павел. All rights reserved.
//

#include <CoreAudio/AudioHardware.h>
#include <CoreGraphics/CoreGraphics.h>

#define BUNDLE_IDENTIFIER @"com.ipomoea.Volume"

#define VOLUME_UP_KEY 111
#define VOLUME_DOWN_KEY 103
#define MUTE_KEY 109

#define VOLUME_STEP 0.05f
#define VOLUME_MAX 1.f
#define VOLUME_MIN 0.f

CGEventRef eventCallback(CGEventTapProxy, CGEventType, CGEventRef, void *);

float getVolume();
void setVolume(float involume);
void toggleMute();

int main(int argc, const char *argv[]) {
    
    if (!(setuid(0) == 0 && setgid(0) == 0)) {
        printf("Failed to gain root privileges\n");
        exit(EXIT_FAILURE);
    }
    
    CFMachPortRef eventTap;
    CGEventMask eventMask;
    CFRunLoopSourceRef runLoopSource ;
    
    eventMask = CGEventMaskBit(kCGEventKeyDown);
    eventTap = CGEventTapCreate(kCGSessionEventTap, kCGHeadInsertEventTap, kCGEventTapOptionDefault, eventMask, eventCallback, NULL);
    
    if (!eventTap) {
        printf("Failed to create event tap\n");
        exit(EXIT_FAILURE);
    }
    
    runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);
    
    CFRunLoopRun();
    
    return EXIT_FAILURE;
}

float getVolume() {
    
    float b_vol;
    OSStatus err;
    AudioDeviceID device;
    UInt32 size;
    UInt32 channels[2];
    float volume[2];
    
    size = sizeof device;
    err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &size, &device);
    
    if (err != noErr) {
        printf("audio-volume error get device\n");
        return 0.0;
    }
    
    size = sizeof b_vol;
    err = AudioDeviceGetProperty(device, 0, 0, kAudioDevicePropertyVolumeScalar, &size, &b_vol);
    
    if (err == noErr) {
        return b_vol;
    }
    
    size = sizeof(channels);
    err = AudioDeviceGetProperty(device, 0, 0,kAudioDevicePropertyPreferredChannelsForStereo, &size, &channels);
    if (err != noErr) {
        printf("error getting channel-numbers\n");
    }
    
    size = sizeof(float);
    err = AudioDeviceGetProperty(device, channels[0], 0, kAudioDevicePropertyVolumeScalar, &size, &volume[0]);
    if (noErr != err) {
        printf("error getting volume of channel %d\n", channels[0]);
    }
    
    err = AudioDeviceGetProperty(device, channels[1], 0, kAudioDevicePropertyVolumeScalar, &size, &volume[1]);
    if (noErr != err) {
        printf("error getting volume of channel %d\n", channels[1]);
    }
    
    b_vol = (volume[0] + volume[1]) / 2.00;
    
    return  b_vol;
}

void setVolume(float involume) {
    
    OSStatus err;
    AudioDeviceID device;
    UInt32 size;
    Boolean canset = false;
    UInt32 channels[2];
    
    size = sizeof device;
    err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &size, &device);
    if (err != noErr) {
        printf("audio-volume error get device\n");
        return;
    }
    
    size = sizeof canset;
    err = AudioDeviceGetPropertyInfo(device, 0, false, kAudioDevicePropertyVolumeScalar, &size, &canset);
    if (err == noErr && canset == true) {
        size = sizeof involume;
        err = AudioDeviceSetProperty(device, NULL, 0, false, kAudioDevicePropertyVolumeScalar, size, &involume);
        return;
    }
    
    size = sizeof(channels);
    err = AudioDeviceGetProperty(device, 0, false, kAudioDevicePropertyPreferredChannelsForStereo, &size,&channels);
    if (err != noErr) {
        printf("error getting channel-numbers\n");
        return;
    }
    
    size = sizeof(float);
    err = AudioDeviceSetProperty(device, 0, channels[0], false, kAudioDevicePropertyVolumeScalar, size, &involume);
    if (noErr != err) {
        printf("error setting volume of channel %d\n",channels[0]);
    }
    
    err = AudioDeviceSetProperty(device, 0, channels[1], false, kAudioDevicePropertyVolumeScalar, size, &involume);
    if (noErr != err) {
        printf("error setting volume of channel %d\n",channels[1]);
    }
    
}

CGEventRef eventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    
    if (type != kCGEventKeyDown) {
        return event;
    }
    
    CGKeyCode keycode = CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
    
    switch (keycode) {
        case VOLUME_UP_KEY: {
            
            float vol = getVolume();
            vol += VOLUME_STEP;
            if (vol > VOLUME_MAX) {
                vol = VOLUME_MAX;
            }
            setVolume(vol);
            
            break;
        }
        case VOLUME_DOWN_KEY: {
            
            float vol = getVolume();
            vol -= VOLUME_STEP;
            if (vol < VOLUME_MIN) {
                vol = VOLUME_MIN;
            }
            setVolume(vol);
            
            break;
        }
        case MUTE_KEY: {
            
            break;
        }
            
        default:
            break;
    }
    
    return event;
}


