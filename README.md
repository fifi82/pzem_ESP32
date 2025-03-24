Test ESP32 et PZEM pour la gestion de panneaux solaire 

lecture de la puissance de la maison et de la production solaire avec deux PZEM 004t

lecture et affichage sur une page web, possibilité d'ajouter un afficheur via I2C

Watts et KW/h maison
Watts et KW/h solaire
Watts et KW/h solaire => EDF
Watts et KW/h EDF => Solaire

les PZEM sont à configurer un à un pour changer leurs adresses, par défaut c'est 0x10, en 0x2 pour la maison et 0x3 pour le solaire
