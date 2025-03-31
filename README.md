Test ESP-WROOM-32 et PZEM-004T v3.0 pour la gestion de panneaux solaire <br>

lecture de la puissance de la maison et de la production solaire avec deux PZEM 004t<br>

lecture et affichage sur une page web, possibilité d'ajouter un afficheur via I2C<br>

affichage :<br>
Watts et KW/h maison<br>
Watts et KW/h solaire<br>
Watts et KW/h solaire => EDF<br>
Watts et KW/h EDF => Solaire<br>

un enregistrement est effectué tous les jour à minuit dans la EEprom de l'esp, 31 enregistrements.<br>


les PZEM sont à configuré un à un pour changer leurs adresses qui par défaut est 0x10, en 0x2 pour la maison et 0x3 pour le solaire<br>

<br>
pzem_esp_v0.4.ino :<br>
le nom du wifi et le mot de passe sont à configurer via le terminal ide arduino<br>
la vitesse est de 921600 bauds<br>
tapez 'reboot' pour reconnecter l'esp32<br>
tapez 'nom wifi' pour afficher le nom du réseau wifi<br>
tapez 'ssid nom_wifi mot_de_passe' pour se connecter à un réseau wifi<br>
<br>
Le montage : <br>

![alt text](https://github.com/fifi82/pzem_ESP32/blob/main/image/montage.JPG)

<br>
Le câblage : <br>

![alt text](https://github.com/fifi82/pzem_ESP32/blob/main/image/cablage_pzem_esp32.png)

<br>
La page web, posibilité d'ajouter des boutons pour d'autres option : <br>

![alt text](https://github.com/fifi82/pzem_ESP32/blob/main/image/page_web.png)

<br>
ide arduino (ESP32-WROOM-DA Module), la carte à choisir : <br>

![alt text](https://github.com/fifi82/pzem_ESP32/blob/main/image/ide_arduino.png)



