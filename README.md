Test ESP-WROOM-32 et PZEM-004T v3.0 pour la gestion de panneaux solaire 

lecture de la puissance de la maison et de la production solaire avec deux PZEM 004t

lecture et affichage sur une page web, possibilité d'ajouter un afficheur via I2C

affichage :
Watts et KW/h maison
Watts et KW/h solaire
Watts et KW/h solaire => EDF
Watts et KW/h EDF => Solaire

les PZEM sont à configuré un à un pour changer leurs adresses qui par défaut est 0x10, en 0x2 pour la maison et 0x3 pour le solaire

Le montage : 

![alt text](https://github.com/fifi82/pzem_ESP32/blob/main/image/montage.JPG)


Le câblage : 

![alt text](https://github.com/fifi82/pzem_ESP32/blob/main/image/cablage_pzem_esp32.png)


La page web, posibilité d'ajouter des boutons pour d'autres option : 

![alt text](https://github.com/fifi82/pzem_ESP32/blob/main/image/page_web.png)


ide arduino (ESP32-WROOM-DA Module), la carte à choisir : 

![alt text](https://github.com/fifi82/pzem_ESP32/blob/main/image/ide_arduino.png)



