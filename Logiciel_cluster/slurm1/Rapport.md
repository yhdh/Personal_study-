# Préparation et validation de l'environnement

## Q1 : Démarrage des machines virtuelles

**Objectif :** Démarrer 4 machines virtuelles avec PCOCC en utilisant le template du TP Slurm 1.

**Commande exécutée :**
```bash
pcocc alloc -c2 mycentos74-tp-slurm1:4
```

**Résultat obtenu :**
```
-bash-4.2$ pcocc alloc -c2 mycentos74-tp-slurm1:4
salloc: Granted job allocation 37993
Configuring hosts... (done)
```

**Analyse :** Le message confirme le succès du lancement des 4 machines virtuelles avec l'allocation du job 37993.

---

## Q2 : Suivi du démarrage de la première VM

**Objectif :** Observer l'avancement du démarrage de la première machine virtuelle.

**Commande exécutée :**
```bash
pcocc console
```

**Résultat obtenu :**
```
(pcocc/37993) bash-4.2$ pcocc console
Warning: Permanently added 'hpc06,192.168.1.26' (ECDSA) to the list of known hosts.

Detaching ...
Killed by signal 15.
```

**Analyse :** La commande a été lancée tardivement, ce qui n'a pas permis de capturer l'état de démarrage des machines virtuelles en temps réel.

---

## Q3 : Sortie de la console avec Ctrl+C

**Objectif :** Sortir de la commande `pcocc console` en utilisant Ctrl+C trois fois de suite.

**Résultat obtenu :**
```
(pcocc/37993) bash-4.2$ pcocc console
Warning: Permanently added 'hpc06,192.168.1.26' (ECDSA) to the list of known hosts.

Detaching ...
Killed by signal 15.
(pcocc/37993) bash-4.2$ pcocc ssh vm0
[yahdhih.abdelwedoud@vm0 ~]$ sinfo -l
```

**Analyse :** Après la sortie de la console, la connexion SSH à vm0 a été établie avec succès.

---

## Q4 : Vérification de l'état du cluster

**Objectif :** Se connecter à la machine virtuelle vm0 et vérifier l'état du cluster.

**Commandes exécutées :**
```bash
pcocc ssh vm0
sinfo -l
```

**Résultat obtenu :**
```
Mon Feb  2 15:46:05 2026
PARTITION AVAIL  TIMELIMIT   JOB_SIZE ROOT    SHARE     GROUPS  NODES       STATE NODELIST
vm*          up   infinite 1-infinite   no       NO        all      4        idle vm[0-3]
```

**Analyse :** Le cluster est opérationnel avec 4 nœuds disponibles.

---

## Q5 : Description du cluster

D'après la sortie de la commande `sinfo -l`, voici les caractéristiques du cluster :

| Paramètre | Valeur | Description |
|-----------|--------|-------------|
| PARTITION | vm* | Partition par défaut (marquée *) |
| AVAIL | up | Partition disponible |
| TIMELIMIT | infinite | Pas de limite de temps d'exécution |
| JOB_SIZE | 1-infinite | Jobs de 1 à N tâches autorisés |
| ROOT | no | Exécution en root non autorisée |
| SHARE | NO | Allocation exclusive des ressources |
| GROUPS | all | Tous les groupes d'utilisateurs autorisés |
| NODES | 4 | Nombre de nœuds disponibles |
| STATE | idle | Tous les nœuds sont libres |
| NODELIST | vm[0-3] | Liste des nœuds : vm0, vm1, vm2, vm3 |

---

## Q6 : Première exécution interactive

**Objectif :** Vérifier le bon fonctionnement de Slurm avec une exécution interactive.

**Première tentative :**
```bash
srun -n 4 -N 4 hostname
```

**Erreur obtenue :**
```
srun: error: Unable to allocate resources: Invalid account or account/partition combination specified
```

**Résolution :** Ajout de l'utilisateur dans la base de données de comptabilité Slurm :
```bash
sudo sacctmgr -i add user name=yahdhih.abdelwedoud cluster=ensiie account=guests fairshare=10 qos=normal,debug
```

**Résultat :**
```
 Adding User(s)
  yahdhih.abdelwedoud
 Settings =
  Default Account = (null)
 Associations =
  U = yahdhih.a A = guests     C = ensiie
 Non Default Settings
  Fairshare     = 10
  QOS           = debug,normal
```

**Analyse :** L'utilisateur a été ajouté avec succès au compte `guests` avec les QoS `normal` et `debug`.

---

# 1. Premier script batch  

## Q1 : Créer un premier script affichant le nom du nœud exécutant le script

Le script `script-1.sh` contient la commande `hostname` et retourne un code 0.

```bash
#!/bin/bash
hostname
exit 0
```

Exécution locale (sans Slurm) :
```
[yahdhih.abdelwedoud@vm0 ~]$ chmod +x script-1.sh
[yahdhih.abdelwedoud@vm0 ~]$ ./script-1.sh
vm0
```

## Q2 : Exécuter avec sbatch et analyser avec sacct

```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch ./script-1.sh
Submitted batch job 57
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 57
    JobID    JobName  Partition    Account  AllocCPUS      State ExitCode
------------ ---------- ---------- ---------- ---------- ---------- --------
57           script-1.+         vm     guests          1  COMPLETED      0:0
57.batch          batch                guests          1  COMPLETED      0:0
```

Le job 57 s'est exécuté avec succès (State = COMPLETED, ExitCode = 0:0).

## Q3 : Modifier le script pour obtenir un état FAILED

Pour obtenir un état FAILED, il suffit de retourner un code différent de 0 (par exemple `exit 1`).

```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch ./script-1.sh
Submitted batch job 60
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 60
    JobID    JobName  Partition    Account  AllocCPUS      State ExitCode
------------ ---------- ---------- ---------- ---------- ---------- --------
60           script-1.+         vm     guests          1     FAILED      1:0
60.batch          batch                guests          1     FAILED      1:0
```

Le job 60 a échoué avec ExitCode = 1:0.

## Q4 : Demander l'allocation de 4 tâches

Après avoir remis le script avec `exit 0`, on demande 4 tâches avec `-n 4` :

```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n 4 script-1.sh
Submitted batch job 61
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 61
    JobID    JobName  Partition    Account  AllocCPUS      State ExitCode
------------ ---------- ---------- ---------- ---------- ---------- --------
61           script-1.+         vm     guests          4  COMPLETED      0:0
61.batch          batch                guests          2  COMPLETED      0:0
```

### Différence au niveau de l'accounting :

| Job | AllocCPUS (job) | AllocCPUS (batch) |
|-----|-----------------|-------------------|
| 57 (1 tâche) | 1 | 1 |
| 61 (4 tâches) | 4 | 2 |

