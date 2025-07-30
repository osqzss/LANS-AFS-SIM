#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <process.h>
#else
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#endif

#include <lime/LimeSuite.h>

#define EXIT_CODE_NO_DEVICE_LIST (-3)
#define EXIT_CODE_NO_DEVICE (-2)
#define EXIT_CODE_LMS_OPEN (-1)

#define TX_FREQUENCY    2492028000.0
#define TX_SAMPLERATE   12000000.0
#define TX_BANDWIDTH    1200000.0
#define DEFAULT_ANTENNA 1 // antenna with BW [30MHz .. 2000MHz]

#define RING_BUFFER_SIZE 4096 // in blocks, not samples
#define SAMPLES_PER_BLOCK 4096 // can tune this for performance

#ifdef _WIN32
typedef struct timeval {
    long tv_sec;
    long tv_usec;
} timeval;

int gettimeofday(struct timeval* tp, struct timezone* tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 
    static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((uint64_t)file_time.dwLowDateTime);
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}
#endif

typedef struct s16iq_sample_s {
    signed short int i;
    signed short int q;
} s16iq_sample_s;

typedef struct ring_buffer_s {
    s16iq_sample_s *buffer; // big array
    size_t blocks;          // number of blocks
    size_t block_size;      // samples per block
    volatile size_t head;   // written by producer
    volatile size_t tail;   // consumed by consumer
    int done;               // producer done?
#ifdef _WIN32
    HANDLE mutex;
    HANDLE not_empty;
    HANDLE not_full;
#else
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
#endif
} ring_buffer_t;

// Ring buffer functions
void ring_buffer_init(ring_buffer_t *rb, size_t blocks, size_t block_size) {
    rb->buffer = (s16iq_sample_s *)malloc(sizeof(s16iq_sample_s) * blocks * block_size);
    rb->blocks = blocks;
    rb->block_size = block_size;
    rb->head = 0;
    rb->tail = 0;
    rb->done = 0;
#ifdef _WIN32
    rb->mutex = CreateMutex(NULL, FALSE, NULL);
    rb->not_empty = CreateEvent(NULL, FALSE, FALSE, NULL);
    rb->not_full = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
    pthread_mutex_init(&rb->mutex, NULL);
    pthread_cond_init(&rb->not_empty, NULL);
    pthread_cond_init(&rb->not_full, NULL);
#endif
}

void ring_buffer_destroy(ring_buffer_t *rb) {
    free(rb->buffer);
#ifdef _WIN32
    CloseHandle(rb->mutex);
    CloseHandle(rb->not_empty);
    CloseHandle(rb->not_full);
#else
    pthread_mutex_destroy(&rb->mutex);
    pthread_cond_destroy(&rb->not_empty);
    pthread_cond_destroy(&rb->not_full);
#endif
}

// Returns pointer to block for writing, or NULL if full
s16iq_sample_s *ring_buffer_get_write_block(ring_buffer_t *rb) {
    if (((rb->head + 1) % rb->blocks) == rb->tail)
        return NULL; // full
    return rb->buffer + rb->head * rb->block_size;
}

// Returns pointer to block for reading, or NULL if empty
s16iq_sample_s *ring_buffer_get_read_block(ring_buffer_t *rb) {
    if (rb->head == rb->tail)
        return NULL; // empty
    return rb->buffer + rb->tail * rb->block_size;
}

void ring_buffer_produce(ring_buffer_t *rb) {
    rb->head = (rb->head + 1) % rb->blocks;
}

void ring_buffer_consume(ring_buffer_t *rb) {
    rb->tail = (rb->tail + 1) % rb->blocks;
}

