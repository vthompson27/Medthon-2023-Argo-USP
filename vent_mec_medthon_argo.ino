/*
 * ------------------------------------------------------------------------------------------------------
 *  Arquivo : vent_mec_medthon_argo.ino
 *  Projeto : Ventilador mecânico para o Medthon
 * ------------------------------------------------------------------------------------------------------
 * Descrição: algorítmo utilizado pelo microcontrolador ESP32 para efetuar o controle total da respiração
 * do paciente utilizando a ventilação ciclada a volume
 * ------------------------------------------------------------------------------------------------------
 * Revisões : 
 *    Data        Versão  Autor            Descrição
 *    25/02/2024  1.0     Vitor Thompson   versão inicial
 *    26/02/2024  1.1     Vitor Thompson   Retirada da pressão alvo de expiração
 *    26/02/2024  1.2     Vitor Thompson   Adição da função espera
 *    26/02/2024  1.3     Vitor Thompson   Retirada da trava de seguraça da expiração 
 *    27/02/2024  1.4     Laura Montenegro Conversao de cmH2O para escala do sensor
 *    27/02/2024  1.5     Leo Azevedo      Adição do Trigger
 *    27/02/2024  1.6     Vitor Thompson   Refatoração da função espera
 * ------------------------------------------------------------------------------------------------------
 */

// Parametros dos pinos do ESP32
#define PIN_EXPIRACAO  33 // Pino da válvula de expiração
#define PIN_INSPIRACAO 32 // Pino da válvula de inspiração
#define PIN_PRESSAO    34 // Pino da saída do sensor de pressão

// Parametros de pressão em centímetro de água [cmH2O]
#define INS_PRESSAO_ALVO_CMH2O 10 // Pressao alvo no final da inspiração
#define INS_PRESSAO_MAX_CMH2O  30 // Pressao máxima durante a inspiração, quando ultrapassada ativa a válvula de expiração imediatamente
#define EXP_PRESSAO_ALVO_CMH2O 3  // Pressao alvo no final da expiração
#define EXP_PRESSAO_MIN_CMH2O  1  // Pressao mínima durante a expiração, quando abaixo dela ativa-se a válvula de inspiração imediatamente
  
// Parametros de pressão, valores na escala do sensor, conversão de cmH2O para a escala do sensor
#define INS_PRESSAO_ALVO ((((INS_PRESSAO_ALVO_CMH2O / 70.31) + 1.0) * 2.08) + 0.6) * ((4095) / 5) 
#define INS_PRESSAO_MAX  ((((INS_PRESSAO_MAX_CMH2O / 70.31) + 1.0) * 2.08) + 0.6) * ((4095 ) / 5) 
#define EXP_PRESSAO_ALVO ((((EXP_PRESSAO_ALVO_CMH2O / 70.31) + 1.0) * 2.08) + 0.6) * ((4095 * 3.2) / 51.0) 
#define EXP_PRESSAO_MIN  ((((EXP_PRESSAO_MIN_CMH2O / 70.31) + 1.0) * 2.0) + 0.6) * ((1000.0 * 33.0) / 51.0) 

// Parametros de tempo
#define RAZAO_I_E        1/2                               // razão entre o tempo gasto na inspiração versus o tempo gasto na expiração
#define CICLOS_POR_MIN   20                                // Quantidade de ciclos inpiração-expiração que são feitos em um minuto (normal: 14 a 20)
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
  int trigger = 0;

  float somatorio_pressao;
  for (int n = 1; n < 200; n++) {
    somatorio_pressao += analogRead(PIN_PRESSAO);
  }
  float med_pressao = somatorio_pressao/200;

  while (trigger < 100){
    somatorio_pressao = 0;
    for (int n = 1; n < 150; n++) {
      somatorio_pressao += analogRead(PIN_PRESSAO);
    }
    med_pressao = somatorio_pressao/150.0;
    
    Serial.print(TEMPO_INSPIRACAO/1000.00);
    Serial.print(" ");
    Serial.print((millis() - timer)/1000.00);
    Serial.print(" - INS - P_medida: ");
    Serial.print(med_pressao);
    Serial.print(" P_alvo: ");
    Serial.println(INS_PRESSAO_ALVO);

    if (med_pressao > INS_PRESSAO_ALVO) {
      trigger++;
    }
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
  while (((millis() - timer) < TEMPO_EXPIRACAO)) {
    Serial.print(TEMPO_EXPIRACAO/1000.00);
    Serial.print(" ");
    Serial.print((millis() - timer)/1000.00);
    Serial.print(" - EXP - ");
    Serial.println(analogRead(PIN_PRESSAO));

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
  float timer_aux = millis();

  if (timer < TEMPO_INSPIRACAO) {
    float timerEspera = 0;
    float delta = TEMPO_INSPIRACAO - timer;

    while (timerEspera < (timer + delta)) {
      Serial.print(timerEspera);
      Serial.print(" - ESPERA - ");
      Serial.println(timer + delta);

      digitalWrite(PIN_INSPIRACAO, LOW);
      digitalWrite(PIN_EXPIRACAO, LOW);

      timerEspera = (millis() - timer_aux) - timerEspera;
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