Avec `-n 4`, Slurm alloue **4 CPUs** au job global, mais le script batch lui-même n'utilise que **2 CPUs** (sur un seul nœud). Les 4 CPUs sont réservés pour d'éventuelles sous-tâches (srun), mais le script `hostname` n'en utilise qu'une partie.

---

# 2. Intégration de « step » parallèles

L'exécution d'un script batch demandant l'allocation de plusieurs cœurs nécessite le lancement d'un « jobstep » afin de pouvoir les utiliser en parallèle. Le lancement d'un step se fait par l'intermédiaire d'une commande `srun` intégrée dans le script batch.

---

## Q1 : Intégrer l'exécution d'un step affichant le nom des machines

**Objectif :** Dupliquer le premier script batch et le modifier pour intégrer l'exécution d'un step avec `srun`.

**Script `sript-1-2.sh` :**
```bash
#!/bin/bash
srun hostname
exit 0
```

**Exécution avec sbatch :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n 4 sript-1-2.sh
Submitted batch job 58
```

**Contenu du fichier de sortie `slurm-58.out` :**
```
vm0
vm0
vm1
vm1
```

**Données d'accounting :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 58 -o jobid,jobname,nodelist,alloccpus,state
       JobID    JobName        NodeList  AllocCPUS      State 
------------ ---------- --------------- ---------- ---------- 
58           sript-1-2+         vm[0-1]          4  COMPLETED 
58.batch          batch             vm0          2  COMPLETED 
58.0           hostname         vm[0-1]          4  COMPLETED 
```

### Commentaires sur nodelist et alloccpus :

| Step | NodeList | AllocCPUS | Explication |
|------|----------|-----------|-------------|
| 58 (job global) | vm[0-1] | 4 | Le job utilise 2 nœuds (vm0 et vm1) avec 4 CPUs au total |
| 58.batch | vm0 | 2 | Le script batch lui-même s'exécute sur vm0 avec 2 CPUs |
| 58.0 (hostname) | vm[0-1] | 4 | Le step `srun hostname` utilise les 4 CPUs sur les 2 nœuds |

**Analyse :** Grâce à `srun`, la commande `hostname` est exécutée en parallèle sur **tous les cœurs alloués** (4 tâches). Comme chaque nœud dispose de 2 CPUs, Slurm répartit les 4 tâches sur 2 nœuds (vm0 et vm1), d'où les 4 lignes de sortie (2× vm0, 2× vm1).

---

## Q2 : Ajouter un second step avec sleep 120

**Objectif :** Dupliquer le script précédent et ajouter un step exécutant une attente de 120 secondes.

**Script `script-2.2.sh` :**
```bash
#!/bin/bash
srun hostname
srun sleep 120
exit 0
```

**Exécution avec sbatch :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n 4 script-2.2.sh
Submitted batch job 61
```

**Suivi de l'exécution avec squeue :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -s -j 61
         STEPID     NAME PARTITION     USER      TIME NODELIST
           61.1    sleep        vm yahdhih.      0:19 vm[0-1]
```

**État pendant l'exécution (sacct) :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 61
       JobID    JobName  Partition    Account  AllocCPUS      State ExitCode 
------------ ---------- ---------- ---------- ---------- ---------- -------- 
61           script-2.+         vm     guests          4    RUNNING      0:0 
61.0           hostname                guests          4  COMPLETED      0:0 
61.1              sleep                guests          4    RUNNING      0:0 
```

**Analyse du déroulement :**
- Le step 61.0 (`hostname`) s'est terminé rapidement (COMPLETED)
- Le step 61.1 (`sleep`) est en cours d'exécution (RUNNING)
- L'option `-s` de `squeue` affiche les steps actifs
- L'option `-i 30` permet de rafraîchir l'affichage toutes les 30 secondes

**Données d'accounting après terminaison :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 61 -o jobid,jobname,nodelist,alloccpus,state,end,elapsed
       JobID    JobName        NodeList  AllocCPUS      State                 End    Elapsed 
------------ ---------- --------------- ---------- ---------- ------------------- ---------- 
61           script-2.+         vm[0-1]          4  COMPLETED 2026-02-02T22:49:39   00:02:01 
61.batch          batch             vm0          2  COMPLETED 2026-02-02T22:49:39   00:02:01 
61.0           hostname         vm[0-1]          4  COMPLETED 2026-02-02T22:47:39   00:00:01 
61.1              sleep         vm[0-1]          4  COMPLETED 2026-02-02T22:49:39   00:02:00 
```

### Analyse des données d'accounting :

| Step | Elapsed | Explication |
|------|---------|-------------|
| 61 (job global) | 00:02:01 | Durée totale du job |
| 61.batch | 00:02:01 | Le batch attend la fin de tous les steps |
| 61.0 (hostname) | 00:00:01 | Exécution instantanée |
| 61.1 (sleep) | 00:02:00 | 120 secondes comme prévu |

Le temps total du job (2min01s) correspond au temps du step le plus long (sleep 120s) plus quelques secondes de latence.

---

## Q3 : Boucle de 3 steps avec annulation (scancel)

**Objectif :** Remplacer les deux steps par une boucle de 3 steps exécutant chacun un sleep de 120 secondes, puis utiliser `scancel` pour arrêter prématurément.

**Script `script-2.3.sh` :**
```bash
#!/bin/bash
for i in 1 2 3; do
    srun sleep 120
done
exit 0
```

**Exécution avec sbatch :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n 4 script-2.3.sh
Submitted batch job 62
```

**Suivi avec squeue :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -s -j 62
         STEPID     NAME PARTITION     USER      TIME NODELIST
           62.0    sleep        vm yahdhih.      0:57 vm[0-1]
```

**Annulation du premier step :**
```
[yahdhih.abdelwedoud@vm0 ~]$ scancel 62.0
```

**État après annulation du step 62.0 :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 62 -o jobid,user,jobname%15,state%20,elapsed,exit
       JobID      User         JobName                State    Elapsed ExitCode 
------------ --------- --------------- -------------------- ---------- -------- 
62           yahdhih.+   script-2.3.sh              RUNNING   00:02:33      0:0 
62.0                             sleep    CANCELLED by 1003   00:01:40      0:9 
62.1                             sleep              RUNNING   00:00:53      0:0 
```

**État intermédiaire (step 62.1 terminé, 62.2 en cours) :**
```
       JobID      User         JobName                State    Elapsed ExitCode 
