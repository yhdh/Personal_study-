# Rapport TP Slurm 2 - Configuration d'un cluster Slurm

**Auteur :** Yahdhih Abdelwedoud  
**Date :** 6 février 2026

---

# 0. Préparation et validation de l'environnement

Dans cette introduction, nous allons démarrer une machine virtuelle pour préparer la configuration d'un cluster Slurm. Nous sauvegarderons ensuite l'image de cette VM puis l'instancierons dans 4 machines virtuelles pour disposer d'un cluster équivalent au cluster du TP slurm1.

**Architecture du cluster :**
- **vm0** : leader et worker slurm
- **vm[1-3]** : worker slurm seulement

---

## Q1 : Démarrer 1 machine virtuelle avec PCOCC

**Commande exécutée :**
```bash
pcocc alloc -c2 mycentos74-tp-slurm2:1
```

**Résultat obtenu :**
```
-bash-4.2$ pcocc alloc -c2 mycentos74-tp-slurm2:1
salloc: Granted job allocation 38062
Configuring hosts... (done)
```

**Analyse :** La machine virtuelle a été démarrée avec succès. Le job d'allocation 38062 a été accordé.

---

## Q2 : Se connecter à la machine virtuelle vm0

**Commande exécutée :**
```bash
pcocc ssh vm0
```

**Résultat :**
```
(pcocc/38062) bash-4.2$ pcocc ssh vm0
[yahdhih.abdelwedoud@vm0 ~]$
```

**Analyse :** Connexion SSH réussie à la machine virtuelle vm0.

---

# 1. Mise en place et validation de munge

Dans un premier temps nous allons mettre en place munge afin de permettre la sécurisation des communications et des « job credentials » de Slurm.

---

## Q1 : Installation des packages munge

**Commande exécutée :**
```bash
sudo yum -y install munge
```

**Résultat obtenu :**
```
Résolution des dépendances
--> Lancement de la transaction de test
---> Le paquet munge.x86_64 0:0.5.11-3.el7 sera installé
--> Traitement de la dépendance : munge-libs = 0.5.11-3.el7 pour le paquet : munge-0.5.11-3.el7.x86_64
--> Traitement de la dépendance : libmunge.so.2()(64bit) pour le paquet : munge-0.5.11-3.el7.x86_64
--> Lancement de la transaction de test
---> Le paquet munge-libs.x86_64 0:0.5.11-3.el7 sera installé
--> Résolution des dépendances terminée

Dépendances résolues

==========================================================================================
 Package               Architecture      Version                    Dépôt           Taille
==========================================================================================
Installation :
 munge                 x86_64            0.5.11-3.el7               epel             95 k
Installation pour dépendances :
 munge-libs            x86_64            0.5.11-3.el7               epel             37 k

Résumé de la transaction
==========================================================================================
Installation   1 Paquet (+1 Paquet en dépendance)

Taille totale des téléchargements : 132 k
Taille d'installation : 318 k

...

Installé :
  munge.x86_64 0:0.5.11-3.el7                                                             

Dépendances installées :
  munge-libs.x86_64 0:0.5.11-3.el7                                                        

Terminé !
```

**Analyse :** Le package munge (version 0.5.11-3.el7) a été installé avec succès, ainsi que sa dépendance munge-libs.

---

## Q2 : Création du secret partagé de munge

**Commande exécutée :**
```bash
sudo /usr/sbin/create-munge-key
```

**Résultat obtenu :**
```
Generating a pseudo-random key using /dev/urandom completed.
```

**Analyse :** La clé secrète partagée de munge a été générée avec succès à partir de `/dev/urandom`. Cette clé sera utilisée pour chiffrer et authentifier les communications entre les nœuds du cluster.

---

## Q3 : Activation et démarrage du service munge

**Commandes exécutées :**
```bash
sudo systemctl enable munge
sudo systemctl start munge
```

**Résultat obtenu :**
```
Created symlink from /etc/systemd/system/multi-user.target.wants/munge.service to /usr/lib/systemd/system/munge.service.
```

**Analyse :** 
- `systemctl enable` : Le service munge est maintenant activé au démarrage (lien symbolique créé)
- `systemctl start` : Le service munge a été démarré (aucune erreur affichée)

---

## Q4 : Test du bon fonctionnement de munge

**Commandes exécutées :**
```bash
echo "Hello" | munge
echo "Hello" | munge | unmunge
```