// Producer thread
#ifdef _WIN32
unsigned __stdcall producer_thread(void *arg)
#else
void *producer_thread(void *arg)
#endif
{
    struct {
        FILE *fp;
        ring_buffer_t *rb;
    } *ctx = arg;
    FILE *fp = ctx->fp;
    ring_buffer_t *rb = ctx->rb;

    while (1) {
#ifdef _WIN32
        WaitForSingleObject(rb->mutex, INFINITE);
#else
        pthread_mutex_lock(&rb->mutex);
#endif
        s16iq_sample_s *block = ring_buffer_get_write_block(rb);
        if (!block) {
            // Full, wait
#ifdef _WIN32
            ReleaseMutex(rb->mutex);
            WaitForSingleObject(rb->not_full, 100);
#else
            pthread_cond_wait(&rb->not_full, &rb->mutex);
            pthread_mutex_unlock(&rb->mutex);
#endif
            continue;
        }
#ifdef _WIN32
        ReleaseMutex(rb->mutex);
#endif

        size_t n = fread(block, sizeof(s16iq_sample_s), rb->block_size, fp);
        if (n == 0) {
            // EOF
#ifdef _WIN32
            WaitForSingleObject(rb->mutex, INFINITE);
#else
            pthread_mutex_lock(&rb->mutex);
#endif
            rb->done = 1;
#ifdef _WIN32
            SetEvent(rb->not_empty);
            ReleaseMutex(rb->mutex);
#else
            pthread_cond_signal(&rb->not_empty);
            pthread_mutex_unlock(&rb->mutex);
#endif
            break;
        }

#ifdef _WIN32
        WaitForSingleObject(rb->mutex, INFINITE);
#endif
        // Mark block as produced
        ring_buffer_produce(rb);
        // Wake consumer
#ifdef _WIN32
        SetEvent(rb->not_empty);
        ReleaseMutex(rb->mutex);
#else
        pthread_cond_signal(&rb->not_empty);
        pthread_mutex_unlock(&rb->mutex);
#endif
    }
    free(ctx);
    return 0;
}

void usage(const char *progname)
{
    fprintf(stderr, "Usage: %s [options] file | -\n"
        "  -g <gain> with gain in [0.0 .. 1.0] set the so-called normalized RF gain in LimeSDR (default: 0.4)\n"
        "  -c <channel> with channel either 0 or 1 (default: 0)\n"
        "  -a <antenna> with antenna in { 0, 1, 2 } (default: 1)\n"
        "  -i <index> select LimeSDR if multiple devices connected (default: 0)\n"
        "  -s <samplerate> configure baseband sample rate (default: 12000000.0)\n",
        progname);

    return;
}

