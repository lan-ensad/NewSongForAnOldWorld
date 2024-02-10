# Old Word Controller

1. [Old Word Controller](#old-word-controller)
   1. [Allumage du réseau](#allumage-du-réseau)
   2. [Connexion à l'interface](#connexion-à-linterface)
   3. [Démarrage des appareils](#démarrage-des-appareils)
   4. [Pannes](#pannes)

## Allumage du réseau

1. Appuyer 4 seconde sur le bouton d'allumage du router jusq'uà voir les icones lumineuses. Brancher le cordon usb si nécessaire pour recharger la batterie interne

![routeur](_img/routeur.jpg)

2. Brancher ensuite le cordon de la RaspberryPi pour qu'elle puisse se connecter sur le réseau.

![Raspb](_img/routerEtRaspb.jpg)

## Connexion à l'interface

1. Se connecter au réseau Wifi :

ssid     →   `SSID` \
pass    →   `PASS`

2. Interface :

Ouvrir un navigateur et inscrire l'url dans la barre de navigation : `192.168.0.196:1880/ui`

En cliquant sur le burger menu en haut à gauche de la fenêtre, on pourra sélectionner sur quelle machine prendre le contrôle.

![interface](_img/interface_2.png)

## Démarrage des appareils

Sur chaque appareil, il faut alimenter le micro-contrôler interne afin qu'il puisse se connecter au réseau.

1. Brancher le cordon USB sur la power bank en vérifiant que les leds soient bien allumées.

![powerBanck](_img/batterie_state.jpg)

2. Si la batterie ne s'allume pas lorsqu'on branche le cordon, appuyer sur le bouton de l'autre côté.

![powerBankBtn](_img/batterie_btn.jpg)

## Pannes

- Si un appareil ne répond plus :
  - Vérifier que la batterie ne s'est pas mise en veille ou qu'elle n'est pas déchargée (les 4 lumières sur le côté).
  - Débrancher et rebrancher.

- Si le réseau `OldWordController` ne s'affiche pas sur un appareil :
  - Vérifier que la batterie ne s'est pas mise en veille ou qu'elle n'est pas déchargée (les 4 lumières sur le côté).
  - Redémarrer le routeur [↑Séquence d'allumage](#allumage-du-réseau).

- Impossible de se connecter au réseau `OldWordController` :
  - Vérifier le mot de passe : `uSeU699pW7`
  - Vérifier que le protocole de sécurité sélectionner soit : `WPA/WPA2 Personnal` (pas de `WPA3` ou autre...)?.

- L'interface n'est pas accessible dans le navigateur :
  - Vérifier l'adresse  : `http://192.168.1.18:1880/ui` (pas de `https://...`).
  - Débrancher puis rebrancher le cordon de la RaspberryPi.