**Résultat de `echo "Hello" | munge` :**
```
MUNGE:AwQDAADFouUwWDhnxHfoDHobA+AAesg9OXgyXdanltlAKRehCNGkTRRfzAmGOZCXK8ia0JIfqpLmT3aHjfITwKi1JUARZ9QdMFMXNlH5tTxZoHYXmfG/Djw=:
```

**Résultat de `echo "Hello" | munge | unmunge` :**
```
STATUS:           Success (0)
ENCODE_HOST:      vm0 (10.252.0.1)
ENCODE_TIME:      2026-02-06 10:38:22 +0000 (1770374302)
DECODE_TIME:      2026-02-06 10:38:22 +0000 (1770374302)
TTL:              300
CIPHER:           aes128 (4)
MAC:              sha1 (3)
ZIP:              none (0)
UID:              yahdhih.abdelwedoud (1002)
GID:              yahdhih.abdelwedoud (1002)
LENGTH:           6

Hello
```

### Commentaires sur les résultats :

| Champ | Valeur | Signification |
|-------|--------|---------------|
| STATUS | Success (0) | Le décodage a réussi |
| ENCODE_HOST | vm0 (10.252.0.1) | Machine qui a créé le credential |
| ENCODE_TIME | 2026-02-06 10:38:22 | Horodatage de création |
| DECODE_TIME | 2026-02-06 10:38:22 | Horodatage de décodage |
| TTL | 300 | Durée de vie du credential (5 minutes) |
| CIPHER | aes128 (4) | Algorithme de chiffrement utilisé |
| MAC | sha1 (3) | Algorithme de signature utilisé |
| UID/GID | yahdhih.abdelwedoud (1002) | Identité de l'utilisateur encodée |
| LENGTH | 6 | Taille du payload ("Hello\n") |

**Conclusion :** Munge fonctionne correctement. Il chiffre le message avec AES-128, le signe avec SHA1, et encode l'identité de l'utilisateur (UID/GID) dans le credential.

---

## Q5 : Validation de la détection du « replay »

**Commandes exécutées :**
```bash
echo "Hello" | munge > /tmp/munge_cred
cat /tmp/munge_cred | unmunge    # Première utilisation
cat /tmp/munge_cred | unmunge    # Deuxième utilisation (replay)
```

**Première utilisation :**
```
STATUS:           Success (0)
ENCODE_HOST:      vm0 (10.252.0.1)
ENCODE_TIME:      2026-02-06 10:40:06 +0000 (1770374406)
DECODE_TIME:      2026-02-06 10:42:02 +0000 (1770374522)
TTL:              300
CIPHER:           aes128 (4)
MAC:              sha1 (3)
ZIP:              none (0)
UID:              yahdhih.abdelwedoud (1002)
GID:              yahdhih.abdelwedoud (1002)
LENGTH:           6

Hello
```

**Deuxième utilisation (replay) :**
```
STATUS:           Replayed credential (17)
ENCODE_HOST:      vm0 (10.252.0.1)
ENCODE_TIME:      2026-02-06 10:40:06 +0000 (1770374406)
DECODE_TIME:      2026-02-06 10:42:41 +0000 (1770374561)
TTL:              300
CIPHER:           aes128 (4)
MAC:              sha1 (3)
ZIP:              none (0)
UID:              yahdhih.abdelwedoud (1002)
GID:              yahdhih.abdelwedoud (1002)
LENGTH:           6

Hello
```

**Code de retour :** `echo $?` → **17**

### Élément surlignant la détection du replay :

**`STATUS: Replayed credential (17)`**

### Explication du replay :

Un **replay** (ou attaque par rejeu) consiste à réutiliser un credential munge déjà utilisé. C'est une attaque de sécurité où un attaquant capture un message authentique et le retransmet plus tard.

**Pourquoi munge protège contre le replay :**
1. Munge maintient un **cache des credentials déjà utilisés**
2. Chaque credential possède un identifiant unique (basé sur ENCODE_TIME et autres métadonnées)
3. Lorsqu'un credential est décodé une première fois, il est marqué comme "utilisé"
4. Toute tentative de réutilisation retourne l'erreur `Replayed credential (17)`

**Importance pour Slurm :** Cette protection empêche un attaquant de capturer un job credential et de le rejouer pour soumettre des jobs frauduleux au nom d'un autre utilisateur.

