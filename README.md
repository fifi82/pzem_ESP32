Test ESP32 et PZEM pour la gestion de panneaux solaire 

lecture de la puissance de la maison et de la production solaire avec deux PZEM 004t

lecture et affichage sur une page web, possibilité d'ajouter un afficheur via I2C

Watts et KW/h maison
Watts et KW/h solaire
Watts et KW/h solaire => EDF
Watts et KW/h EDF => Solaire

les PZEM sont à configuré un à un pour changer leurs adresses, par défaut 0x10, en 0x2 pour la maison et 0x3 pour le solaire

Montage : 

![alt text](https://github.com/fifi82/pzem_ESP32/blob/main/cablage_pzem_esp32.png)


Câblage : 

![alt text](https://github.com/fifi82/pzem_ESP32/blob/main/cablage_pzem_esp32.png)


page web : 

![alt text](https://github.com/fifi82/pzem_ESP32/blob/main/page_web.png)


ide arduino (ESP32-WROOM-DA Module): 

![alt text](https://github.com/fifi82/pzem_ESP32/blob/main/ide_arduino.png)
