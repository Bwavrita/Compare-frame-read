#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <stdio.h>
#include <time.h> 

int process_rtsp_stream(const char *url, int n_frames) {
    AVFormatContext *formatCtx = NULL;
    AVCodecContext *codecCtx = NULL;
    AVFrame *frame = NULL;
    AVPacket packet;
    struct SwsContext *swsCtx = NULL;

    // Inicializar as bibliotecas, padroes para ler frame (ffmpeg)
    av_log_set_level(AV_LOG_DEBUG);
    avformat_network_init();

    // Configurar opções para o stream, como o tipo de transporte
    AVDictionary *options = NULL;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    clock_t start_time = clock();

    // Fazer a conexão rtsp para começar a leitura de frames
    if (avformat_open_input(&formatCtx, url, NULL, &options) != 0) {
        av_dict_free(&options);
        return 0;
    }
    av_dict_free(&options);

    // Reber formato de video da camera
    if (avformat_find_stream_info(formatCtx, NULL) < 0) {
        avformat_close_input(&formatCtx);
        return 0;
    }

    // Encontrar o stream de vídeo
    int videoStreamIndex = 0;
    for (int i = 0; i < formatCtx->nb_streams; i++) {
        if (formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }
    if (videoStreamIndex == 0) {
        avformat_close_input(&formatCtx);
        return 0;
    }

    // Encontrar o codec necessário para decodificar o stream de vídeo
    const AVCodec *codec = avcodec_find_decoder(formatCtx->streams[videoStreamIndex]->codecpar->codec_id);
    if (!codec) {
        // Se o codec não for encontrado, encerra a função com erro
        avformat_close_input(&formatCtx);
        return 0;
    }

    // Alocar um contexto para o codec
    codecCtx = avcodec_alloc_context3(codec);
    if (!codecCtx) {
        // Se falhar na alocação do contexto, encerra com erro
        avformat_close_input(&formatCtx);
        return 0;
    }

    // Copiar os parâmetros do codec do stream para o contexto do codec
    if (avcodec_parameters_to_context(codecCtx, formatCtx->streams[videoStreamIndex]->codecpar) < 0) {
        // Em caso de erro, libera o contexto do codec e encerra
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return 0;
    }

    // Abrir o codec para preparar a decodificação
    if (avcodec_open2(codecCtx, codec, NULL) < 0) {
        // Se não conseguir abrir o codec, libera os recursos e encerra
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return 0;
    }

    // Alocar memória para armazenar os frames decodificados
    frame = av_frame_alloc();
    if (!frame) {
        // Se a alocação falhar, libera os recursos e encerra
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return 0;
    }

    // Configurar o contexto de conversão de formato de imagem (de YUV para RGB)
    swsCtx = sws_getContext(
        codecCtx->width, codecCtx->height, codecCtx->pix_fmt, // Resolução e formato de origem
        codecCtx->width, codecCtx->height, AV_PIX_FMT_RGB24,  // Resolução e formato de destino
        SWS_BICUBIC, NULL, NULL, NULL // Usando o algoritmo de redimensionamento bicúbico
    );

    if (!swsCtx) {
        // Se o contexto de conversão falhar, libera os recursos e encerra
        av_frame_free(&frame);
        avcodec_free_context(&codecCtx);
        avformat_close_input(&formatCtx);
        return 0;
    }

    // Inicializar a variável para contar os frames lidos
    int frames_read = 0;

    // Ler múltiplos frames do stream
    while (frames_read < n_frames && av_read_frame(formatCtx, &packet) >= 0) {
        // Verificar se o pacote pertence ao stream de vídeo
        if (packet.stream_index == videoStreamIndex) {
            // Enviar o pacote para o codec
            if (avcodec_send_packet(codecCtx, &packet) == 0) {
                // Receber e processar frames decodificados
                while (avcodec_receive_frame(codecCtx, frame) == 0) {
                    frames_read++; // Incrementar o contador de frames lidos
                }
            }
        }
        // Liberar os dados do pacote para reutilização
        av_packet_unref(&packet);
    }

    // Medir o tempo: fim
    clock_t end_time = clock(); // Registrar o tempo após a execução
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC; // Calcular o tempo decorrido
    printf("Tempo de execução: %.2f segundos.\n", elapsed_time);

    av_frame_free(&frame);               // Liberar memória do frame
    avcodec_free_context(&codecCtx);     // Liberar contexto do codec
    avformat_close_input(&formatCtx);    // Fechar o arquivo de entrada
    avformat_network_deinit();           // Finalizar o uso da rede pelo FFmpeg

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