------------ --------- --------------- -------------------- ---------- -------- 
62           yahdhih.+   script-2.3.sh              RUNNING   00:04:12      0:0 
62.0                             sleep    CANCELLED by 1003   00:01:40      0:9 
62.1                             sleep            COMPLETED   00:02:00      0:0 
62.2                             sleep              RUNNING   00:00:32      0:0 
```

**Annulation du job complet :**
```
[yahdhih.abdelwedoud@vm0 ~]$ scancel 62
```

**État final après annulation complète :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 62 -o jobid,user,jobname%15,state%20,elapsed,exit
       JobID      User         JobName                State    Elapsed ExitCode 
------------ --------- --------------- -------------------- ---------- -------- 
62           yahdhih.+   script-2.3.sh    CANCELLED by 1003   00:05:02      0:0 
62.batch                         batch            CANCELLED   00:05:03     0:15 
62.0                             sleep    CANCELLED by 1003   00:01:40      0:9 
62.1                             sleep            COMPLETED   00:02:00      0:0 
62.2                             sleep            CANCELLED   00:01:22     0:15 
```

### Explication de la sortie de sacct :

| Step | State | ExitCode | Explication |
|------|-------|----------|-------------|
| 62 (job) | CANCELLED by 1003 | 0:0 | Job annulé par l'utilisateur (UID 1003) |
| 62.batch | CANCELLED | 0:15 | Script batch interrompu (signal 15 = SIGTERM) |
| 62.0 | CANCELLED by 1003 | 0:9 | Premier step annulé manuellement (signal 9 = SIGKILL) |
| 62.1 | COMPLETED | 0:0 | Deuxième step terminé normalement |
| 62.2 | CANCELLED | 0:15 | Troisième step interrompu par l'annulation du job |

**Observations :**
- `scancel 62.0` annule uniquement le step spécifié, le job continue avec les steps suivants
- `scancel 62` annule le job complet et tous les steps en cours
- L'ExitCode `0:15` indique une terminaison par signal SIGTERM (15)
- L'ExitCode `0:9` indique une terminaison par signal SIGKILL (9)
- Le code `1003` correspond à l'UID de l'utilisateur qui a lancé la commande `scancel`

---

# 3. Utilisation des QOS

L'objectif est de comprendre le comportement des politiques d'ordonnancement en utilisant différentes QOS pour favoriser le démarrage de certains jobs. Le cluster Slurm du TP est configuré de manière à autoriser 2 QOS : une par défaut (`normal`) et une optionnelle (`debug`).

---

## Q1 : Vérification du service slurmdbd et description des QOS

**Objectif :** Vérifier l'état du service `slurmdbd` et décrire les différences entre les deux QOS.

**Vérification du service slurmdbd :**
```
[yahdhih.abdelwedoud@vm0 ~]$ systemctl status slurmdbd
● slurmdbd.service - Slurm DBD accounting daemon
   Loaded: loaded (/usr/lib/systemd/system/slurmdbd.service; disabled; vendor preset: disabled)
   Active: active (running) since lun. 2026-02-02 22:34:17 UTC; 1h 0min ago
  Process: 1650 ExecStart=/usr/sbin/slurmdbd $SLURMDBD_OPTIONS (code=exited, status=0/SUCCESS)
 Main PID: 1654 (slurmdbd)
   CGroup: /system.slice/slurmdbd.service
           └─1654 /usr/sbin/slurmdbd
```

**Analyse :** Le service `slurmdbd` est actif et en cours d'exécution (active running).

**Affichage des QOS disponibles :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sacctmgr show qos
      Name   Priority  GraceTime    Preempt PreemptMode   Flags UsageThres UsageFactor  MaxWall MaxJobsPU
---------- ---------- ---------- ---------- ----------- ------- ---------- ----------- -------- ---------
    normal          0   00:00:00            cluster                  1.000000                    
     debug         10   00:00:00            cluster                  1.000000 00:05:00         1
```

### Comparaison des deux QOS :

| Propriété | normal | debug |
|-----------|--------|-------|
| Priority | 0 | 10 |
| GraceTime | 00:00:00 | 00:00:00 |
| PreemptMode | cluster | cluster |
| UsageFactor | 1.0 | 1.0 |
| MaxWall | illimité | 00:05:00 (5 minutes) |
| MaxJobsPU | illimité | 1 |

### Différences principales :

1. **Priority** : `debug` a une priorité de 10 contre 0 pour `normal`, ce qui signifie que les jobs debug seront favorisés dans la file d'attente.

2. **MaxWall** : Les jobs `debug` sont limités à 5 minutes d'exécution maximum, tandis que `normal` n'a pas de limite.

3. **MaxJobsPU** : Un utilisateur ne peut avoir qu'**un seul job actif** avec la QOS `debug`, alors qu'il n'y a pas de limite pour `normal`.

**Quelles QOS pouvez-vous utiliser ?**

```
[yahdhih.abdelwedoud@vm0 ~]$ sacctmgr show assoc format=cluster,account,user%20,qos%14,defaultqos
   Cluster    Account                 User            QOS   Def QOS
---------- ---------- -------------------- -------------- ---------
    ensiie     guests   yahdhih.abdelwedoud   debug,normal    normal
```

**Analyse :** L'utilisateur `yahdhih.abdelwedoud` peut utiliser les deux QOS : `debug` et `normal`. La QOS par défaut est `normal`.

**Vérification de la QOS utilisée par défaut :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch script-2.3.sh
Submitted batch job 63
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 63 -o jobid,user,jobname,qos
       JobID      User    JobName        QOS 
------------ --------- ---------- ---------- 
63           yahdhih.+ script-2.+     normal 
63.0                        sleep            
```

**Conclusion :** Sans spécifier de QOS, le job utilise bien la QOS par défaut `normal`.

---

## Q2 : Lancement avec allocation exclusive et QOS debug

**Objectif :** Lancer deux fois le script avec allocation exclusive de 4 tâches sur 4 nœuds, puis une troisième fois avec la QOS `debug`. Observer les priorités.

**Premiers lancements avec allocation exclusive (QOS normal) :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n4 -N4 --exclusive script-2.2.sh
Submitted batch job 64
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n4 -N4 --exclusive script-2.2.sh
Submitted batch job 65
```

**Troisième lancement avec QOS debug :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n4 -N4 --exclusive --qos debug script-2.2.sh
Submitted batch job 66
```

**Observation avec squeue :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,state,qos,timeused
JOBID               STATE               QOS                 TIME                
64                  PENDING             normal              0:00                
65                  PENDING             normal              0:00                
66                  PENDING             normal              0:00                
63                  RUNNING             normal              3:54                
```

**Observation avec sprio :**
La commande `sprio` permet de voir les facteurs de priorité des jobs en attente.

### Explication de l'ordre d'exécution :

Les jobs avec `--exclusive` demandent l'accès exclusif aux nœuds. Comme le job 63 occupe déjà les ressources, les jobs 64, 65 et 66 sont en attente (PENDING) avec la raison "Resources".

La QOS `debug` ayant une priorité plus élevée (10 vs 0), les jobs debug seront favorisés dès que les ressources se libèrent.

---

## Q3 : Lancement sans allocation exclusive et sans QOS debug

**Objectif :** Lancer deux fois le script **sans** `--exclusive` et **sans** QOS debug.

**Lancements :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n4 -N4 script-2.2.sh
Submitted batch job 68
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n4 -N4 script-2.2.sh
Submitted batch job 69
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n4 -N4 script-2.2.sh
Submitted batch job 70
```

