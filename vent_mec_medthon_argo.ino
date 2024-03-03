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
 * ------------------------------------------------------------------------------------------------------
 */

// Parametros dos pinos do ESP32
#define PIN_EXPIRACAO  33 // Pino da válvula de expiração
#define PIN_INSPIRACAO 32 // Pino da válvula de inspiração
#define PIN_PRESSAO    34 // Pino da saída do sensor de pressão

// Parametros para controle das valvulas
#define OPEN   1
#define CLOSED 0

// Parametros de pressão em centímetro de água [cmH2O]
#define INS_PRESSAO_ALVO_CMH2O 8  // Pressao alvo no final da inspiração
#define INS_PRESSAO_MAX_CMH2O  10 // Pressao máxima durante a inspiração, quando ultrapassada ativa a válvula de expiração imediatamente
#define EXP_PRESSAO_ALVO_CMH2O 3  // Pressao alvo no final da expiração
#define EXP_PRESSAO_MIN_CMH2O  1  // Pressao mínima durante a expiração, quando abaixo dela ativa-se a válvula de inspiração imediatamente

/* DESCOMENTE E ESSA SECÇÃO E COMENTE A PRÓXIMA CASO QUEIRA CONFIGURAR OS VALORES ALVOS DE PRESSÃO UTILIZANDO  OS PARAMETROS EM [cmH2O]
 * 
// Parametros de pressão, valores na escala do sensor, conversão de cmH2O para a escala do sensor
#define INS_PRESSAO_ALVO ((((INS_PRESSAO_ALVO_CMH2O / 70.31) + 1.0) * 2.0) + 0.5) * ((1000.0 * 33.0) / 51.0) 
#define INS_PRESSAO_MAX  ((((INS_PRESSAO_MAX_CMH2O / 70.31) + 1.0) * 2.0) + 0.5) * ((1000.0 * 33.0) / 51.0) 
#define EXP_PRESSAO_ALVO ((((EXP_PRESSAO_ALVO_CMH2O / 70.31) + 1.0) * 2.0) + 0.5) * ((1000.0 * 33.0) / 51.0) 
#define EXP_PRESSAO_MIN  ((((EXP_PRESSAO_MIN_CMH2O / 70.31) + 1.0) * 2.0) + 0.5) * ((1000.0 * 33.0) / 51.0) 
*/

// Parametros de pressão, valores na escala do sensor, sem realizar conversões
#define INS_PRESSAO_ALVO 2500 // Pressao alvo no final da inspiração
#define INS_PRESSAO_MAX  3800 // Pressao máxima durante a inspiração, quando ultrapassada ativa a válvula de expiração imediatamente
#define EXP_PRESSAO_ALVO 1800 // Pressao alvo no final da expiração
#define EXP_PRESSAO_MIN  1300 // Pressao mínima durante a expiração, quando abaixo dela ativa-se a válvula de inspiração imediatamente

// Parametros de tempo
#define RAZAO_I_E        1/3                               // razão entre o tempo gasto na inspiração versus o tempo gasto na expiração
#define CICLOS_POR_MIN   16                                // Quantidade de ciclos inpiração-expiração que são feitos em um minuto (normal: 14 a 20)
#define PERIODO_CICLO    (60000 / CICLOS_POR_MIN)          // [ms]
#define TEMPO_INSPIRACAO (PERIODO_CICLO * RAZAO_I_E)       // [ms]
#define TEMPO_EXPIRACAO  (PERIODO_CICLO * (1 - RAZAO_I_E)) // [ms]

// Porcentagem do tempo que as valvulas são ativadas (Duty Cycle)
// PWM feito para não sobreaquecer as valvulas
#define DUTY_CYCLE_MAX 205 // 205/256 = 0.8 (80%)
#define FREQ_PWM 5000

// Definem qual será o modo de acionamento das vávulas
#define INS_PWM_OU_DIGITAL 1 // 1 para utilizar o PWM e 0 para utilizar o digital
#define EXP_PWM_OU_DIGITAL 1 // 1 para utilizar o PWM e 0 para utilizar o digital


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

  // Configuração dos padrões do PWM
  analogWriteResolution(8);
  analogWriteFrequency (FREQ_PWM);
}

/* 
 * Faz o acionamento da válvula especificada no parametro pin,
 * abre ou fecha ela de acordo com o valor do parametro val,
 * utiliza o pwm ou o acionamento digital de acordo com os valores
 * definidos pelas constrantes INS_PWM_OU_DIGITAL e EXP_PWM_OU_DIGITAL.
 */
void writeValve (uint8_t pin, int val ) {
  if (pin == PIN_INSPIRACAO) {
    if (INS_PWM_OU_DIGITAL == 1) {
      if (val == OPEN)
        analogWrite(pin, DUTY_CYCLE_MAX);
      else
        analogWrite(pin, 0);
    }
    else {
      if (val == OPEN)
        digitalWrite(pin, HIGH);
      else
        digitalWrite(pin, LOW);
    }
  }
  else {
    if (EXP_PWM_OU_DIGITAL == 1) {
      if (val == OPEN)
        analogWrite(pin, DUTY_CYCLE_MAX);
      else
        analogWrite(pin, 0);
    }
    else {
      if (val == OPEN)
        digitalWrite(pin, HIGH);
      else
        digitalWrite(pin, LOW);
    }
  }
}

/*
 * Obtem a leitura do sensor de pressao, caso ela seja
 * maior que o limite máx. permitido imediatamente sai do loop.
 * Caso contrário abre a válv. de inpiração e fecha a de expiração
 * até que o tempo de inspiração e a pressão alvo sejam ambos atingidos.
 */
void Inspiracao (unsigned long timer) {
  float pressao = analogRead(PIN_PRESSAO);

  // POSSÍVEL MODIFICAÇÃO: talvez no projeto a pressão seja atingida muito antes do tempo
  // caso isso ocorrer, diminuir fluxo de ar ou retirar a dependencia do tempo
  while (((millis() - timer) < TEMPO_INSPIRACAO) || (pressao < INS_PRESSAO_ALVO)) {
    pressao = analogRead(PIN_PRESSAO);
    if (pressao < INS_PRESSAO_MAX) {
      writeValve(PIN_INSPIRACAO, OPEN);
      writeValve(PIN_EXPIRACAO , CLOSED);
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
  float pressao = analogRead(PIN_PRESSAO);

  // POSSÍVEL MODIFICAÇÃO: talvez no projeto a pressão seja atingida muito antes do tempo
  // caso isso ocorrer, diminuir o duty cycle da válv. de exp. ou retirar a dependencia do tempo
  while (((millis() - timer) < TEMPO_EXPIRACAO) || (pressao > EXP_PRESSAO_ALVO)) {
    if (pressao > EXP_PRESSAO_MIN) {
      writeValve(PIN_INSPIRACAO, CLOSED);
      writeValve(PIN_EXPIRACAO , OPEN);
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
