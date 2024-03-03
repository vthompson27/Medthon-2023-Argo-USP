/*
 * ------------------------------------------------------------------------------------------------------
 *  Arquivo : vent_mec_medthon_argo_v.ino
 *  Projeto : Ventilador mecânico para o Medthon
 * ------------------------------------------------------------------------------------------------------
 * Descrição: algorítmo utilizado pelo microcontrolador ESP32 para efetuar o controle total da respiração
 * do paciente utilizando a ventilação ciclada a volume
 * ------------------------------------------------------------------------------------------------------
 * Revisões : 
 *    Data        Versão  Autor           Descrição
 *    25/02/2024  1.0     Vitor Thompson  versão inicial
 *    26/02/2024  1.1     Vitor Thompson  Retirada da pressão alvo de expiração
 * ------------------------------------------------------------------------------------------------------
 */

// Parametros dos pinos do ESP32
#define PIN_EXPIRACAO  33 // Pino da válvula de expiração
#define PIN_INSPIRACAO 32 // Pino da válvula de inspiração
#define PIN_PRESSAO    34 // Pino da saída do sensor de pressão

// Parametros de pressão, valores na escala do sensor, sem realizar conversões
#define INS_PRESSAO_ALVO 2200 // Pressao alvo no final da inspiração
#define INS_PRESSAO_MAX  3600 // Pressao máxima durante a inspiração, quando ultrapassada ativa a válvula de expiração imediatamente
#define EXP_PRESSAO_MIN  1500 // Pressao mínima durante a expiração, quando abaixo dela ativa-se a válvula de inspiração imediatamente

// Parametros de tempo
#define RAZAO_I_E        1/3                              // razão entre o tempo gasto na inspiração versus o tempo gasto na expiração
#define CICLOS_POR_MIN   16                                // Quantidade de ciclos inpiração-expiração que são feitos em um minuto (normal: 14 a 20)
#define PERIODO_CICLO    (60000 / CICLOS_POR_MIN)          // [ms]
#define TEMPO_INSPIRACAO (PERIODO_CICLO * RAZAO_I_E)       // [ms]
#define TEMPO_EXPIRACAO  (PERIODO_CICLO * (1 - RAZAO_I_E)) // [ms]

void setup () {
  // Inicializa print serial
  Serial.begin  (115200);
  Serial.println("Ventilador mecanico didatico");

  // Entrada para receber a leitura do sensor
  pinMode(PIN_PRESSAO, INPUT);
  // Saida para controlar Solenoide de Expiracao
  pinMode(PIN_EXPIRACAO, OUTPUT);
  // Saida para controlar Solenoide de Inspiracao
  pinMode(PIN_INSPIRACAO, OUTPUT);
}

/*
 * Obtem a leitura do sensor de pressao, caso ela seja
 * maior que o limite máx. permitido imediatamente sai do loop.
 * Caso contrário abre a válv. de inpiração e fecha a de expiração
 * até que o tempo de inspiração e a pressão alvo sejam ambos atingidos.
 */
void Inspiracao (unsigned long timer) {
  float pressao = analogRead(PIN_PRESSAO);

  Serial.print(TEMPO_INSPIRACAO/1000.00);
  Serial.print(" ");
  Serial.print((millis() - timer)/1000.00);
  Serial.print(" - INS - ");
  Serial.println(pressao);

  while ((millis() - timer) < TEMPO_INSPIRACAO) {
    pressao = analogRead(PIN_PRESSAO);

    Serial.print(TEMPO_INSPIRACAO/1000.00);
    Serial.print(" ");
    Serial.print((millis() - timer)/1000.00);
    Serial.print(" - INS - ");
    Serial.println(pressao);

    while (pressao < INS_PRESSAO_ALVO) {
      pressao = analogRead(PIN_PRESSAO);

      Serial.print(TEMPO_INSPIRACAO/1000.00);
      Serial.print(" ");
      Serial.print((millis() - timer)/1000.00);
      Serial.print(" - INS - ");
      Serial.println(pressao);

      if (pressao < INS_PRESSAO_MAX) {
        digitalWrite(PIN_INSPIRACAO, HIGH);
        digitalWrite(PIN_EXPIRACAO, LOW);
      }
      else
        break;
    }
    if (pressao < INS_PRESSAO_MAX) {
        digitalWrite(PIN_INSPIRACAO, LOW);
        digitalWrite(PIN_EXPIRACAO, LOW);
      }
      else
        break;
  }
}

/*
 * Obtem a leitura do sensor de pressao, caso ela seja
 * menor que o limite mín. permitido imediatamente sai do loop.
 * Caso contrário fecha a válv. de inpiração e abre a de expiração
 * até que o tempo de expiração e a pressão alvo sejam ambos atingidos.
 */
void Expiracao (unsigned long timer) {
  float pressao;

  while (((millis() - timer) < TEMPO_EXPIRACAO)) {
    pressao = analogRead(PIN_PRESSAO);

    Serial.print(TEMPO_EXPIRACAO/1000.00);
    Serial.print(" ");
    Serial.print((millis() - timer)/1000.00);
    Serial.print(" - EXP - ");
    Serial.println(pressao);
    
    if (pressao > EXP_PRESSAO_MIN) {
      digitalWrite(PIN_INSPIRACAO, LOW);
      digitalWrite(PIN_EXPIRACAO, HIGH);
    }
    else
      break;
  }
}

void loop () {
  unsigned long timer = millis();
  Inspiracao(timer);
  timer = millis();
  Expiracao(timer);
}