int main(int argc, char *const argv[])
{
    if (argc<2) {
        usage(argv[0]);
        exit(1);
    }

    int i;
    double gain = 0.4;
    int32_t antenna = DEFAULT_ANTENNA;
    int32_t channel = 0;
    int32_t index = 0;
    double sampleRate = TX_SAMPLERATE;

    FILE* fp = NULL;

    struct timeval tv;
    long last_status_time = 0;
    long start_sec;

    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-g") && i + 1 < argc) {
            gain = atof(argv[++i]);
        }
        else if (!strcmp(argv[i], "-c") && i + 1 < argc) {
            channel = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-a") && i + 1 < argc) {
            antenna = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-i") && i + 1 < argc) {
            index = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-s") && i + 1 < argc) {
            sampleRate = atof(argv[++i]);
        }
        else if (argv[i][0] == '-' && argv[i][1] == '\0') {
            fp = stdin;
#ifdef _WIN32
            int retval = _setmode(_fileno(stdin), _O_BINARY);
#endif
        }
        else if (argv[i][0] == '-' && argv[i][1] != '\0') {
            usage(argv[0]);
            exit(1);
        }
        else {
            fp = fopen(argv[i], "rb");
            if (!fp) {
                fprintf(stderr, "ERROR: Failed to open input file.\n");
                exit(1);
            }
        }
    }

    int device_count = LMS_GetDeviceList(NULL);

    if (device_count < 1)
    {
        fprintf(stderr, "ERROR: No device was found.\n");
        return(EXIT_CODE_NO_DEVICE);
    }

    lms_info_str_t* device_list = malloc(sizeof(lms_info_str_t) * device_count);
    if (device_list == NULL)
    {
        fprintf(stderr, "ERROR: Failed to allocate device list.\n");
        return(EXIT_CODE_NO_DEVICE_LIST);
    }

    device_count = LMS_GetDeviceList(device_list);

    i = 0;
    while (i < device_count)
    {
        fprintf(stderr, "device[%d/%d]=%s" "\n", i + 1, device_count, device_list[i]);
        i++;
    }

    if (index < 0)
        index = 0;
    if (index >= device_count)
        index = 0;

    fprintf(stderr, "Using device index %d [%s]" "\n", index, device_list[index]);

    if (gain < 0.0)
        gain = 0.0;
    if (gain > 1.0)
        gain = 1.0;

    fprintf(stderr, "Using normalized gain %lf" "\n", gain);

    lms_device_t *device = NULL;

    if (LMS_Open(&device, device_list[index], NULL))
        return(EXIT_CODE_LMS_OPEN);

    int lmsReset = LMS_Reset(device);
    if (lmsReset)
        fprintf(stderr, "lmsReset %d(%s)" "\n", lmsReset, LMS_GetLastErrorMessage());

    int lmsInit = LMS_Init(device);
    if (lmsInit)
        fprintf(stderr, "lmsInit %d(%s)" "\n", lmsInit, LMS_GetLastErrorMessage());

    int channel_count = LMS_GetNumChannels(device, LMS_CH_TX);
    if (channel < 0)
        channel = 0;
    if (channel >= channel_count)
        channel = 0;

    fprintf(stderr, "Using channel %d" "\n", channel);

    int antenna_count = LMS_GetAntennaList(device, LMS_CH_TX, channel, NULL);
    fprintf(stderr, "TX%d Channel has %d antenna(ae)" "\n", channel, antenna_count);

    lms_name_t *antenna_name = malloc(sizeof(lms_name_t) * antenna_count);

    if (antenna_count > 0)
    {
        int i = 0;
        lms_range_t *antenna_bw = malloc(sizeof(lms_range_t) * antenna_count);
        LMS_GetAntennaList(device, LMS_CH_TX, channel, antenna_name);
        for (i = 0; i < antenna_count; i++)
        {
            LMS_GetAntennaBW(device, LMS_CH_TX, channel, i, antenna_bw + i);
            fprintf(stderr, "Channel %d, antenna [%s] has BW [%lf .. %lf] (step %lf)" "\n", channel, antenna_name[i], antenna_bw[i].min, antenna_bw[i].max, antenna_bw[i].step);
        }
    }

    if (antenna < 0)
        antenna = DEFAULT_ANTENNA;
    if (antenna >= antenna_count)
        antenna = DEFAULT_ANTENNA;

    LMS_SetNormalizedGain(device, LMS_CH_TX, channel, gain);
    LMS_EnableChannel(device, LMS_CH_TX, 1 - channel, false);
    LMS_EnableChannel(device, LMS_CH_RX, 0, false);
    LMS_EnableChannel(device, LMS_CH_RX, 1, false);
    LMS_EnableChannel(device, LMS_CH_TX, channel, true);

    double tx_freq = TX_FREQUENCY;

    int setLOFrequency = LMS_SetLOFrequency(device, LMS_CH_TX, channel, tx_freq);
    if (setLOFrequency)
        fprintf(stderr, "setLOFrequency(%lf)=%d(%s)" "\n", tx_freq, setLOFrequency, LMS_GetLastErrorMessage());

    lms_range_t sampleRateRange;
    int getSampleRateRange = LMS_GetSampleRateRange(device, LMS_CH_TX, &sampleRateRange);
    if (getSampleRateRange)
        fprintf(stderr, "getSampleRateRange=%d(%s)" "\n", getSampleRateRange, LMS_GetLastErrorMessage());
    else
        fprintf(stderr, "sampleRateRange [%lf MHz.. %lf MHz] (step=%lf Hz)" "\n", sampleRateRange.min / 1e6, sampleRateRange.max / 1e6, sampleRateRange.step);

    fprintf(stderr, "Set sample rate to %lf ..." "\n", sampleRate);
    int setSampleRate = LMS_SetSampleRate(device, sampleRate, 0);
    if (setSampleRate)
        fprintf(stderr, "setSampleRate=%d(%s)" "\n", setSampleRate, LMS_GetLastErrorMessage());

    double actualHostSampleRate = 0.0;
    double actualRFSampleRate = 0.0;
    int getSampleRate = LMS_GetSampleRate(device, LMS_CH_TX, channel, &actualHostSampleRate, &actualRFSampleRate);
    if (getSampleRate)
        fprintf(stderr, "getSampleRate=%d(%s)" "\n", getSampleRate, LMS_GetLastErrorMessage());
    else
        fprintf(stderr, "actualRate %lf (Host) / %lf (RF)" "\n", actualHostSampleRate, actualRFSampleRate);

    fprintf(stderr, "Calibrating ..." "\n");
    int calibrate = LMS_Calibrate(device, LMS_CH_TX, channel, TX_BANDWIDTH, 0);
    if (calibrate)
        fprintf(stderr, "calibrate=%d(%s)" "\n", calibrate, LMS_GetLastErrorMessage());

    fprintf(stderr, "Setup TX stream ..." "\n");
    lms_stream_t tx_stream = { .channel = channel,.fifoSize = 1024 * 1024,.throughputVsLatency = 0.5,.isTx = true,.dataFmt = LMS_FMT_I12 };
    int setupStream = LMS_SetupStream(device, &tx_stream);
    if (setupStream)
        fprintf(stderr, "setupStream=%d(%s)" "\n", setupStream, LMS_GetLastErrorMessage());

    // Setup ring buffer and thread
    ring_buffer_t ring;
    ring_buffer_init(&ring, RING_BUFFER_SIZE, SAMPLES_PER_BLOCK);

    struct { FILE *fp; ring_buffer_t *rb; } *ctx = malloc(sizeof(*ctx));
    ctx->fp = fp;
    ctx->rb = &ring;

