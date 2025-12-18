
# Serrure Connectée – Client-Serveur en C

## Description

Ce projet consiste à développer une application de gestion d'accès pour une serrure connectée en C, utilisant des sockets TCP.  
Le système comprend un **serveur** représentant la serrure et deux types de **clients** :  

- **Propriétaire** : peut créer/modifier le code d’accès, définir la durée de validité, et recevoir des alertes.  
- **Locataire** : peut saisir un code pour accéder à la serrure, avec un maximum de 3 tentatives.  

Le serveur gère plusieurs clients simultanément grâce à la fonction `poll()` et conserve un historique des accès dans un fichier log.

---

## Structure du Projet

```plaintext
client-server-RC52/
├── server/
│   ├── server.c             # Code source du serveur principal
│   ├── linked_list.h        # Gestion de la liste chaînée (utilisateurs/codes)
│   ├── access_code.h        # Logique de vérification des codes
│   ├── history.h            # Prototypes pour la gestion de l'historique
│   ├── Makefile             # Compilation du serveur
│   └── access_history.log   # Généré à l'exécution (journal des accès)
├── client_locataire/
│   ├── locataire.c          # Code source pour l'interface locataire
│   └── Makefile             # Compilation du client locataire
└── client_proprietaire/
    ├── proprietaire.c       # Code source pour l'interface propriétaire
    └── Makefile             # Compilation du client propriétaire 

---

## Fonctionnalités

### Jalon 1 – Client-Serveur TCP
- Connexion TCP entre client et serveur.
- Le client peut envoyer un message au serveur.
- Le serveur renvoie le message au même client.
- `/quit` permet de fermer proprement la connexion.

### Jalon 2 – Multi-clients
- Gestion de plusieurs clients simultanément via `poll()`.
- Chaque client peut envoyer/recevoir des messages indépendamment.
- Les clients sont stockés dans une **liste chaînée**.

### Jalon 3 – Gestion d’accès
- Identification des clients : propriétaire ou locataire.
- Le propriétaire reçoit un code par défaut et peut le modifier.
- Le locataire saisit le code d’accès et dispose de 3 tentatives maximum.
- Alarme déclenchée après 3 échecs : code renouvelé et notification au propriétaire.
- Historique des accès enregistré dans `access_history.log`.
- Rotation automatique du code après expiration.

### Bonus (optionnel)
- Mise en forme des messages côté client (couleurs).
- Relais de messages entre locataire et propriétaire.

---

## Compilation

Utilisez le **Makefile** pour compiler tous les programmes :  

make

Cela génèrera 3 exécutables :  
- `server`  
- `locataire`  
- `proprietaire`  

---

## Exécution

### Lancer le serveur
./server <port>

Exemple :
./server 7777

### Lancer le client propriétaire
./proprietaire <server_ip> <server_port> <pseudo>

Exemple :
./proprietaire 127.0.0.1 7777 gabriel

### Lancer le client locataire
./locataire <server_ip> <server_port>

Exemple :
./locataire 127.0.0.1 7777

---

## Utilisation

1. **Propriétaire** :  
   - S’identifie avec un pseudo.  
   - Reçoit un code d’accès par défaut.  
   - Peut modifier le code avec la commande :  
     SETCODE:<6chiffres>:<durée_en_secondes>
   - Peut consulter le code actuel avec :
     GETCODE

2. **Locataire** :  
   - S’identifie automatiquement comme locataire.  
   - Saisit le code envoyé par le propriétaire.  
   - A 3 tentatives pour entrer le code correct.  
   - Après 3 échecs, une alarme se déclenche et le code est renouvelé.  

3. **Messages généraux** :  
   - Les clients peuvent taper `/quit` pour quitter proprement.  
   - Les messages inter-client peuvent être relayés (si implémentation bonus).

---

## Historique des accès

Toutes les actions (tentatives de locataire, changements de code, alarmes) sont enregistrées dans le fichier `access_history.log`.  
Format exemple :
[DATE_HEURE] CLIENT:<IP>:<PORT> ACTION:ACCESS_GRANTED CODE:123456

---

## Remarques

- Le projet utilise des sockets TCP IPv4.  
- `poll()` est utilisé pour gérer simultanément plusieurs clients et l’entrée standard.  
- Le code est structuré pour être clair et extensible : fichiers séparés pour la liste chaînée, la gestion des codes et l’historique.

---

## Auteurs

- Gabriel Chalmet  