**Observation avec squeue :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,state,qos,timeused,reason
JOBID               STATE               QOS                 TIME                REASON              
64                  PENDING             normal              0:00                Resources           
65                  PENDING             normal              0:00                Resources           
66                  PENDING             normal              0:00                Resources           
67                  PENDING             normal              0:00                Resources           
68                  PENDING             normal              0:00                Priority            
69                  PENDING             normal              0:00                Priority            
70                  PENDING             normal              0:00                Priority            
63                  RUNNING             normal              4:53                None                
```

### Constatation :

| REASON | Signification |
|--------|---------------|
| Resources | Le job attend que des ressources (nœuds/CPUs) se libèrent |
| Priority | Le job attend car d'autres jobs ont une priorité plus élevée |
| None | Le job est en cours d'exécution |

**Analyse :**
- Les jobs 64-67 (avec `--exclusive`) sont en attente pour cause de **Resources** : ils ont besoin d'un accès exclusif aux nœuds qui sont occupés.
- Les jobs 68-70 (sans `--exclusive`) sont en attente pour cause de **Priority** : ils pourraient partager les ressources mais les jobs exclusifs sont prioritaires dans la file d'attente.

---

## Q4 : Lancement avec QOS debug

**Objectif :** Lancer deux jobs avec la QOS `debug` et observer le comportement.

**Lancements avec QOS debug :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n4 -N4 --qos debug script-2.2.sh
Submitted batch job 71
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n4 -N4 --qos debug script-2.2.sh
Submitted batch job 72
```

**Observation avec squeue :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,state,qos,timeused,reason
JOBID               STATE               QOS                 TIME                REASON              
71                  PENDING             debug               0:00                Resources           
72                  PENDING             debug               0:00                Priority            
64                  PENDING             normal              0:00                Resources           
65                  PENDING             normal              0:00                Resources           
66                  PENDING             normal              0:00                Resources           
67                  PENDING             normal              0:00                Resources           
69                  PENDING             normal              0:00                Priority            
70                  PENDING             normal              0:00                Priority            
68                  RUNNING             normal              0:29                None                
63                  RUNNING             normal              5:26                None                
```

### Constatation :

1. **Les jobs debug (71, 72) apparaissent en tête de la file d'attente**, avant les jobs normal (64-70), grâce à leur priorité plus élevée (Priority = 10).

2. **Le job 72 est en attente avec REASON = Priority** : ceci est dû à la propriété **MaxJobsPU = 1** de la QOS `debug`. Un utilisateur ne peut avoir qu'un seul job debug actif ou en première position de la file.

3. **Le job 71 attend les Resources** : il sera le prochain à démarrer dès que les ressources se libèrent.

### Explication basée sur les propriétés des QOS :

| Propriété | Impact observé |
|-----------|----------------|
| **Priority = 10** | Les jobs debug passent devant les jobs normal dans la file |
| **MaxJobsPU = 1** | Le job 72 doit attendre que le job 71 se termine |
| **MaxWall = 5min** | Les jobs debug sont limités à 5 minutes (non visible ici) |

**Conclusion :** La QOS `debug` est conçue pour des tests rapides :
- **Avantage** : Priorité élevée pour un démarrage rapide
- **Contraintes** : Durée limitée (5 min) et un seul job à la fois par utilisateur

Cela évite qu'un utilisateur monopolise les ressources avec de nombreux jobs debug prioritaires.

---

# 4. Comportements aux limites

L'idée est maintenant de constater le comportement de Slurm lorsqu'un job batch ne se comporte pas de la façon attendue ou que le système lui même introduit des problèmes.

---

## Q1 : Limite de temps d'exécution (time limit)

**Objectif :** Lancer le script `script-2.3.sh` en demandant 4 tâches et un temps d'allocation de ressources de 1 minute seulement (`-t 1`).

**Script `script-2.3.sh` (avec sleep 120) :**
```bash
#!/bin/bash
for i in 1 2 3; do
    srun sleep 120
done
exit 0
```

**Premier test avec `-t 1` (1 minute) :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -t 1 script-2.3.sh
Submitted batch job 73
```

**Suivi de l'exécution :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,name,state,timeused
JOBID               NAME                STATE               TIME                
73                  script-2.3.sh       RUNNING             0:31                
```

**État après terminaison :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 73 -o jobid,user,jobname%15,state,elapsed
       JobID      User         JobName      State    Elapsed 
------------ --------- --------------- ---------- ---------- 
73           yahdhih.+   script-2.3.sh  COMPLETED   00:01:00 
73.batch                         batch  COMPLETED   00:01:00 
73.0                             sleep  COMPLETED   00:01:00 
```

**Observation :** Le job s'est terminé avec **State = COMPLETED** après exactement 1 minute. Le script a été modifié pour avoir un sleep de 60 secondes (au lieu de 120) pour tenir dans la limite de temps.

**Test avec un script dépassant la limite de temps (sleep 120) :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -t 1 script-2.3.sh
Submitted batch job 74
```

**État après terminaison :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 74 -o jobid,user,jobname%15,state,elapsed
       JobID      User         JobName      State    Elapsed 
------------ --------- --------------- ---------- ---------- 
74           yahdhih.+   script-2.3.sh    TIMEOUT   00:01:04 
74.batch                         batch  CANCELLED   00:01:05 
74.0                             sleep  CANCELLED   00:01:04 
```

### Analyse :

| Step | State | Elapsed | Explication |
|------|-------|---------|-------------|
| 74 (job) | TIMEOUT | 00:01:04 | Le job a dépassé la limite de temps |
| 74.batch | CANCELLED | 00:01:05 | Le batch a été annulé suite au timeout |
| 74.0 (sleep) | CANCELLED | 00:01:04 | Le step en cours a été annulé |

**Réponses :**
- **State associé au job une fois terminé :** `TIMEOUT`
- **Nombre de steps exécutés :** 1 seul step (74.0) a été lancé avant le timeout. La boucle prévoyait 3 steps mais seul le premier a pu démarrer avant que la limite de temps soit atteinte.

---

## Q2 : Limite de mémoire par CPU

**Objectif :** Lancer un script consommant 128Mo de mémoire avec une limite de 100Mo par CPU.

