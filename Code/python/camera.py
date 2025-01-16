import cv2
import time
import argparse

def camera(ip_camera_url, qtd_frame):
    # Iniciar a medição do tempo
    start_time = time.time()

    cap = cv2.VideoCapture(ip_camera_url)

    if not cap.isOpened():
        print("Erro: Não foi possível abrir o stream de vídeo.")
        return 0

    count_frame = 0
    while count_frame < qtd_frame:
        ret, frame = cap.read()

        if not ret:
            print("Erro: Não foi possível ler o frame.")
            cap.release()
            return 0
        count_frame +=1

    end_time = time.time()
    elapsed_time = end_time - start_time

    print(f"Tempo de execução: {elapsed_time:.2f} segundos.")
    cap.release()
    return 1

def main():
    parser = argparse.ArgumentParser(description="Processar um número específico de frames de uma câmera RTSP.")
    parser.add_argument("qtd_frames", type=int, help="Número de frames a serem processados")
    
    args = parser.parse_args()

    qtd_frames = args.qtd_frames

    if(camera("rtsp://nautec:nautec@2024@191.37.251.151:8084/cam/realmonitor?channel=3&subtype=0", qtd_frames)):
        print(f"{qtd_frames} frames lidos com sucesso")
    else:
        print(f"Erro ao tentar ler {qtd_frames} frames")

if __name__ == "__main__":
    main()
