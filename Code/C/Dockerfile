# Usar uma imagem base do Ubuntu
FROM ubuntu:22.04

# Instalar dependências necessárias (compiladores, bibliotecas FFmpeg e outros pacotes)
RUN apt-get update && apt-get install -y \
    build-essential \
    wget \
    yasm \
    cmake \
    libavformat-dev \
    libavcodec-dev \
    libavutil-dev \
    libswscale-dev \
    libswresample-dev \
    libavdevice-dev \
    libx264-dev \
    ffmpeg \
    bash \
    procps

# Criar um diretório para o código fonte
WORKDIR /app

# Copiar o código C para dentro do contêiner
COPY camera.c /app/camera.c



# Compilar o código C
RUN gcc -o camera camera.c -lavformat -lavcodec -lavutil -lswscale -lpthread

# Comando para rodar o script de stats e o programa
CMD ["/bin/bash", "-c", " ./camera"]