**Script `gen100Mo.sh` :**
```bash
#!/bin/bash
srun dd if=/dev/zero of=/dev/shm/128Mo bs=1M count=128
sleep 30
rm /dev/shm/128Mo
exit 0
```

**Lancement avec limite de mémoire :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch --mem-per-cpu=100 -n1 gen100Mo.sh
Submitted batch job 76
```

**Suivi de l'exécution :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue
             JOBID PARTITION     NAME     USER ST       TIME  NODES NODELIST(REASON)
                76        vm gen100Mo yahdhih.  R       0:12      1 vm0
```

**État après terminaison :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 76 -o jobid,user,jobname%15,state,elapsed,exit
       JobID      User         JobName      State    Elapsed ExitCode 
------------ --------- --------------- ---------- ---------- -------- 
76           yahdhih.+     gen100Mo.sh  COMPLETED   00:00:30      0:0 
76.batch                         batch  COMPLETED   00:00:30      0:0 
76.0                                dd  CANCELLED   00:00:00      0:9 
```

### Analyse des états de sortie :

| Step | State | ExitCode | Explication |
|------|-------|----------|-------------|
| 76 (job) | COMPLETED | 0:0 | Le job s'est terminé normalement |
| 76.batch | COMPLETED | 0:0 | Le script batch s'est terminé normalement |
| 76.0 (dd) | CANCELLED | 0:9 | Le step `dd` a été tué (signal 9 = SIGKILL) |

### Explication du contenu du listing :

1. **Le step `dd` a été annulé** (CANCELLED) avec ExitCode `0:9` car il a tenté d'allouer 128Mo alors que la limite était de 100Mo par CPU.

2. **Le signal 9 (SIGKILL)** indique que Slurm a forcé la terminaison du processus car il dépassait la limite de mémoire autorisée.

3. **Le job global est COMPLETED** car le script batch a continué après l'échec du step `dd` (les commandes `sleep 30` et `rm` ont pu s'exécuter ou le script s'est terminé proprement).

4. **Durée de 00:00:00 pour le step dd** : le processus a été tué quasiment immédiatement après avoir dépassé la limite mémoire.

**Conclusion :** Slurm surveille activement l'utilisation mémoire et tue les processus qui dépassent les limites configurées.

---

## Q3 : Défaillance d'un nœud (NODE_FAIL)

**Objectif :** Lancer le script `script-2.3.sh` avec 4 tâches sans restriction, puis provoquer le reboot d'un nœud utilisé par le job.

**Lancement du job :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n 4 script-2.3.sh
Submitted batch job 77
```

**Vérification du job en cours :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue
             JOBID PARTITION     NAME     USER ST       TIME  NODES NODELIST(REASON)
                77        vm script-2 yahdhih.  R       0:08      2 vm[0-1]
```

**Le job utilise les nœuds vm0 et vm1.**

**Reboot du nœud vm1 :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sudo ssh vm1 reboot
Connection to vm1 closed by remote host.
```

**État du cluster après le reboot :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sinfo
PARTITION AVAIL  TIMELIMIT  NODES  STATE NODELIST
vm*          up   infinite      1   comp vm0
vm*          up   infinite      1   idle vm2
vm*          up   infinite      2   down vm[1,3]
```

**État final du job :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 77 -o jobid,user,jobname%15,state,elapsed,exit
       JobID      User         JobName      State    Elapsed ExitCode 
------------ --------- --------------- ---------- ---------- -------- 
77           yahdhih.+   script-2.3.sh  NODE_FAIL   00:03:48      1:0 
77.batch                         batch  CANCELLED   00:03:49     0:15 
77.0                             sleep  COMPLETED   00:01:30      0:0 
77.1                             sleep  COMPLETED   00:01:30      0:0 
77.2                             sleep  NODE_FAIL   00:00:47      0:0 
```

### Analyse des états :

| Step | State | ExitCode | Explication |
|------|-------|----------|-------------|
| 77 (job) | NODE_FAIL | 1:0 | Le job a échoué à cause d'une défaillance de nœud |
| 77.batch | CANCELLED | 0:15 | Le batch a été interrompu (signal SIGTERM) |
| 77.0 | COMPLETED | 0:0 | Premier step terminé normalement |
| 77.1 | COMPLETED | 0:0 | Deuxième step terminé normalement |
| 77.2 | NODE_FAIL | 0:0 | Troisième step en cours lors de la défaillance |

### Réponse à la question :

**Quel est le « state » associé au job une fois terminé ?**

Le state est **NODE_FAIL**, ce qui indique que le job a échoué suite à une défaillance d'un nœud de calcul.

### Observations complémentaires :

1. **Les steps 77.0 et 77.1 sont COMPLETED** : ils ont eu le temps de se terminer avant le reboot du nœud.

2. **Le step 77.2 est en NODE_FAIL** : c'est le step qui était en cours d'exécution au moment où vm1 a été redémarré.

3. **Les nœuds vm1 et vm3 sont marqués "down"** dans `sinfo` : Slurm détecte automatiquement les nœuds non disponibles.

4. **ExitCode 1:0** pour le job global indique une erreur (code de retour 1) sans signal.

**Conclusion :** Slurm gère les défaillances de nœuds de manière transparente en marquant les jobs affectés avec l'état `NODE_FAIL` et en mettant les nœuds défaillants en état `down`.

---

# 5. Gestion des nœuds

L'état actuel du cluster n'est pas complètement opérationnel. Le reboot de vm1 a changé son état et ne le rend plus éligible à l'allocation. Il est donc nécessaire de corriger ce problème pour pouvoir à nouveau l'utiliser.

---

## Q1 : Lancement d'un script nécessitant 4 nœuds exclusivement

**Objectif :** Lancer un script utilisant les 4 nœuds exclusivement, sachant qu'un nœud est down.

**Lancement du job :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n4 -N4 --exclusive script-2.3.sh
Submitted batch job 58
```

**État du cluster :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sinfo
PARTITION AVAIL  TIMELIMIT  NODES  STATE NODELIST
vm*          up   infinite      1   comp vm0
vm*          up   infinite      2   idle vm[2-3]
vm*          up   infinite      1   down vm1
```

**État du job :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue
             JOBID PARTITION     NAME     USER ST       TIME  NODES NODELIST(REASON)
                58        vm script-2 yahdhih. PD       0:00      4 (Resources)
```

### Constatation :

Le job est en attente (**PENDING**) avec la raison **Resources**. Il demande 4 nœuds mais seulement 3 sont disponibles (vm1 est down). Le job ne peut pas démarrer tant que les 4 nœuds ne sont pas disponibles.

**Informations sur le nœud down :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sinfo -o "%10N %8T %30E %20H"
NODELIST   STATE    REASON                         TIMESTAMP           
vm[0,2-3]  idle     none                           Unknown             
vm1        down     Node unexpectedly rebooted     2026-02-05T05:10:13 
```

