/*
 * ------------------------------------------------------------------------------------------------------
 *  Arquivo : vent_mec_medthon_argo.ino
 *  Projeto : Ventilador mecânico para o Medthon
 * ------------------------------------------------------------------------------------------------------
 * Descrição: algorítmo utilizado pelo microcontrolador ESP32 para efetuar o controle total da respiração
 * do paciente utilizando a ventilação ciclada a volume
 * ------------------------------------------------------------------------------------------------------
 * Revisões : 
 *    Data        Versão  Autor           Descrição
 *    25/02/2024  1.0     Vitor Thompson  versão inicial
 *    26/02/2024  1.1     Vitor Thompson  Retirada da pressão alvo de expiração
 *    26/02/2024  1.2     Vitor Thompson  Adição da função espera
 *    26/02/2024  1.3     Vitor Thompson  Retirada da trava de seguraça da expiração 
 * ------------------------------------------------------------------------------------------------------
 */

// Parametros dos pinos do ESP32
#define PIN_EXPIRACAO  33 // Pino da válvula de expiração
#define PIN_INSPIRACAO 32 // Pino da válvula de inspiração
#define PIN_PRESSAO    34 // Pino da saída do sensor de pressão

// Parametros de pressão, valores na escala do sensor, sem realizar conversões
#define INS_PRESSAO_ALVO 2400 // Pressao alvo no final da inspiração
#define INS_PRESSAO_MAX  2800 // Pressao máxima durante a inspiração, quando ultrapassada ativa a válvula de expiração imediatamente
#define EXP_PRESSAO_ALVO 1900 // Pressao alvo no final da expiração
#define EXP_PRESSAO_MIN  1500 // Pressao mínima durante a expiração, quando abaixo dela ativa-se a válvula de inspiração imediatamente

// Parametros de tempo
#define RAZAO_I_E        1/3                               // razão entre o tempo gasto na inspiração versus o tempo gasto na expiração
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
 * até que a pressão alvo seja atingida. Ao final, retorna o valor 
 * de quanto tempo de inspiração foi gasto na abertura da válvula.
 */
unsigned long Inspiracao (unsigned long timer) {
  float somatorio_pressao;
  for (int n = 1; n < 200; n++) {
    somatorio_pressao += analogRead(PIN_PRESSAO);
  }
  float med_pressao = somatorio_pressao/200;

  while (med_pressao < INS_PRESSAO_ALVO) {
    somatorio_pressao = 0;
    for (int n = 1; n < 400; n++) {
      somatorio_pressao += analogRead(PIN_PRESSAO);
    }
    med_pressao = somatorio_pressao/400.0;

    Serial.print(TEMPO_INSPIRACAO/1000.00);
    Serial.print(" ");
    Serial.print((millis() - timer)/1000.00);
    Serial.print(" - INS - ");
    Serial.println(med_pressao);

    if (med_pressao < INS_PRESSAO_MAX) {
      digitalWrite(PIN_INSPIRACAO, HIGH);
      digitalWrite(PIN_EXPIRACAO, LOW);
    }
    else
      break;
  }
  return timer = millis() - timer;
}

/*
 * Recebe o instante de tempo atual. Abre válvula de expiração
 * até que o tempo de expiração seja atingido.
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

    digitalWrite(PIN_INSPIRACAO, LOW);
    digitalWrite(PIN_EXPIRACAO, HIGH);
  }
}

/*
 * Recebe o quanto do tempo de inspiração foi gasto dentro da
 * função de inspiração, caso o tempo for menor que o tempo de
 * inspiração, realiza o fechamento de ambas as válvulas até que
 * o tempo com as válvulas fechadas mais o tempo gasto para inspirar
 * seja correspondente ao tempo de inspiração desejado.
 */
void Espera (unsigned long timer) {
  if (timer < TEMPO_INSPIRACAO) {
    float timerEspera = 0;
    while (timerEspera < (TEMPO_INSPIRACAO - timer)) {
      Serial.println(" - ESPERA - ");

      digitalWrite(PIN_INSPIRACAO, LOW);
      digitalWrite(PIN_EXPIRACAO, LOW);

      timerEspera = millis() - timer;
    }
  }
}

void loop () {
  unsigned long timer = millis();
  timer = Inspiracao(timer);
  Espera(timer);
  timer = millis();
  Expiracao(timer);
}