---

## Q6 : Validation du fonctionnement de l'expiration

**Commandes exécutées :**
```bash
echo "Hello" | munge -t 30 > /tmp/munge_cred
sleep 31; cat /tmp/munge_cred | unmunge
```

**Résultat après expiration (31 secondes) :**
```
STATUS:           Expired credential (15)
ENCODE_HOST:      vm0 (10.252.0.1)
ENCODE_TIME:      2026-02-06 10:45:41 +0000 (1770374741)
DECODE_TIME:      2026-02-06 10:46:56 +0000 (1770374816)
TTL:              30
CIPHER:           aes128 (4)
MAC:              sha1 (3)
ZIP:              none (0)
UID:              yahdhih.abdelwedoud (1002)
GID:              yahdhih.abdelwedoud (1002)
LENGTH:           6

Hello
```

**Code de retour :** `echo $?` → **15**

### Élément surlignant l'expiration :

**`STATUS: Expired credential (15)`** et **`TTL: 30`**

### Explication de l'expiration :

L'option `-t 30` définit un **TTL (Time To Live)** de 30 secondes pour le credential. Après ce délai, le credential expire et ne peut plus être utilisé.

**Pourquoi imposer l'expiration d'un message munge :**

1. **Limiter la fenêtre d'attaque** : Si un credential est intercepté, l'attaquant n'a qu'un temps limité pour l'utiliser
2. **Fraîcheur des credentials** : Garantit que les credentials utilisés sont récents
3. **Synchronisation temporelle** : Nécessite que les nœuds du cluster aient des horloges synchronisées
4. **Éviter l'accumulation** : Empêche l'accumulation de credentials valides dans le cache

**Pour Slurm :** Le TTL par défaut de 300 secondes (5 minutes) est suffisant pour le fonctionnement normal tout en limitant les risques de sécurité.

---

## Q7 : Validation de la restriction des uid/gid

**Commandes exécutées (en tant qu'utilisateur normal, pas root) :**
```bash
echo "Hello" | munge -u 0 -g 0 | unmunge
echo "Hello" | munge -u 0 -g 0 | sudo unmunge
```

**Résultat sans sudo :**
```
unmunge: Error: Unauthorized credential for client UID=1002 GID=1002
```

**Résultat avec sudo :**
```
STATUS:           Success (0)
ENCODE_HOST:      vm0 (10.252.0.1)
ENCODE_TIME:      2026-02-06 10:49:11 +0000 (1770374951)
DECODE_TIME:      2026-02-06 10:49:11 +0000 (1770374951)
TTL:              300
CIPHER:           aes128 (4)
MAC:              sha1 (3)
ZIP:              none (0)
UID:              yahdhih.abdelwedoud (1002)
GID:              yahdhih.abdelwedoud (1002)
UID_RESTRICTION:  root (0)
GID_RESTRICTION:  root (0)
LENGTH:           6

Hello
```

### Différence entre les deux résultats :

| Aspect | Sans sudo | Avec sudo |
|--------|-----------|-----------|
| Exécuteur de unmunge | UID=1002 (utilisateur normal) | UID=0 (root) |
| Restriction demandée | UID=0, GID=0 (root) | UID=0, GID=0 (root) |
| Résultat | **Unauthorized** | **Success** |

### Explication :

Les options `-u 0 -g 0` créent un credential qui **ne peut être décodé que par root** (UID=0, GID=0).

- **Sans sudo** : L'utilisateur yahdhih.abdelwedoud (UID=1002) tente de décoder un credential restreint à root → **Refusé**
- **Avec sudo** : L'utilisateur root (UID=0) décode le credential → **Autorisé**

### Pourquoi restreindre la lecture des messages munge :

1. **Contrôle d'accès fin** : Permet de créer des credentials destinés uniquement à certains utilisateurs ou groupes
2. **Séparation des privilèges** : Les services système (comme slurmctld) peuvent échanger des credentials que seul root peut lire
3. **Sécurité des communications inter-démons** : Les démons Slurm (slurmctld, slurmd) s'exécutent souvent en tant que root ou utilisateur dédié
4. **Protection contre l'espionnage** : Empêche les utilisateurs non autorisés de lire les credentials même s'ils les interceptent

**Pour Slurm :** Cette fonctionnalité garantit que les communications entre les démons Slurm ne peuvent être lues que par les processus autorisés.
