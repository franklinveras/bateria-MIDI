/*  
	Bateria Eletrônica MIDI Arduino
	Criado por: Franklin Véras Sertão
	Criado em: 21 de Dezembro de 2016
*/

/***********************************************************
	
	Tarefas:

	Reescrever em Programação orientada a objetos
	para que seja possível criar uma simples 
	biblioteca arduino


************************************************************/

const unsigned char NUM_PIN              = 8;

const unsigned long SOFTWARE_SERIAL_BAUD = 115200;
const unsigned long MIDI_BAUD            = 38400;

const unsigned char NOTE_ON              = 0b10010000;
const unsigned char NOTE_OFF             = 0b10000000;

enum tipo_de_entrada{
	VAZIO,
	PAD,
	BUMBO,
	HIHAT,
	PDAF
};

/***********************************************************
	
	Variáveis definidas pelo usuário:

	Essas variáveis serão editáveis através dos botões
	e do display podendo ser salvas na EEPROM interna
	para resgate posterior

************************************************************/

byte     pino    [NUM_PIN] = {     4,     3,    A0,    A1,    A2,    A3,    A4,    A5}; 
boolean  ativo   [NUM_PIN] = {  true,  true,  true,  true, false, false, false, false}; // Habilita ou desabilita pads
byte     tipo    [NUM_PIN] = {  PDAF, BUMBO, HIHAT,   PAD, VAZIO, VAZIO, VAZIO, VAZIO};
byte     nota    [NUM_PIN] = {    44,    35,    42,    38,     0,     0,     0,     0}; // Valor da nota MIDI (C3 -> 60)  
int      limiar  [NUM_PIN] = {     0,     0,   100,   100,   100,   100,   100,   100}; // Valor mnimo da leitura analógica para disparar uma nota
byte     ataque  [NUM_PIN] = {    10,    40,     5,     5,     1,     1,     1,     1}; // Tempo em milissegundos para detectar o pico aps atingir o limiar 
byte     duracao [NUM_PIN] = {     0,     0,    20,    10,    30,    30,    30,    30}; // Duraxima da nota
byte     maxVel  [NUM_PIN] = {   127,   127,   127,   127,   127,   127,   127,   127}; // Intensidade máxima da nota MIDI


int           pico       [NUM_PIN] = {0,0,0,0,0,0,0,0};
boolean       noteOn     [NUM_PIN] = {0,0,0,0,0,0,0,0};
byte          estado     [NUM_PIN] = {0,0,0,0,0,0,0,0};
unsigned long cronometro [NUM_PIN] = {0,0,0,0,0,0,0,0};


//Variáveis relativas ao controle do hihat
byte pdaf_pin    = 0;
byte hihat_pin   = 0;
byte hihat_pico  = 0;
byte hihat_tempo = 40;
unsigned long hihat_cronometro=0;

void setup() {

	Serial.begin(SOFTWARE_SERIAL_BAUD);

	for(unsigned char pin=0; pin<NUM_PIN; pin++){
		pinMode(pino[pin], INPUT);
		if (tipo[pin] == PDAF) pdaf_pin = pin; //detecta se há um Pedal de hihat declarado
		if (tipo[pin] == HIHAT) hihat_pin = pin;
		pinMode(9, OUTPUT); //LED  - Feedback das batidas
	}

}

void loop() {
	for(unsigned char pin=0; pin<NUM_PIN; pin++){
		if (ativo[pin]){
			int leitura;
			byte tp = tipo[pin];

			//Para pedal, faz-se o debounce com o tempo de <ataque>
			if ((tp == BUMBO)|(tp == PDAF)){

				leitura = digitalRead(pino[pin]);
				/* 
					noteOn[pin] para os pedais, representa que o sinal de uma determinada entrada
				 	mudou de estado e o cronômetro do debounce foi disparado
				*/
				if (noteOn[pin]){

					unsigned long agora = millis();
					unsigned long tempo_decorrido  = agora - cronometro[pin];

					if(tempo_decorrido > ataque[pin]){
						estado[pin] = leitura;
						byte vel = (leitura)?maxVel[pin]:0;
						MIDI_TX(NOTE_ON, nota[pin], vel);
					}

					if ((agora - hihat_cronometro) < hihat_tempo && tp == PDAF && !leitura) {
						MIDI_TX(NOTE_OFF, nota[hihat_pin], 0);
						MIDI_TX(NOTE_ON, nota[hihat_pin]+4, hihat_pico);
						noteOn[pin] = false;
					}

				}else if (leitura != estado[pin]){
					cronometro[pin]  = millis();
					noteOn[pin] = true;
				}
				
			}else{

				leitura  = analogRead(pino[pin]);

				/* 
					noteOn[pin] para os Pads, representa que o sinal de uma determinada
				 	entrada superou o limiar e o cronômetro do debounce foi disparado.
				 	Para reconhecer o pico da vibração de um PAD, usa-se o tempo de ataque
					Para evitar fantasmas de nota, decorre-se o tempo de <duracao> antes que outra nota seja capturada
				*/
					if(noteOn[pin]){

						unsigned long agora = millis();
						unsigned long tempo_decorrido  = agora - cronometro[pin];

						if(tempo_decorrido < duracao[pin]){

							if(tempo_decorrido < ataque[pin]){
						
								if (leitura > pico[pin]) {
									pico[pin] = leitura;
								}
						
							}else{
						
								if(!estado[pin]){
								
									byte vel = pico[pin]/8 ;
									pico[pin] = 0;
								
								if (tipo[pin] == HIHAT){

									byte note = nota[pin] + ((estado[pdaf_pin])?0:4);
									MIDI_TX(NOTE_ON, note, vel);
									hihat_cronometro = millis();
									hihat_pico = vel;

								}else{

									MIDI_TX(NOTE_ON, nota[pin], vel);

								}
						
								estado[pin] = true;
						
							}
						}

					}else{

						estado[pin] = false;
						noteOn[pin] = false;
						MIDI_TX(NOTE_OFF, nota[pin], 0);
					
					}

				}else{

					if(leitura > limiar[pin]){
						noteOn[pin] = true;
						cronometro[pin]  = millis();
						pico[pin]   = 0;
					}

				}
			}
		}
	}
}

void MIDI_TX(byte MESSAGE, byte PITCH, byte VELOCITY) {

	analogWrite(9, VELOCITY);
	Serial.write(MESSAGE);
	Serial.write(PITCH);
	Serial.write(VELOCITY);
}
