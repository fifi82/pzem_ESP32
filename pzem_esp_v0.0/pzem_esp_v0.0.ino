

#include <BetterOTA.h>  // programme l'esp en wifi, https://github.com/radekrudak/BetterOTA
#include <WiFi.h>       // gestion du wifi
#include <NTPClient.h>  // pour hologe web
#include <WiFiUdp.h>    // pour hologe web
#include <PZEM004Tv30.h> // gestion des pzems
#include <EEPROM.h>     // EEprom pour la sauvegarde des datas

#define PZEM_RX 16 // Serial2 -> tx pzem vert
#define PZEM_TX 17 // Serial2 -> rx pzem blet

PZEM004Tv30 PZEM_solaire;//pzem[a].power()
PZEM004Tv30 PZEM_maison;

// pzem.resetEnergy();      // reset des kwh
// float kw = pzem.energy();  // kiloWatt/heure
// float u  = pzem.voltage(); // tension
// float i  = pzem.current(); // intencité
// float f  = pzem.frequency(); // fréquence
// float w  = pzem[a].power() // watt
// float pf = pzem.pf(); // cos phi

const char* ssid = "ssid";            // non du wifi
const char* password = "password";  // mot de passe du wifi

////PZEM004Tv30 pzem_solaire(14, 15);   // RX TX solaire
//PZEM004Tv30 pzem_maison(16, 17);   // RX TX maison 

WiFiServer server(80);  // canal wifi
WiFiUDP ntpUDP;                               // hologe web
NTPClient timeClient(ntpUDP, "pool.ntp.org"); // adresse de l'hologe web


String p; // p contiendra le code HTML

volatile unsigned long horloge;   // horloge en seconde 
volatile bool minuit;             // permet de faire la mise à jour de l'horloge à minuit
volatile bool pzem;  // pzem=bit pour la lecture des valeurs toute les secondes
volatile bool reset_pzem; // à minuit on fait un reset des kwh et mémorise les valeurs

unsigned long h2; 

// mem_dwh = déci-watt/heure val maxi 655,36 kw sur deux octets
uint16_t mem_dwh[32][6];   // mémorisation des valeurs, date,dwh_maison, dwh_solaire, dwh_solaire_EDF,dwh_EDF_maison

// puissance maison :
float w_maison = 0;       // consomation instantanée en watt
float wh_maison = 0;     // consomation cumulée sur un jour en kilo watt heure

// puissance solaire :
float w_solaire = 0;      // consomation instantanée en watt
float wh_solaire = 0;    // consomation cumulée sur un jour en kilo watt heure

// puissance solaire envoyée à EDF :
float w_solaire_EDF = 0;  // consomation instantanée en watt
float wh_solaire_EDF = 0;// consomation cumulée sur un jour en kilo watt heure

//  puissance consomée sur EDF :
float w_EDF_maison = 0;   // consomation instantanée en watt
float wh_EDF_maison = 0; // consomation cumulée sur un jour en kilo watt heure

// tention du réseau
float tension = 0;

hw_timer_t *timer = NULL; // définition du nom du timer

String t_mois[12]={"Janvier","Fevrier","Mars","Avril","Mai","Juin","Juillet","Aout","Septembre","Octobre","Novembre","Decembre"};
int m_jour,jour, mois, annee;       // jour mois année
int heure,minute,seconde;       // heure ESP

/*##################################################  setup  ###################################################*/
void setup() {

  Serial.begin(115200);
  EEPROM.begin(320);
 
  PZEM_maison  = PZEM004Tv30(Serial2, PZEM_RX, PZEM_TX, 0x2 );
  PZEM_solaire = PZEM004Tv30(Serial2, PZEM_RX, PZEM_TX, 0x3 );  

  // configuration d'une intéruption timer de 1 seconde
  timer = timerBegin(1000000);                //timer 1Mhz resolution
  timerAttachInterrupt(timer, &une_seconde);  //attache la fonction au timer
  timerAlarm(timer, 1000 * 1000, true, 0);    // fixe le timer à 1 seconde

  WiFi.begin(ssid, password); // se connecte au wifi
  server.begin();             // initialise le server web

  pinMode(2, OUTPUT);       // led sur la pin 2 pour faire jolie

  OTACodeUploader.begin();  // permer de programmer l'esp par wifi
  OTATerminal.begin();      // permer de programmer l'esp par wifi
  
  timeClient.begin();       // server heure d'internet
  timeClient.setTimeOffset(3600); // 1h de décalage sur le GMT
  sychro_heure_web();       // lit l'heure depuis internet
  delay(1000);

    for ( int i=1; i<32; i++) {
      int ad = 10 * (i - 1); // adresse dans la EEprom
     mem_dwh[i][0] = EEPROM.readUShort( ad + 0 ); 
     mem_dwh[i][1] = EEPROM.readUShort( ad + 2 ); 
     mem_dwh[i][2] = EEPROM.readUShort( ad + 4 ); 
     mem_dwh[i][3] = EEPROM.readUShort( ad + 6 ); 
     mem_dwh[i][4] = EEPROM.readUShort( ad + 8 ); 
  }

}

