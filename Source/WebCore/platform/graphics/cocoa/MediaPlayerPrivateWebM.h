/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#if ENABLE(ALTERNATE_WEBM_PLAYER)

#include "MediaPlayerPrivate.h"
#include "PlatformLayer.h"
#include "SourceBufferParserWebM.h"
#include "TimeRanges.h"
#include "WebMResourceClient.h"
#include <wtf/HashFunctions.h>
#include <wtf/HashMap.h>
#include <wtf/LoggerHelper.h>
#include <wtf/RobinHoodHashMap.h>
#include <wtf/UniqueRef.h>
#include <wtf/Vector.h>

OBJC_CLASS AVSampleBufferAudioRenderer;
OBJC_CLASS AVSampleBufferDisplayLayer;

namespace WebCore {

class AudioTrackPrivateWebM;
class FragmentedSharedBuffer;
class MediaDescription;
class MediaSample;
class MediaSampleAVFObjC;
class ResourceError;
class SharedBuffer;
class TextTrackRepresentation;
class TrackBuffer;
class VideoLayerManagerObjC;
class VideoTrackPrivateWebM;

class MediaPlayerPrivateWebM
    : public MediaPlayerPrivateInterface
    , public WebMResourceClientParent
    , private LoggerHelper {
    WTF_MAKE_FAST_ALLOCATED;
public:
    MediaPlayerPrivateWebM(MediaPlayer*);
    ~MediaPlayerPrivateWebM();

    static void registerMediaEngine(MediaEngineRegistrar);
private:
    void load(const String&) final;

#if ENABLE(MEDIA_SOURCE)
    void load(const URL&, const ContentType&, MediaSourcePrivateClient&) final;
#endif
#if ENABLE(MEDIA_STREAM)
    void load(MediaStreamPrivate&) final;
#endif
    
    // WebMResourceClientParent
    friend class WebMResourceClient;
    void dataReceived(const SharedBuffer&) final;
    void loadFailed(const ResourceError&) final;
    void loadFinished(const FragmentedSharedBuffer&) final;

    void cancelLoad() final;

    PlatformLayer* platformLayer() const final;

    bool supportsPictureInPicture() const override { return true; }
    bool supportsFullscreen() const final { return true; }

    void play() final;
    void pause() final;
    bool paused() const final;

    FloatSize naturalSize() const final { return m_naturalSize; };

    bool hasVideo() const final { return m_hasVideo; };
    bool hasAudio() const final { return m_hasAudio; };

    void setPageIsVisible(bool) final;

    MediaTime timeFudgeFactor() const { return { 1, 10 }; }
    MediaTime currentMediaTime() const final;
    MediaTime durationMediaTime() const final { return m_duration; };
    MediaTime startTime() const final { return MediaTime::zeroTime(); };
    MediaTime initialTime() const final { return MediaTime::zeroTime(); };

    void seek(const MediaTime&) final;
    bool seeking() const final { return m_seeking; };

    void setRateDouble(double) final;
    double rate() const final { return m_rate; }
    double effectiveRate() const final;

    void setVolume(float) final;
    void setMuted(bool) final;

    MediaPlayer::NetworkState networkState() const final { return m_networkState; };
    MediaPlayer::ReadyState readyState() const final { return m_readyState; };

    std::unique_ptr<PlatformTimeRanges> seekable() const final;
    MediaTime maxMediaTimeSeekable() const final { return durationMediaTime(); }
    MediaTime minMediaTimeSeekable() const final { return startTime(); }
    std::unique_ptr<PlatformTimeRanges> buffered() const final;

    void setBufferedRanges(PlatformTimeRanges);
    void updateBufferedFromTrackBuffers(bool);

    bool didLoadingProgress() const final;

    void paint(GraphicsContext&, const FloatRect&) final { };

    DestinationColorSpace colorSpace() final { return DestinationColorSpace::SRGB(); };

    void setNaturalSize(FloatSize);
    void setHasAudio(bool);
    void setHasVideo(bool);
    void setDuration(MediaTime);
    void setNetworkState(MediaPlayer::NetworkState);
    void setReadyState(MediaPlayer::ReadyState);
    void characteristicsChanged();

    bool supportsAcceleratedRendering() const final { return true; };

    RetainPtr<PlatformLayer> createVideoFullscreenLayer() final;
    void setVideoFullscreenLayer(PlatformLayer*, Function<void()>&& completionHandler) final;
    void setVideoFullscreenFrame(FloatRect) final;

    bool requiresTextTrackRepresentation() const final;
    void setTextTrackRepresentation(TextTrackRepresentation*) final;
    void syncTextTrackBounds() final;

    void enqueueSample(Ref<MediaSample>&&, uint64_t);
    void reenqueSamples(uint64_t);
    void reenqueueMediaForTime(TrackBuffer&, uint64_t, const MediaTime&);
    void notifyClientWhenReadyForMoreSamples(uint64_t);

    bool canSetMinimumUpcomingPresentationTime(uint64_t) const;
    void setMinimumUpcomingPresentationTime(uint64_t, const MediaTime&);
    void clearMinimumUpcomingPresentationTime(uint64_t);

    bool isReadyForMoreSamples(uint64_t);
    void didBecomeReadyForMoreSamples(uint64_t);
    void provideMediaData(uint64_t);
    void provideMediaData(TrackBuffer&, uint64_t);

    void trackDidChangeSelected(VideoTrackPrivate&, bool);
    void trackDidChangeEnabled(AudioTrackPrivate&, bool);

    using InitializationSegment = SourceBufferParserWebM::InitializationSegment;
    void didParseInitializationData(InitializationSegment&&);
    void didEncounterErrorDuringParsing(int32_t);
    void didProvideMediaDataForTrackId(Ref<MediaSampleAVFObjC>&&, uint64_t trackId, const String& mediaType);

    void append(SharedBuffer&);
    void abort();

    void flush();
#if PLATFORM(IOS_FAMILY)
    void flushIfNeeded();
#endif
    void flushTrack(uint64_t);
    void flushVideo();
    void flushAudio(AVSampleBufferAudioRenderer*);

    void addTrackBuffer(uint64_t, RefPtr<MediaDescription>&&);

    void ensureLayer();
    void addAudioRenderer(uint64_t);
    void removeAudioRenderer(uint64_t);

    void destroyLayer();
    void destroyAudioRenderer(RetainPtr<AVSampleBufferAudioRenderer>);
    void destroyAudioRenderers();
    void clearTracks();

    const Logger& logger() const final { return m_logger.get(); }
    const char* logClassName() const final { return "MediaPlayerPrivateWebM"; }
    const void* logIdentifier() const final { return reinterpret_cast<const void*>(m_logIdentifier); }
    WTFLogChannel& logChannel() const final;

    friend class MediaPlayerFactoryWebM;
    static bool isAvailable();
    static void getSupportedTypes(HashSet<String, ASCIICaseInsensitiveHash>&);
    static MediaPlayer::SupportsType supportsType(const MediaEngineSupportParameters&);

    MediaPlayer* m_player;
    RetainPtr<AVSampleBufferRenderSynchronizer> m_synchronizer;
    RetainPtr<id> m_durationObserver;
    WeakPtr<WebMResourceClient> m_resourceClient;

    Vector<RefPtr<VideoTrackPrivateWebM>> m_videoTracks;
    Vector<RefPtr<AudioTrackPrivateWebM>> m_audioTracks;
    HashMap<uint64_t, UniqueRef<TrackBuffer>, DefaultHash<uint64_t>, WTF::UnsignedWithZeroKeyHashTraits<uint64_t>> m_trackBufferMap;
    RefPtr<TimeRanges> m_buffered { TimeRanges::create() };

    RetainPtr<AVSampleBufferDisplayLayer> m_displayLayer;
    HashMap<uint64_t, RetainPtr<AVSampleBufferAudioRenderer>, DefaultHash<uint64_t>, WTF::UnsignedWithZeroKeyHashTraits<uint64_t>> m_audioRenderers;
    Ref<SourceBufferParserWebM> m_parser;

    MediaPlayer::NetworkState m_networkState { MediaPlayer::NetworkState::Empty };
    MediaPlayer::ReadyState m_readyState { MediaPlayer::ReadyState::HaveNothing };

    Ref<const Logger> m_logger;
    const void* m_logIdentifier;
    std::unique_ptr<VideoLayerManagerObjC> m_videoLayerManager;

    FloatSize m_naturalSize;
    MediaTime m_currentTime;
    MediaTime m_duration;
    double m_rate { 1 };

    uint64_t m_enabledVideoTrackID { notFound };
    uint32_t m_abortCalled { 0 };
#if PLATFORM(IOS_FAMILY)
    bool m_displayLayerWasInterrupted { false };
#endif
    bool m_hasAudio { false };
    bool m_hasVideo { false };
    bool m_seeking { false };
    bool m_visible { false };
    bool m_parsingSucceeded { true };
    bool m_processingInitializationSegment { false };
};

} // namespace WebCore

#endif // ENABLE(ALTERNATE_WEBM_PLAYER)