La raison du down est "Node unexpectedly rebooted" - Slurm a détecté un reboot inattendu.

---

## Q2 : Remise en production du nœud vm1

**Objectif :** Remettre le nœud vm1 en « production » à l'aide de la commande `scontrol`.

**Commande utilisée :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sudo scontrol update nodename=vm1 state=idle
```

**État du cluster après :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sinfo
PARTITION AVAIL  TIMELIMIT  NODES  STATE NODELIST
vm*          up   infinite      4  alloc vm[0-3]
```

**État du job :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue
             JOBID PARTITION     NAME     USER ST       TIME  NODES NODELIST(REASON)
                58        vm script-2 yahdhih.  R       0:28      4 vm[0-3]
```

### Conséquences sur le job :

Le job 58 qui était en attente (PENDING) est maintenant en cours d'exécution (**RUNNING**) sur les 4 nœuds vm[0-3]. Dès que vm1 a été remis en état idle, Slurm a automatiquement démarré le job en attente.

**Détails du job avec scontrol :**
```
[yahdhih.abdelwedoud@vm0 ~]$ scontrol show jobid=58
JobId=58 JobName=script-2.3.sh
   UserId=yahdhih.abdelwedoud(1003) GroupId=yahdhih.abdelwedoud(1003)
   Priority=76615 Nice=0 Account=guests QOS=normal
   JobState=RUNNING Reason=None Dependency=(null)
   Requeue=0 Restarts=0 BatchFlag=1 Reboot=0 ExitCode=0:0
   RunTime=00:00:57 TimeLimit=UNLIMITED TimeMin=N/A
   SubmitTime=2026-02-05T05:10:20 EligibleTime=2026-02-05T05:10:20
   StartTime=2026-02-05T05:22:03 EndTime=Unknown Deadline=N/A
   NodeList=vm[0-3]
   NumNodes=4 NumCPUs=8 CPUs/Task=1
   ...
```

---

## Q3 : Utilisation de l'option --requeue

**Objectif :** Réaliser la même opération qu'en Q2 mais en ajoutant l'option `--requeue` lors du sbatch. Observer la différence lorsque le nœud devient down puis est remis en production.

**Lancement avec --requeue :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch --requeue -n 4 script-2.3.sh
Submitted batch job 59
```

**État du cluster :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sinfo
PARTITION AVAIL  TIMELIMIT  NODES  STATE NODELIST
vm*          up   infinite      2  alloc vm[0-1]
vm*          up   infinite      2   idle vm[2-3]
```

**Reboot du nœud vm1 pendant l'exécution :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sudo ssh vm1 reboot
Connection to vm1 closed by remote host.
```

**État du job après le reboot :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,name,state,timeused
JOBID               NAME                STATE               TIME                
59                  script-2.3.sh       COMPLETING          0:00                
```

**Remise en production du nœud :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sudo scontrol update nodename=vm1 state=resume
```

**État du cluster et du job :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sinfo
PARTITION AVAIL  TIMELIMIT  NODES  STATE NODELIST
vm*          up   infinite      4   idle vm[0-3]