/*##################################################  ISR une_seconde  ###################################################*/
void ARDUINO_ISR_ATTR une_seconde() {  // fonction ISR appelée toute les secondes

 horloge++; // horloge

 if (horloge > 86400 and !minuit) { // si c'est minuit
  minuit = true;      // mémorise que c'est minuit
  reset_pzem = true;  // demande de sauvegarde et reset des kwh
 }
 else if (horloge > 3600 and minuit) minuit = false; // à 1 heure du mat on reset la mémorisation de minuit
 // à faire la gestion d'heure d'été/hivers ou trouver un serveur heure de paris

 pzem = true;
  
}

/*##################################################  loop  ###################################################*/
void loop() {

  if (pzem){
    pzem = false; // bit antirebond pour la lecture toutes les secondes

    if (annee<2025) sychro_heure_web(); // si pas de synchro on recharge l'heure

    tension =   PZEM_maison.voltage(); // lecture de la tension électrique pour info
    w_maison  = PZEM_maison.power();   // lecture des watts de la maison
    w_solaire = PZEM_solaire.power();  // lecture des watts du solaire
    wh_maison  += w_maison  / 3600.0; // calcule les kwh de la maison
    wh_solaire += w_solaire / 3600.0; // calcule les kwh produit par le solaire
    float val = w_maison - w_solaire;  // regarde si on pred ou donne à EDF
    if (val < 0) {                      // solaire => EDF
      w_solaire_EDF = -val;              // calcul des watts donné à EDF
      wh_solaire_EDF += w_solaire_EDF/ 3600.0; // calcul les kw/h donné à EDF
      w_EDF_maison = 0;
    }
    else {                              // EDF => maison
      w_EDF_maison = val;          // calcul des watts consomé sur EDF     
      wh_EDF_maison += w_EDF_maison/ 3600.0; // calcul des kw/h consomé sur EDF 
      w_solaire_EDF = 0;
    }
    
    h2++;

    if (minuit and reset_pzem){ // à minuit pile sauvegarde et reset des kwh
      reset_pzem = false;
      int a = annee - 2000;
      mem_dwh[jour][0] = mois + a * 256;
      mem_dwh[jour][1] = wh_maison/10;
      mem_dwh[jour][2] = wh_solaire/10;
      mem_dwh[jour][3] = wh_solaire_EDF/10;
      mem_dwh[jour][4] = wh_EDF_maison/10;

      int ad =  10 * (jour - 1); // adresse dans la EEprom 10=nb de byte utilisé par jour
      EEPROM.writeUShort(0 + ad, mem_dwh[jour][0] );
      EEPROM.writeUShort(2 + ad, mem_dwh[jour][1] );
      EEPROM.writeUShort(4 + ad, mem_dwh[jour][2] );
      EEPROM.writeUShort(6 + ad, mem_dwh[jour][3] );
      EEPROM.writeUShort(8 + ad, mem_dwh[jour][4] );
      EEPROM.commit(); // valide l'écriture dans le eeprom
      wh_maison = wh_solaire = wh_solaire_EDF = wh_EDF_maison = 0;
      
      m_jour = jour;
      sychro_heure_web(); // recalle l'horloge interne avec l'heure du web

    }
  }

  BetterOTA.handle(); // permer de programmer l'esp par wifi 

  WiFiClient client = server.available();
  if (!client) { // si pas de connection
    return; // pas de requette
  }
    seconde = horloge      % 60; // calcul des secondes
    minute  = horloge/60   % 60; // calcul des minutes
    heure   = horloge/3600 % 24; // calcul des heures
  delay(10);

  String request = client.readStringUntil('\r'); // lit la requette du client
  client.flush(); // attend la fin de la connection 

  if (request.indexOf("/led") != -1 ) { // si requette = led
    if (digitalRead(2)) digitalWrite(2, LOW); // éteint la led
    else digitalWrite(2, HIGH); // allume la led
    client.println( page_0() );
  }
  else if (request.indexOf("/h") != -1 ) { // si requette = h
    sychro_heure_web(); //mise a l'heure
    Serial.println("mise a l'heure");
    client.println( page_0() );
  } 
  else if (request.indexOf("/recap") != -1 ) { // si requette = h
    client.println( page_recap() ); //
  }   
  else client.println( page_0() ); // charge la page    
  
}

