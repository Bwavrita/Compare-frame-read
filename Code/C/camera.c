#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <stdio.h>
#include <time.h> // Adicionado para medir o tempo

int process_rtsp_stream(const char *url, int n_frames) {
    AVFormatContext *formatCtx = NULL;
    AVCodecContext *codecCtx = NULL;
    AVFrame *frame = NULL;
    AVPacket packet;
    struct SwsContext *swsCtx = NULL;

    // Inicializar as bibliotecas
    av_log_set_level(AV_LOG_DEBUG);
    avformat_network_init();

    // Configurar opções para o stream
    AVDictionary *options = NULL;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);

    // Medir o tempo: início
    clock_t start_time = clock();

    // Abrir o stream RTSP
    if (avformat_open_input(&formatCtx, url, NULL, &options) != 0) {
        av_dict_free(&options);
        return -1;
    }
    av_dict_free(&options);

    // Encontrar informações do stream
    if (avformat_find_stream_info(formatCtx, NULL) < 0) {
        avformat_close_input(&formatCtx);
        return -1;
    }

    // Encontrar o stream de vídeo
    int videoStreamIndex = -1;
    for (int i = 0; i < formatCtx->nb_streams; i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == -1) {
        avformat_close_input(&formatCtx);
        return -1;
    }

    // Configurar o codec
    const AVCodec *codec = avcodec_find_decoder(formatCtx->streams[videoStreamIndex]->codecpar->codec_id);
    if (!codec) {
        avformat_close_input(&formatCtx);
        return -1;
    }

    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        avformat_close_input(&formatCtx);
        return -1;
    }

    if (avcodec_parameters_to_context(codecCtx, formatCtx->streams[videoStreamIndex]->codecpar) < 0) {
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return -1;
    }

    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return -1;
    }

    frame = av_frame_alloc();
    if (!frame) {
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return -1;
    }

    swsCtx = sws_getContext(
        codecCtx->width, codecCtx->height, codecCtx->pix_fmt, // Origem
        codecCtx->width, codecCtx->height, AV_PIX_FMT_RGB24,  // Destino
        SWS_BICUBIC, NULL, NULL, NULL
    );

    if (!swsCtx) {
        av_frame_free(&frame);
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return -1;
    }

    int frames_read = 0;
    // Ler múltiplos frames do stream
    while (frames_read < n_frames && av_read_frame(formatCtx, &packet) >= 0) {
        if (packet.stream_index == videoStreamIndex) {
            if (avcodec_send_packet(codecCtx, &packet) == 0) {
                while (avcodec_receive_frame(codecCtx, frame) == 0) {
                    frames_read++;
                }
            }
        }
        av_packet_unref(&packet);
    }

    // Medir o tempo: fim
    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Tempo de execução: %.2f segundos.\n", elapsed_time);

    // Liberar recursos
    av_frame_free(&frame);
    avcodec_free_context(&codecCtx);
    avformat_close_input(&formatCtx);
    avformat_network_deinit();

    return frames_read;
}

int main(int argc, char *argv[]) {

    int n_frames = atoi(argv[1]);
    const char *url = "rtsp://nautec:nautec@2024@191.37.251.151:8084/cam/realmonitor?channel=1&subtype=0";

    if (process_rtsp_stream(url, n_frames)) {
        printf("Processados %d frames com sucesso.\n", n_frames);
    } else {
        printf("Erro ao processar os frames.\n");
    }

    return 0;
}
