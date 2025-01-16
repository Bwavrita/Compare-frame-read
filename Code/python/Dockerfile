# Escolhe uma imagem base com Python e OpenCV
FROM python:3.9-slim

# Instala dependências para OpenCV (como ffmpeg e outros pacotes necessários para captura de vídeo)
RUN apt-get update && apt-get install -y \
    libgl1-mesa-glx \
    libglib2.0-0 \
    libsm6 \
    libxext6 \
    libxrender-dev \
    ffmpeg

# Instala o OpenCV para Python
RUN pip install opencv-python

# Copia o código Python para o contêiner
COPY camera.py /app/camera.py

# Define o diretório de trabalho dentro do contêiner
WORKDIR /app

CMD ["/bin/bash", "-c", "python camera.py"]