/////////// gestion des pages html //////////////

/*##################################################  page_0 ###################################################*/
String page_0(){ // page HTML

  entete("ESP32 pzem wifi<br>Montaulab - fifi82 2025<br>" + date(), 5 ); // cré une nouvelle page qui se rafraichi toute les 5s 
  titre( "affichage des donnees", "h2" , "#33aa55", "#FFFFFF");

  // bouton de la led en fonction de son état
  if(digitalRead(2)) bouton( "éteindre la led" ,"led","aaaaaa","24");
  else               bouton( "alumer la led"   ,"led","aaaaaa","24");
  br();   
  
  titre ("maison : " + String(w_maison) +" W | " + String(wh_maison/1000.0) +" KW/h","h2" , "#2222AA","#FFFFFF");
  titre ("Solaire : "+ String(w_solaire) +" W | " + String(wh_solaire/1000.0) +" KW/h","h2" ,"#ffeb3b","#0");
  titre ("Solaire => EDF : "+ String(w_solaire_EDF) +" W | " + String(wh_solaire_EDF/1000.0) +" KW/h","h2" ,"#80cbc4","#0");
  titre ("EDF => Maison : " + String(w_EDF_maison) +" W | " + String(wh_EDF_maison/1000.0) +" KW/h","h2" ,"#f48fb1","#0");
  titre ("tension : " + String(tension) + " V" ,"h2","#ffffff", "#0000ff");
  
  bouton( "recap sur un mois" ,"recap","aaaaaa","24"); br();
  
  br(); // retour à la ligne
  
  fin_page();  // fin de page HTML
  return p;  // retourne la page qui vient d'être créée
}

/*################################################## page récap ###################################################*/
 String page_recap(){ // page html
  entete("ESP32 pzem wifi<br>Montaulab - fifi82 2025<br>" + date() , 0 ); // cré une nouvelle page qqui ne se rafraichit pas
  titre( "recap sur un mois", "h2" , "#33aa55", "#FFFFFF");

  bool c;
  String t;

  for (int i=1; i<32; i++){
    int a = mem_dwh[i][0] / 256;    // année
    int m = mem_dwh[i][0] - a*256;// mois
    if (a<25 or m<1 or m>12) t = " pas de donnee pour cette journee";
    else {
      t = String(i) + " " + String( t_mois[m] ) + " " + String( 2000 + a ); // date
      t +=  ", Maison: " + String( mem_dwh[i][1] / 100.0 ) + "kwh "; 
      t += "| solaire: " + String( mem_dwh[i][2] / 100.0 ) + "kwh " ;
      t += "| solaire => EDF: " +  String( mem_dwh[i][3] / 100.00 ) + "kwh " ; // 
      t += "| EDF  => maison: " +  String( mem_dwh[i][4] / 100.00) + "kwh " ; // 
    }
    
    if (i== m_jour)p += " <div style=\"background-color:#ffcccc;\">" + t + "</div>"; // dernier enregistrement
    else if (c)    p += " <div style=\"background-color:#dddddd;\">" + t + "</div>"; // gris
    else           p += " <div style=\"background-color:#ffffff;\">" + t + "</div>"; // blanc

    c = !c; // permet de changer de couleur pour une meilleure visibilité

  }

  br();br();
  bouton( "retour" ,"retour","aaaaaa","24");
  fin_page();  // fin de page HTML
  return p;  // retourne la page qui vient d'être créée
 }


