#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>

#include <array>
#include <atomic>
#include <memory>
#include <vector>

namespace icguitar
{

enum class Articulation : int
{
    SusDown = 0,
    SusUp,
    MuteDown,
    MuteUp,
    FingerMute,
    Brush,
    ReleaseNoise,
    Count
};

inline constexpr int kNumArticulations = static_cast<int>(Articulation::Count);
inline constexpr int kMaxRoundRobins = 4;
inline constexpr int kNumMidiNotes = 128;

struct SampleData
{
    juce::AudioBuffer<float> buffer;
    double sampleRate = 44100.0;
    int rootMidiNote = 60;
    int onsetSample = 0;
    std::atomic<bool> ready { false };
};

struct BankData
{
    std::array<std::array<std::array<SampleData, kMaxRoundRobins>, kNumMidiNotes>, kNumArticulations> slots;
};

/**
 * Thread-safe, lock-free-read sample library manager for UI Standard Guitar FLAC assets.
 * Loads samples asynchronously on a background thread and computes pre-attack onset offsets.
 */
class SampleBank
{
public:
    SampleBank();
    ~SampleBank();

    void startAsyncLoad(const juce::File& directory);
    bool loadSynchronously(const juce::File& directory);
    void stopLoading();

    class ReaderGuard
    {
    public:
        explicit ReaderGuard(const SampleBank& bank) noexcept
            : owner(bank)
        {
            owner.activeReaders.fetch_add(1, std::memory_order_acquire);
            data = owner.activeBank.load(std::memory_order_acquire);
        }

        ~ReaderGuard() noexcept
        {
            owner.activeReaders.fetch_sub(1, std::memory_order_release);
        }

        const BankData* get() const noexcept { return data; }

        ReaderGuard(const ReaderGuard&) = delete;
        ReaderGuard& operator=(const ReaderGuard&) = delete;

    private:
        const SampleBank& owner;
        const BankData* data = nullptr;
    };

    const SampleData* findSample(const BankData* bank,
                                 Articulation articulation,
                                 int targetMidiNote,
                                 int roundRobinIndex,
                                 double& outPitchRatio) const noexcept;

    bool isLoading() const noexcept { return loading.load(std::memory_order_acquire); }
    bool isLoaded() const noexcept { return loadedCount.load(std::memory_order_acquire) > 0 && !loading.load(std::memory_order_acquire); }
    int getLoadedCount() const noexcept { return loadedCount.load(std::memory_order_acquire); }
    int getTotalTargetCount() const noexcept { return totalTargetCount.load(std::memory_order_acquire); }
    float getProgress() const noexcept;
    juce::String getCurrentPath() const;
    juce::String getLastError() const;

    static juce::File resolveSamplesDirectory(const juce::File& candidate);
    static juce::File getDefaultSamplesDirectory();
    static juce::File getSavedSamplesDirectory();
    static void saveSamplesDirectoryPreference(const juce::File& dir);
    static bool parseNoteAndRoundRobin(const juce::String& stem, int& outMidiNote, int& outRrIndex);
    static int computeOnsetSample(const juce::AudioBuffer<float>& buffer, double sampleRate);

private:
    struct LoadTask
    {
        juce::File file;
        Articulation articulation;
        int midiNote;
        int rrIndex;
        int priority;
    };

    class LoaderThread : public juce::Thread
    {
    public:
        explicit LoaderThread(SampleBank& ownerBank)
            : juce::Thread("ICStandardGuitarSampleLoader"), owner(ownerBank) {}
        void run() override { owner.runLoaderThread(*this); }

    private:
        SampleBank& owner;
    };

    std::vector<LoadTask> buildLoadTasks(const juce::File& samplesDir) const;
    bool decodeTask(const LoadTask& task, BankData& targetBank, juce::AudioFormatManager& formatManager);
    void runLoaderThread(juce::Thread& thread);
    void swapBankData(std::unique_ptr<BankData> nextBank);

    std::unique_ptr<BankData> ownedBank;
    std::atomic<const BankData*> activeBank { nullptr };
    mutable std::atomic<int> activeReaders { 0 };

    std::unique_ptr<LoaderThread> loaderThread;
    juce::CriticalSection stateLock;
    juce::File currentDirectory;
    juce::String lastErrorMessage;

    std::atomic<bool> loading { false };
    std::atomic<int> loadedCount { 0 };
    std::atomic<int> totalTargetCount { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleBank)
};

} // namespace icguitar