[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,name,state,timeused,nodelist
JOBID               NAME                STATE               TIME                NODELIST            
59                  script-2.3.sh       PENDING             0:00                                    
```

### Différence avec Q2 (sans --requeue) :

| Aspect | Sans --requeue | Avec --requeue |
|--------|----------------|----------------|
| État après NODE_FAIL | Job terminé avec NODE_FAIL | Job remis en file d'attente (PENDING) |
| Après remise en prod du nœud | Job reste NODE_FAIL | Job reprend l'exécution |
| Comportement | Le job est définitivement échoué | Le job est automatiquement relancé |

**Conclusion :** Avec l'option `--requeue`, le job n'est pas annulé lorsque le nœud devient down. Il est remis en file d'attente et sera relancé sur d'autres nœuds disponibles. C'est utile pour les jobs critiques qui doivent absolument se terminer.

---

## Q4 : Exclusivité et nœud drainé

**Objectif :** Lancer deux jobs avec allocation exclusive, puis « drainer » un nœud. Observer l'état avant et après la fin du premier job.

Un nœud drainé finit les tâches qui lui sont actuellement attribuées, puis se met hors service et n'accepte plus de nouvelles tâches.

**Lancements des jobs :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n4 -N4 --exclusive script-2.2.sh
Submitted batch job 60
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n4 -N4 --exclusive script-2.2.sh
Submitted batch job 61
```

**Drainage du nœud vm1 :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sudo scontrol update node=vm1 state=drain reason="drainage"
```

**État du cluster après drainage :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sinfo
PARTITION AVAIL  TIMELIMIT  NODES  STATE NODELIST
vm*          up   infinite      1   drng vm1
vm*          up   infinite      3  alloc vm[0,2-3]
```

**État des jobs :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,qos,timeused,state,reason,nodelist
JOBID               QOS                 TIME                STATE               REASON              NODELIST            
61                  normal              0:00                PENDING             Resources                               
60                  normal              1:45                RUNNING             None                vm[0-3]             
```

### Observations :

1. **Le nœud vm1 est en état "drng" (draining)** : il termine les jobs en cours mais n'accepte plus de nouveaux jobs.

2. **Le job 60 continue de tourner** sur les 4 nœuds (y compris vm1) car il était déjà en cours quand le drainage a été lancé.

3. **Le job 61 est PENDING** avec raison "Resources" car vm1 sera bientôt indisponible.

**État après la fin du job 60 :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sinfo
PARTITION AVAIL  TIMELIMIT  NODES  STATE NODELIST
vm*          up   infinite      1  drain vm1
vm*          up   infinite      3   idle vm[0,2-3]
```

Le nœud vm1 passe de "drng" (draining) à "drain" (drainé) une fois le job terminé.

**Remise en production du nœud :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sudo scontrol update node=vm1 state=resume
```

**Le job 61 démarre :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,qos,timeused,state,reason,nodelist
JOBID               QOS                 TIME                STATE               REASON              NODELIST            
61                  normal              0:12                RUNNING             None                vm[0-3]             
```

---

## Q5 : Passage d'un nœud en état "down"

**Objectif :** Lancer un job, puis passer un des nœuds à l'état « down ». Observer l'état du job.

**Lancement du job :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n4 -N4 script-2.2.sh
Submitted batch job 62
```

**Mise du nœud en état down :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sudo scontrol update node=vm1 state=down reason="down"
```

**État du job :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 62
       JobID    JobName  Partition    Account  AllocCPUS      State ExitCode 
------------ ---------- ---------- ---------- ---------- ---------- -------- 
62           script-2.+         vm     guests          4  NODE_FAIL      1:0 
62.batch          batch                guests          1  CANCELLED     0:15 
62.0           hostname                guests          4  COMPLETED      0:0 
62.1              sleep                guests          4  CANCELLED     0:15 
```

### Constatation :

Le job 62 est immédiatement terminé avec l'état **NODE_FAIL**. Contrairement au drainage, la mise en "down" interrompt brutalement les jobs en cours.

**Remise en production du nœud :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sudo scontrol update node=vm1 state=resume
```

**Le job reste en NODE_FAIL :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sacct -j 62
       JobID    JobName  Partition    Account  AllocCPUS      State ExitCode 
------------ ---------- ---------- ---------- ---------- ---------- -------- 
62           script-2.+         vm     guests          4  NODE_FAIL      1:0 
...
```

Remettre le nœud en production ne change rien car le job est déjà terminé. Il aurait fallu utiliser `--requeue`.

---

## Q6 : Différence entre "drain" et "down"

### Tableau comparatif :

| Aspect | drain | down |
|--------|-------|------|
| **Jobs en cours** | Continuent jusqu'à leur fin | Interrompus immédiatement |
| **Nouveaux jobs** | Non acceptés | Non acceptés |
| **État du job affecté** | COMPLETED (termine normalement) | NODE_FAIL (échec) |
| **Transition d'état** | idle → drng → drain | idle → down |
| **Usage typique** | Maintenance planifiée | Panne ou urgence |

### Résumé :

- **drain** : Le nœud termine gracieusement ses tâches en cours avant de se mettre hors service. Idéal pour la maintenance planifiée.

- **down** : Le nœud est immédiatement retiré du service, tous les jobs en cours sont interrompus avec l'état NODE_FAIL. Utilisé en cas de panne ou d'urgence.

**Pour remettre un nœud en production :**
```bash
sudo scontrol update node=<nom_noeud> state=resume
# ou
sudo scontrol update nodename=<nom_noeud> state=idle
```

---

# 6. Test du Backfilling

Le cluster est pleinement opérationnel sans aucun job en cours d'exécution. Cette partie illustre le fonctionnement du « backfilling » - une technique d'ordonnancement où des jobs de moindre priorité peuvent passer avant des jobs prioritaires s'ils peuvent se terminer avant que les ressources ne soient disponibles pour le job prioritaire.

**Scripts utilisés :**
- **bkf-1.sh** : Utilise une partie des ressources (2 nœuds) pour une période relativement longue
- **bkf-2.sh** : Nécessite l'intégralité des ressources du cluster (4 nœuds exclusifs) avec grande priorité
- **bkf-3.sh** : Moins prioritaire, utilise l'espace non occupé par le premier pour une durée longue (non backfillable)
- **bkf-4.sh** : Moins prioritaire, utilise l'espace non occupé par le premier pour une durée courte (backfillable)

---

## Q1 : Mise en place du scénario de backfilling

**Objectif :** Lancer les 4 jobs dans un ordre approprié pour créer un scénario de backfilling, puis démontrer que le 4ème job a été « backfillé » (passé avant un job plus prioritaire).

**Lancement des 4 jobs :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -t 2 -n2 -N2 bkf-1.sh ; sbatch -t 3 -n4 -N4 --exclusive --nice=50 bkf-2.sh ; sbatch -t 4 -n2 -N2 --nice=100 bkf-3.sh ; sbatch -t 1 -n1 -N1 --nice=100 bkf-4.sh 
Submitted batch job 69
Submitted batch job 70
Submitted batch job 71
Submitted batch job 72
```

### Configuration des jobs :

| Job ID | Script | Nœuds | Temps max | Nice | Priorité attendue |
|--------|--------|-------|-----------|------|-------------------|
| 69 | bkf-1.sh | 2 (-N2) | 2 min | 0 (défaut) | Haute |
| 70 | bkf-2.sh | 4 (--exclusive) | 3 min | 50 | Moyenne |
| 71 | bkf-3.sh | 2 (-N2) | 4 min | 100 | Basse |
| 72 | bkf-4.sh | 1 (-N1) | 1 min | 100 | Basse |

**Note :** Plus la valeur de `--nice` est élevée, plus la priorité est basse.

### Observation initiale avec squeue :
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,qos,timeused,state,reason,nodelist,priority
JOBID               QOS                 TIME                STATE               REASON              NODELIST            PRIORITY            
69                  normal              0:00                PENDING             Resources                               0.00001783645712    
70                  normal              0:00                PENDING             Resources                               0.00001782481559    
71                  normal              0:00                PENDING             Priority                                0.00001781317406    
72                  normal              0:00                PENDING             Priority                                0.00001781317406    
```

### Vérification des priorités avec sprio :

Les priorités confirment l'ordre :
- **Job 69** : Priority = 0.00001783645712 (la plus haute, nice=0)
- **Job 70** : Priority = 0.00001782481559 (2ème, nice=50)
- **Jobs 71 et 72** : Priority = 0.00001781317406 (plus basse, nice=100)

### Observation du backfilling :

**Évolution de l'état des jobs :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,qos,timeused,state,reason,nodelist,priority
JOBID               QOS                 TIME                STATE               REASON              NODELIST            PRIORITY            
70                  normal              0:00                PENDING             Resources                               0.00001782574691    
71                  normal              0:08                RUNNING             None                vm[2-3]             0.00001781410538    
```

### Constatation du backfilling :

**Le job 71 (priorité basse) s'exécute AVANT le job 70 (priorité plus haute) !**

Ceci est le backfilling en action :
1. Le job 70 nécessite 4 nœuds exclusivement → il doit attendre que TOUS les nœuds soient libres
2. Le job 71 ne nécessite que 2 nœuds → il peut utiliser les nœuds libérés par d'autres jobs
3. Slurm calcule que le job 71 peut se terminer AVANT que les 4 nœuds ne soient disponibles pour le job 70
4. Donc le job 71 est « backfillé » → il passe avant le job 70 sans retarder ce dernier

### Ordre de passage des jobs :

| Ordre | Job ID | Script | Raison |
|-------|--------|--------|--------|
| 1 | 69 | bkf-1.sh | Plus haute priorité, premiers nœuds disponibles |
| 2 | 71 | bkf-3.sh | **Backfillé** - peut utiliser les ressources libres sans retarder job 70 |
| 3 | 70 | bkf-2.sh | Attend les 4 nœuds exclusifs |

**Conclusion :** Le backfilling permet d'optimiser l'utilisation des ressources en exécutant des jobs de moindre priorité pendant les « trous » d'ordonnancement, à condition qu'ils ne retardent pas les jobs prioritaires.

---

## Q2 : Annulation du premier job - Limitation du backfilling

**Objectif :** Réaliser le même scénario mais annuler le premier job une fois les autres jobs en exécution. Observer les limitations du backfilling.

**Lancement des jobs :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -t 2 -n2 -N2 bkf-1.sh ; sbatch -t 3 -n4 -N4 --exclusive --nice=50 bkf-2.sh ; sbatch -t 4 -n2 -N2 --nice=100 bkf-3.sh ; sbatch -t 1 -n1 -N1 --nice=100 bkf-4.sh 
Submitted batch job 73
Submitted batch job 74
Submitted batch job 75
Submitted batch job 76
```

**État initial :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,qos,timeused,state,reason,nodelist,priority
JOBID               QOS                 TIME                STATE               REASON              NODELIST            PRIORITY            
74                  normal              0:00                PENDING             Resources                               0.00001782481559    
73                  normal              0:06                RUNNING             None                vm[2-3]             0.00001783645712    
75                  normal              0:06                RUNNING             None                vm[2-3]             0.00001781317406    
76                  normal              0:06                RUNNING             None                vm0                 0.00001781317406    
```

Les jobs 75 (bkf-3) et 76 (bkf-4) ont été backfillés et s'exécutent, tandis que job 74 (bkf-2, besoin de 4 nœuds exclusifs) attend.

**Annulation du job 73 (bkf-1) :**
```
[yahdhih.abdelwedoud@vm0 ~]$ scancel 73
```

**État après annulation :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,qos,timeused,state,reason,nodelist,priority
JOBID               QOS                 TIME                STATE               REASON              NODELIST            PRIORITY            
74                  normal              0:00                PENDING             Resources                               0.00001782481559    
75                  normal              0:35                RUNNING             None                vm[2-3]             0.00001781317406    
76                  normal              0:35                RUNNING             None                vm0                 0.00001781317406    
```

**Observation critique :**

Le job 74 (priorité plus haute) est **toujours en attente** alors que les jobs 75 et 76 (priorité plus basse) continuent de s'exécuter !

**Évolution :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,qos,timeused,state,reason,nodelist,priority
JOBID               QOS                 TIME                STATE               REASON              NODELIST            PRIORITY            
74                  normal              0:00                PENDING             Resources                               0.00001782481559    
75                  normal              2:52                RUNNING             None                vm[2-3]             0.00001781317406    
```

Le job 74 attend toujours, même après la fin du job 76 !

### Limitation du backfilling :

| Situation | Comportement |
|-----------|--------------|
| Avant annulation | Jobs 75, 76 backfillés car ils devaient finir avant que 74 puisse démarrer |
| Après annulation | 74 **ne peut pas préempter** 75 et 76 déjà en cours |
| Résultat | 74 doit attendre la fin de 75 et 76 |

**Conclusion sur les limitations du backfilling :**

1. **Pas de préemption** : Une fois qu'un job est backfillé et en cours d'exécution, il ne peut pas être interrompu même si les conditions changent.

2. **Décision irrévocable** : Les décisions de backfilling sont prises au moment de l'ordonnancement et ne sont pas recalculées dynamiquement.

3. **Inversion de priorité** : En cas de changement de contexte (annulation d'un job), des jobs de basse priorité peuvent bloquer des jobs de haute priorité.

---

## Q3 : Sans temps d'allocation - Besoins du backfilling

**Objectif :** Réaliser le même scénario sans spécifier les temps d'allocation (`-t`). Observer l'impact sur le backfilling.

**Lancement sans limites de temps :**
```
[yahdhih.abdelwedoud@vm0 ~]$ sbatch -n2 -N2 bkf-1.sh ; sbatch -n4 -N4 --exclusive bkf-2.sh ; sbatch -n2 -N2 bkf-3.sh ; sbatch -n1 -N1 bkf-4.sh 
Submitted batch job 77
Submitted batch job 78
Submitted batch job 79
Submitted batch job 80
```

**Note :** Pas d'option `-t` (temps), pas de `--nice` (même priorité pour tous).

**État des jobs :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,qos,timeused,state,reason,nodelist,priority
JOBID               QOS                 TIME                STATE               REASON              NODELIST            PRIORITY            
78                  normal              0:00                PENDING             Resources                               0.00001783645712    
77                  normal              0:07                RUNNING             None                vm[2-3]             0.00001783645712    
79                  normal              0:07                RUNNING             None                vm[2-3]             0.00001783645712    
80                  normal              0:07                RUNNING             None                vm0                 0.00001783645712    
```

### Observations :

1. **Tous les jobs ont la même priorité** (0.00001783645712) car aucun `--nice` n'a été spécifié.

2. Les jobs 77, 79, 80 s'exécutent mais **ce n'est pas du backfilling** - c'est simplement l'ordre de soumission avec même priorité.

**Évolution dans le temps :**
```
[yahdhih.abdelwedoud@vm0 ~]$ squeue -O jobid,qos,timeused,state,reason,nodelist,priority
JOBID               QOS                 TIME                STATE               REASON              NODELIST            PRIORITY            
78                  normal              0:00                PENDING             Resources                               0.00001783645712    
77                  normal              1:47                RUNNING             None                vm[2-3]             0.00001783645712    
79                  normal              1:47                RUNNING             None                vm[2-3]             0.00001783645712    
```

Le job 78 reste PENDING indéfiniment avec la raison "Resources".

### Analyse : Pourquoi pas de backfilling ?

| Avec `-t` (temps spécifié) | Sans `-t` |
|---------------------------|-----------|
| Slurm connaît la durée maximale des jobs | Durée = UNLIMITED |
| Peut calculer quand les ressources seront libres | Impossible de prédire la fin des jobs |
| Peut décider si un job peut être backfillé | Aucun backfilling possible |

### Besoins du backfilling :

Pour que le backfilling fonctionne, Slurm a besoin de :

1. **TimeLimit défini** : L'option `-t` ou `--time` doit être spécifiée pour que Slurm puisse estimer quand les ressources seront libérées.

2. **Différences de priorité** : Les jobs doivent avoir des priorités différentes (via `--nice` ou autres facteurs) pour qu'il y ait un intérêt à backfiller.

3. **Ressources partielles** : Le job à backfiller doit nécessiter moins de ressources que celles actuellement occupées.

**Conclusion :**

Sans spécification de temps (`-t`), Slurm ne peut pas prédire quand les jobs en cours se termineront, donc :
- Il ne peut pas garantir qu'un job backfillé ne retardera pas un job prioritaire
- Le backfilling est **désactivé** par sécurité
- Les jobs s'exécutent simplement dans l'ordre de priorité/soumission

**Recommandation :** Toujours spécifier un temps maximum avec `-t` ou `--time` pour permettre une ordonnancement optimal avec backfilling.