// fonction pour gérer le HTML plus facilement
/*##################################################  entête  ###################################################*/
void entete(String texte, int r){// header texte=titre de la fenetre, r = au temps en seconde avant de rafraichir la fenetre
   // entête des pages html 
  p = "<!doctype html>\n";
  p += "<head>\n";
  p += "<meta charset=\"utf-8\">\n";
  p += "<title>Solaire - fifi82 2025</title>\n";
  if(r) p += "<meta http-equiv=\"refresh\" content=\"" + String(r) + ";/\">\n"; // permet de recharger la page toute les r secondes
  p += "<link rel=\"stylesheet\" href=\"style.css\">\n";
  p += "<script src=\"script.js\"></script>\n";
  p += "</head>\n";
  p += "<body><center>";
  p += "<h1><p style=\"background-color: #000000; color: white;\">" + texte + "<br>";
  p += "</h1>";
}


/*##################################################  bouton  ###################################################*/
void bouton(String texte, String requette, String couleur, String fonte){// \" affiche un " en HTML
  p +=  "<input type=\"button\" onclick=\"window.location.href = '/" + requette + "';\" value=\"\n  " + texte;
  p +=  "  \n \"style=\"background-color: #" + couleur + "; color: #F000000; font-size:"+fonte+"px\" /><br>"; 
  
}

/*##################################################  fin page  ###################################################*/
void fin_page(){
  p+= "<p style=\"background-color: #ffffff;color: white;\">. . . . . . . . . . . . . . . . . . . . . .</p></h2></center></body> </html>"; 
}

/*##################################################   texte ###################################################*/
void texte(String texte, String h){
  p +="<" + h + ">" + texte + "</" + h + ">" +"<br>";
}

/*##################################################   titre ###################################################*/
void titre2(String texte, String cf, String ct){ // h=taille du texte, cf=couleur du fond, ct=couleur du texte
  p += "<p style=\"background-color:"+ cf +"; color: "+ ct +";\">"+texte+"</p>";
}

/*##################################################   titre ###################################################*/
void titre(String texte, String h, String cf, String ct){ // h=taille du texte, cf=couleur du fond, ct=couleur du texte
  p += "<" + h + "><p style=\"background-color:"+ cf +"; color: "+ ct +";\">"+texte+"</p></" + h + ">";
}

/*##################################################   br ###################################################*/
void br(){ // retour à la ligne
  p += "<br>";
}

/*##################################################   date  ###################################################*/
String date(){ // retourne la date en format string
  String t ="";
  if (jour<10) t += "0"; t += String(jour); 
  t +=  " " + t_mois[mois-1] + " " + String(annee);
  t +=  "  -  ";
  if (heure<10)   t += "0"; t += String(heure);
  t +=  ":";
  if (minute<10)  t += "0"; t += String(minute);      
  t +=  ":";
  if (seconde<10) t += "0"; t += String(seconde);
  return t;
}


void initTime(){
  struct tm timeinfo;
  configTime(3600, 0, "pool.ntp.org");    // First connect to NTP server, with 0 TZ offset
  if(!getLocalTime(&timeinfo)){
    return;
  }
  String timezone = "CET-1CEST,M3.5.0,M10.5.0/3";
  setenv("TZ",timezone.c_str(),0);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
  tzset();
}

/*##################################################  sychro_heure_web ###################################################*/
 void sychro_heure_web(){
  timerStop(timer); // stope l'interupion timer pour éviter des bugs avec la mise à l'heure
  
  timeClient.update(); // se connecte au server web de l'horloge web
  initTime();
  time_t epochTime = timeClient.getEpochTime(); // lit l'heure
  struct tm *ptm = gmtime ((time_t *)&epochTime); // formate l'heure

  jour  = ptm->tm_mday; // récupère le jour
  mois  = ptm->tm_mon+1;// récupère le mois
  annee = ptm->tm_year+1900;// récupère l'année
  // convertit les heures, les minutes et les seconde en secondes et les stock dans horloge
  h2 = horloge = (timeClient.getHours()*3600 + timeClient.getMinutes() * 60 + timeClient.getSeconds()); 
  timerStart(timer); // redemarre l'interuption timer
 }
