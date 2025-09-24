#include <DHT.h>  // Biblioteca para o sensor de umidade DHT22

// ========== DEFINIÇÃO DOS PINOS ==========
#define PINO_FOSFORO 32      // Botão para Fósforo (P)
#define PINO_POTASSIO 33     // Botão para Potássio (K)
#define PINO_NITROGENIO 35   // Botão para Nitrogênio (N)
#define PINO_PH 34           // Entrada analógica do sensor LDR (simula pH)
#define PINO_DHT 4           // Entrada digital do sensor DHT22 (umidade)
#define PINO_RELE 27         // Saída digital para acionar a bomba (relé)

// ========== INICIALIZAÇÃO DO SENSOR DHT22 ==========
#define DHTTYPE DHT22
DHT dht(PINO_DHT, DHTTYPE);  // Criação do objeto sensor DHT ligado ao pino 4

// ========== PARÂMETROS DA CULTURA (EX: TOMATE) ==========
const float UMIDADE_MINIMA = 50.0;    // Umidade abaixo ou igual a este valor é baixa
const float PH_IDEAL_MIN = 5.5;       // pH ideal para tomate (5.5 - 7.5)
const float PH_IDEAL_MAX = 7.5;

// === NOVO: VARIAVEL GLOBAL PARA O COMANDO EXTERNO ===
// Ela não é mais redefinida a cada loop e "lembra" o comando
int comando_externo = -1; // -1 = sem comando

void setup() {
  Serial.begin(115200);      // Inicializa o monitor serial
  dht.begin();               // Inicializa o sensor DHT22

  // Mensagem Boas-vindas
  Serial.println("=================================");
  Serial.println("  SISTEMA DE IRRIGACAO AUTOMATICA");
  Serial.println("=================================");
  delay(2000);

  // Configura os pinos de entrada e saida
  pinMode(PINO_FOSFORO, INPUT_PULLUP);
  pinMode(PINO_POTASSIO, INPUT_PULLUP);
  pinMode(PINO_NITROGENIO, INPUT_PULLUP);
  pinMode(PINO_RELE, OUTPUT);
  
  // Garante que a bomba inicie desligada (rele liga em LOW, entao comeca em HIGH)
  digitalWrite(PINO_RELE, HIGH);  
}

void loop() {
  // ========== 1. LEITURA DOS SENSORES E BOTOES ==========
  bool fosforoPresente = !digitalRead(PINO_FOSFORO);
  bool potassioPresente = !digitalRead(PINO_POTASSIO);
  bool nitrogenioPresente = !digitalRead(PINO_NITROGENIO);

  int valorLDR = analogRead(PINO_PH);
  float ph = map(valorLDR, 0, 4095, 200, 1400) / 100.0;
  
  // Garante que uma leitura de umidade válida seja obtida antes de continuar
  float umidade = dht.readHumidity();
  while (isnan(umidade) || umidade > 100) {
    delay(100); // Pequena pausa para o sensor
    umidade = dht.readHumidity();
  }

  // ========== 2. VERIFICAÇÃO DAS CONDIÇÕES ==========
  bool umidadeBaixa = umidade <= UMIDADE_MINIMA;
  bool phIdeal = (ph >= PH_IDEAL_MIN && ph <= PH_IDEAL_MAX);
  
  bool fosforoSuficiente = fosforoPresente;
  bool potassioSuficiente = potassioPresente;
  bool nitrogenioSuficiente = nitrogenioPresente;
  
  // A bomba liga somente se todas as condições estiverem ideais
  bool ligarBomba = umidadeBaixa && phIdeal && fosforoSuficiente && potassioSuficiente && nitrogenioSuficiente;
  
  // ============ INTEGRAÇÃO COM API EXTERNA ============
  // Lê o Monitor Serial para novos comandos
  if (Serial.available() > 0) {
      char comando = Serial.read();
      if (comando == '1') {
          comando_externo = 1; // Comando para NÃO irrigar (previsão de chuva)
          Serial.println(">>> COMANDO EXTERNO RECEBIDO: Irrigacao suspensa.");
      } else if (comando == '0') {
          comando_externo = 0; // Comando para PODE irrigar
          Serial.println(">>> COMANDO EXTERNO RECEBIDO: Irrigacao liberada.");
      }
  }

  // A lógica final para ligar a bomba considera o comando externo
  if (comando_externo == 1) { // Se houver comando de chuva
      ligarBomba = false; // Ignora as condicoes dos sensores e desliga a bomba
  }
  
  // ========== 3. CONTROLE DO RELE ==========
  if (ligarBomba) {
    digitalWrite(PINO_RELE, LOW);  // Liga a bomba
  } else {
    digitalWrite(PINO_RELE, HIGH); // Desliga a bomba
  }
  
  // ========== 4. IMPRESSAO NO MONITOR SERIAL ==========
  Serial.println("=====================");
  Serial.print("Umidade: "); Serial.print(umidade, 1); Serial.print("%");
  Serial.print(" | pH: "); Serial.print(ph, 2);
  Serial.print(" | Fósforo: "); Serial.print(fosforoPresente ? "Presente" : "Ausente");
  Serial.print(" | Potássio: "); Serial.print(potassioPresente ? "Presente" : "Ausente");
  Serial.print(" | Nitrogênio: "); Serial.println(nitrogenioPresente ? "Presente" : "Ausente");

  // Mostra a razão da decisao
  if (comando_externo == 1) {
    Serial.println(">>> Irrigacao suspensa devido a previsao de chuva.");
  } else if (ligarBomba) {
    Serial.println("CONDICOES IDEAIS PARA IRRIGAR. BOMBA LIGADA!");
  } else {
    Serial.println("IRRIGACAO NAO NECESSARIA/RECOMENDADA. Motivos:");
    if (!umidadeBaixa) {
      Serial.println("  - Umidade suficiente.");
    }
    if (!phIdeal) {
      Serial.println("  - pH fora da faixa ideal.");
    }
    if (!nitrogenioSuficiente) {
      Serial.println("  - Nitrogênio ausente.");
    }
    if (!fosforoSuficiente) {
      Serial.println("  - Fósforo ausente.");
    }
    if (!potassioSuficiente) {
      Serial.println("  - Potássio ausente.");
    }
  }

  delay(3000); // Espera 3 segundos antes da proxima leitura
}