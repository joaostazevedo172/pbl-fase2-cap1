import requests

# Insira sua chave de API e o nome da cidade diretamente aqui
api_key = "6bc7b8e59e999da4d06109a69b31fbcf"
cidade = "Sao Paulo"

# URL da API para a cidade especifica
url = f"http://api.openweathermap.org/data/2.5/weather?q={cidade}&appid={api_key}"

try:
    response = requests.get(url)

    # Verifica o status da resposta HTTP para erros comuns
    response.raise_for_status()
    dados = response.json()

    # Logica de condicoes climaticas mais robusta usando os codigos
    condicoes_chuvosas = range(200, 600)  # Códigos de 2xx (tempestades), 3xx (chuvisco), 5xx (chuva)

    chovendo = False
    if "weather" in dados:
        for item in dados["weather"]:
            weather_id = item.get("id")
            if weather_id in condicoes_chuvosas:
                chovendo = True
                break
    
    # Imprime a decisao para ser lida manualmente pelo ESP32
    print("-" * 20)
    if chovendo:
        print("Previsao: CHUVA")
        print("Decisao: NAO IRRIGAR")
        print("Comando para o Wokwi: 1")
    else:
        print("Previsao: SEM CHUVA")
        print("Decisao: PODE IRRIGAR")
        print("Comando para o Wokwi: 0")
    print("-" * 20)

except requests.exceptions.RequestException as err:
    print(f"Erro na requisicao: {err}")
except Exception as e:
    print(f"Ocorreu um erro: {e}")