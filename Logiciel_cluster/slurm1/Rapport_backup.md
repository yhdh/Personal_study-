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

## Q4 : Se connecter à la première machine virtuelle vm0 et vérifier l’état du cluster avec la commande sinfo -l.

(pcocc/37993) bash-4.2$ pcocc ssh vm0
[yahdhih.abdelwedoud@vm0 ~]$ sinfo -l 
Mon Feb  2 15:46:05 2026
PARTITION AVAIL  TIMELIMIT   JOB_SIZE ROOT    SHARE     GROUPS  NODES       STATE NODELIST
vm*          up   infinite 1-infinite   no       NO        all      4        idle vm[0-3]


Ce resultat montre le succès de lanacemet de ces commande

## Q5 : Décrivez le cluster d’après la sortie de la commande sinfo -l

Le cluster possède une seule partition nommée vm (partition par défaut, marquée *).
La partition est disponible (up).
Le temps limite est infini (TIMELIMIT = infinite).
Les jobs peuvent demander de 1 à un nombre illimité de tâches (JOB_SIZE = 1-infinite).
Les nœuds n’acceptent pas l’exécution en root (ROOT = no).
Il n’y a pas de partage des ressources (SHARE = NO) : allocation exclusive.
Tous les groupes d’utilisateurs sont autorisés (GROUPS = all).
Le cluster contient 4 nœuds : vm0, vm1, vm2, vm3.
Tous les nœuds sont actuellement libres (STATE = idle).

## Q6 : Vérifier le bon fonctionnement de slurm en réalisant une première exécution interactive.
La commande "srun -n 4 -N 4 hostname" renvoie l'erreur suivante : 

[yahdhih.abdelwedoud@vm0 ~]$ srun -n 4 -N 4 hostname
srun: error: Unable to allocate resources: Invalid account or account/partition combination specified 

Puis en lançant la deuxième commande indiquée on trouve le resultat suivant : 
[yahdhih.abdelwedoud@vm0 ~]$ sudo sacctmgr -i add user name=yahdhih.abdelwedoud cluster=ensiie account=guests fairshare=10 qos=normal,debug
 Adding User(s)
  yahdhih.abdelwedoud
 Settings =
  Default Account = (null)
 Associations =
  U = yahdhih.a A = guests     C = ensiie    
 Non Default Settings
  Fairshare     = 10
  QOS           = debug,normal

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

