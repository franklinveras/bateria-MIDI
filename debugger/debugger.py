# Tarefas:
#	 Implementar as demais mensagens MIDI (atualmente apenas NOTE_ON)

import serial
from colors import bcolors

R  = bcolors.RED
G  = bcolors.GREEN
NC = bcolors.NOCOLOR

ser = serial.Serial('/dev/ttyACM2', 115200)


def Parse (msg): 
	
	# Mensagem midi: mmmmcccc ppppppp 0vvvvvv
	# onde: m = mensagem , c = canal, p = nota(pitch), v = velocity(intensidade)

	canal = (ord(msg[0]) & 0b1111) + 1
	mensagem = ord(msg[0]) >> 4
	pitch = ord(msg[1])
	velocidade = ord(msg[2])

	if velocidade > 0:
		print "MIDI ch.%02d Note %3i %s On%s , vel %3i" %(canal,  pitch, G, NC, velocidade)
	else:
		print "MIDI ch.%02d Note %3i %sOff%s" %(canal, pitch, R, NC,)


while True:
	mensagem = ''
	for msg_part in xrange(3):
		mensagem +=  ser.read()
	Parse(mensagem)