#ifdef _WIN32
    HANDLE producer = (HANDLE)_beginthreadex(NULL, 0, producer_thread, ctx, 0, NULL);
#else
    pthread_t producer;
    pthread_create(&producer, NULL, producer_thread, ctx);
#endif

    LMS_StartStream(&tx_stream);

    int loop = 0;
    gettimeofday(&tv, NULL);
    last_status_time = tv.tv_sec;
    start_sec = tv.tv_sec;

    while (1) {
#ifdef _WIN32
        WaitForSingleObject(ring.mutex, INFINITE);
#else
        pthread_mutex_lock(&ring.mutex);
#endif
        s16iq_sample_s *block = ring_buffer_get_read_block(&ring);
        if (!block) {
            // Empty
            if (ring.done) {
#ifdef _WIN32
                ReleaseMutex(ring.mutex);
#else
                pthread_mutex_unlock(&ring.mutex);
#endif
                break;
            }
#ifdef _WIN32
            ReleaseMutex(ring.mutex);
            WaitForSingleObject(ring.not_empty, 100);
#else
            pthread_cond_wait(&ring.not_empty, &ring.mutex);
            pthread_mutex_unlock(&ring.mutex);
#endif
            continue;
        }

        // Send block to LimeSDR
#ifdef _WIN32
        ReleaseMutex(ring.mutex);
#else
        pthread_mutex_unlock(&ring.mutex);
#endif

        int sendStream = LMS_SendStream(&tx_stream, block, SAMPLES_PER_BLOCK, NULL, 1000);
        if (sendStream < 0)
            fprintf(stderr, "sendStream %d(%s)" "\n", sendStream, LMS_GetLastErrorMessage());

#ifdef _WIN32
        WaitForSingleObject(ring.mutex, INFINITE);
#else
        pthread_mutex_lock(&ring.mutex);
#endif
        ring_buffer_consume(&ring);
#ifdef _WIN32
        SetEvent(ring.not_full);
        ReleaseMutex(ring.mutex);
#else
        pthread_cond_signal(&ring.not_full);
        pthread_mutex_unlock(&ring.mutex);
#endif

        loop++;
        if (0 == (loop % 100)) {
            loop = 0;
            gettimeofday(&tv, NULL);
            if (tv.tv_sec != last_status_time) {
                last_status_time = tv.tv_sec;
                int running_time = tv.tv_sec - start_sec;
                int hours = running_time / 3600;
                int minutes = (running_time % 3600) / 60;
                int seconds = running_time % 60;
                lms_stream_status_t status;
                LMS_GetStreamStatus(&tx_stream, &status);
                fprintf(stderr, "%02d:%02d:%02d ; TX rate:%5.1lf MB/s\n", 
                    hours, minutes, seconds, status.linkRate / 1e6);
            }
        }
    }

    fprintf(stderr, "Stopping transmission...\n");

    LMS_StopStream(&tx_stream);
    LMS_DestroyStream(device, &tx_stream);

    ring_buffer_destroy(&ring);

    LMS_EnableChannel(device, LMS_CH_TX, channel, false);
    LMS_Close(device);

    if (fp && fp != stdin)
        fclose(fp);

    fprintf(stderr, "Transmission complete. Device closed.\n");

    return(0);
}
