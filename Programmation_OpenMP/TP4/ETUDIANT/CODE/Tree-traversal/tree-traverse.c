# include<stdio.h>
# include<stdlib.h>
# include <omp.h>
# include <time.h>

/* Global variable declaration */
long int totalNodes,numThreads;
FILE *fp;

/* Structure define tree node */
struct node
{
  int info ;
  int depth;
  struct node *left;
  struct node *right;
}*root,*p,*q;


/* Function declaration*/
struct node *createTree();
void leftNode(struct node * ,int, int );
void rightNode(struct node * ,int, int );
struct node *createNewNode(int,int);
void deleteTree(struct node ** r);

/* Q.1 : Version séquentielle */
int inorderTraverse_Seq(struct node *);

/* Q.2 : Version parallèle avec tasks OpenMP */
int inorderTraverse_Task(struct node *);

/* Q.3 : Version avec limite de profondeur - méthode 1 (clause if) */
int inorderTraverse_Task_DepthLimit_If(struct node *, int);

/* Q.3 : Version avec limite de profondeur - méthode 2 (clause final) */
int inorderTraverse_Task_DepthLimit_Final(struct node *, int);

/* Q.5 : Version avec taskgroup au lieu de taskwait */
int inorderTraverse_Taskgroup(struct node *);


/* Main Function */
int main(int argc,char **argv)
{
  int index=0;
  double start_time,end_time;

  /* Checking for command line arguments */
  if( argc != 3 )
  {
    printf("\t\t Very Few Arguments\n ");
    printf("\t\t Syntax : exec <total-nodes><No. of Threads>\n");
    exit(-1);
  }

  /* Initalizing the Number of nodes in the tree 
   and the Number of threads*/
  totalNodes = atoi(argv[1]);
  numThreads = atoi(argv[2]);

  if(totalNodes<=0)
  {
    printf("\n\t Error : Number of nodes should be greater then 0\n");
    exit(-1);
  }


  /* Creating the data input file */
  if((fp = fopen("tree-input.dat","w"))==NULL)
  {
    printf("\n\t Failed to open the file \n");
    exit(-1);
  }

  srand(time(NULL));

  /* Writing the input data to the file */
  for (index=0; index<totalNodes ; index++)
  {
    fprintf(fp,"%d ",rand());
  }
  fclose(fp);

  /* Function call to create the tree */
  root = createTree();
  int depth = 0;
  int max_depth_limit = 5; /* Q.3 : profondeur max pour la génération de tâches */

  /* ============================================================ */
  /* Q.1 : Version séquentielle                                   */
  /* ============================================================ */
  start_time=omp_get_wtime();
  depth = inorderTraverse_Seq(root);
  end_time=omp_get_wtime();

  printf("\n\t\t === Q.1 : Version Séquentielle ===");
  printf("\n\t\t Total Nodes: %ld ",totalNodes);
  printf("\n\t\t Time Taken: %lf sec",(end_time-start_time));
  printf("\n\t\t Depth: %d \n", depth);

  /* ============================================================ */
  /* Q.2 : Version parallèle avec tasks OpenMP                    */
  /* ============================================================ */
  start_time=omp_get_wtime();
  #pragma omp parallel num_threads(numThreads)
  {
    #pragma omp single
    {
      depth = inorderTraverse_Task(root);
    }
  }
  end_time=omp_get_wtime();

  printf("\n\t\t === Q.2 : Version Parallèle (tasks) ===");
  printf("\n\t\t Total Nodes: %ld ",totalNodes);
  printf("\n\t\t Number of Threads: %ld ",numThreads);
  printf("\n\t\t Time Taken: %lf sec",(end_time-start_time));
  printf("\n\t\t Depth: %d \n", depth);

  /* ============================================================ */
  /* Q.3a : Limite profondeur - méthode 1 (clause if)             */
  /* ============================================================ */
  start_time=omp_get_wtime();
  #pragma omp parallel num_threads(numThreads)
  {
    #pragma omp single
    {
      depth = inorderTraverse_Task_DepthLimit_If(root, 0);
    }
  }
  end_time=omp_get_wtime();

  printf("\n\t\t === Q.3a : Limite profondeur (clause if, max=%d) ===", max_depth_limit);
  printf("\n\t\t Total Nodes: %ld ",totalNodes);
  printf("\n\t\t Number of Threads: %ld ",numThreads);
  printf("\n\t\t Time Taken: %lf sec",(end_time-start_time));
  printf("\n\t\t Depth: %d \n", depth);

  /* ============================================================ */
  /* Q.3b : Limite profondeur - méthode 2 (clause final)          */
  /* ============================================================ */
  start_time=omp_get_wtime();
  #pragma omp parallel num_threads(numThreads)
  {
    #pragma omp single
    {
      depth = inorderTraverse_Task_DepthLimit_Final(root, 0);
    }
  }
  end_time=omp_get_wtime();

  printf("\n\t\t === Q.3b : Limite profondeur (clause final, max=%d) ===", max_depth_limit);
  printf("\n\t\t Total Nodes: %ld ",totalNodes);
  printf("\n\t\t Number of Threads: %ld ",numThreads);
  printf("\n\t\t Time Taken: %lf sec",(end_time-start_time));
  printf("\n\t\t Depth: %d \n", depth);

  /* ============================================================ */
  /* Q.5 : Version avec taskgroup                                 */
  /* ============================================================ */
  start_time=omp_get_wtime();
  #pragma omp parallel num_threads(numThreads)
  {
    #pragma omp single
    {
      depth = inorderTraverse_Taskgroup(root);
    }
  }
  end_time=omp_get_wtime();

  printf("\n\t\t === Q.5 : Version taskgroup ===");
  printf("\n\t\t Total Nodes: %ld ",totalNodes);
  printf("\n\t\t Number of Threads: %ld ",numThreads);
  printf("\n\t\t Time Taken: %lf sec",(end_time-start_time));
  printf("\n\t\t Depth: %d \n", depth);

  printf("\n\n\t Toutes les versions exécutées avec succès.\n");

  while(root)
  deleteTree(&root);

  return 0;
}

/* Description : Function to create the Binary Search tree 
 @return : Retun the pointer to the root node 
*/
struct node *createTree()
{
  int index,value;

  /* Open the file to read the input data */
  if((fp = fopen("tree-input.dat","r"))==NULL)
  {
    printf("\n\t Failed to open the file \n");
    exit(-1);
  }

  index=totalNodes;

  /* reading the first element */
  fscanf(fp,"%d",&value);

  /* Function call to create the first Node of the tree */
  root=createNewNode(value, 0);
  int depth;

  /* Iterate over the loop to create the tree */
  while (index >1)
  {

    /* Reading the data from the input file */
    fscanf(fp,"%d",&value);
    p=root;
    q=root;
    depth = 0;

    if( value == p->info)
    {
      index--;
      continue;
    }
    else
    {
      /* Check the condition for node insertion in right or left*/
      while(value!=p->info && q!=NULL)
      {
        p=q;
        if(value < p->info )
        {
          q = p->left;
        }
        else
        {
          q = p->right;
        }
        ++depth;
      }

      if( value < p->info)
      {
        /* If the value is less then the node value
           then insert the value in left
        */
        leftNode(p,value, depth);
      }
      else if( value > p->info)
      {
        /* If the value is greater then the node value
           then insert the value in right 
        */
        rightNode(p,value, depth);
      }
      index--;
    }
  }

  fclose(fp);

  return root;
}

/* Description : Helper function to create the new node
 @param[value] : Data value of the node 
 @param[depth] : depth value of the node
*/
struct node *createNewNode(int value, int depth )
{
  struct node *newnode;

  /* Allocating the memory to create the new node */
  if((newnode=(struct node *)malloc(sizeof(struct node)))==NULL)
  {
    perror("\n\t Memory allocation for newnode ");
    exit(-1);
  }

  newnode->info=value;
  newnode->depth=depth;
  newnode->right=newnode->left=NULL;
  return(newnode);

}

/* Description : Function to create the left node of the tree 
 @param [*r]: Position to insert the node
 @param[value] : data value in the node
 @param[depth] : depth value of the node

*/
void leftNode(struct node *r,int value, int depth )
{
  if(r->left!=NULL)
    printf("\n Invalid !");
  else
    r->left=createNewNode(value, depth);
}

/* Description : Function to create the right node of the tree
 @param [*r]: Position to insert the node
 @param[value] : data value in the node
 @param[depth] : depth value of the node
*/
void rightNode(struct node *r,int value, int depth)
{
  if(r->right!=NULL)
    printf("\n Invalid !");
  else
   r->right=createNewNode(value, depth);
}

/* ================================================================
 * Q.1 : Version séquentielle
 * Parcours DFS classique : on descend récursivement à gauche et à
 * droite, et on renvoie 1 + max(profondeur gauche, profondeur droite).
 * ================================================================ */
int inorderTraverse_Seq(struct node *r)
{
  int depthLeft = 0;
  int depthRight = 0;

  if (r->right != NULL)
  {
    depthRight = inorderTraverse_Seq(r->right);
  }
  if (r->left != NULL)
  {
    depthLeft = inorderTraverse_Seq(r->left);
  }

  /* +1 pour compter le niveau courant */
  return (depthLeft >= depthRight) ? depthLeft + 1 : depthRight + 1;
}

/* ================================================================
 * Q.2 : Version parallèle avec tâches OpenMP
 * Chaque appel récursif est encapsulé dans une tâche OpenMP.
 * Le taskwait synchronise les deux sous-arbres avant de
 * renvoyer le max.
 * ================================================================ */
int inorderTraverse_Task(struct node *r)
{
  int depthLeft = 0;
  int depthRight = 0;

  if (r->right != NULL)
  {
    #pragma omp task shared(depthRight)
    {
      depthRight = inorderTraverse_Task(r->right);
    }
  }
  if (r->left != NULL)
  {
    #pragma omp task shared(depthLeft)
    {
      depthLeft = inorderTraverse_Task(r->left);
    }
  }

  #pragma omp taskwait

  return (depthLeft >= depthRight) ? depthLeft + 1 : depthRight + 1;
}

/* ================================================================
 * Q.3 – Méthode 1 : clause "if"
 * On ne crée des tâches que si la profondeur courante est < MAX_DEPTH.
 * Au-delà, la clause if(0) fait que le code s'exécute en séquentiel
 * dans le thread courant (pas de création de tâche).
 * ================================================================ */
#define MAX_DEPTH 5

int inorderTraverse_Task_DepthLimit_If(struct node *r, int currentDepth)
{
  int depthLeft = 0;
  int depthRight = 0;

  if (r->right != NULL)
  {
    #pragma omp task shared(depthRight) if(currentDepth < MAX_DEPTH)
    {
      depthRight = inorderTraverse_Task_DepthLimit_If(r->right, currentDepth + 1);
    }
  }
  if (r->left != NULL)
  {
    #pragma omp task shared(depthLeft) if(currentDepth < MAX_DEPTH)
    {
      depthLeft = inorderTraverse_Task_DepthLimit_If(r->left, currentDepth + 1);
    }
  }

  #pragma omp taskwait

  return (depthLeft >= depthRight) ? depthLeft + 1 : depthRight + 1;
}

/* ================================================================
 * Q.3 – Méthode 2 : clause "final"
 * La clause final fait que, une fois la profondeur MAX_DEPTH atteinte,
 * toutes les tâches enfants deviennent "final" et s'exécutent
 * immédiatement (comme en séquentiel). La différence avec if :
 * - if(0) : le code s'exécute dans le thread courant, pas de tâche
 * - final : une tâche est toujours créée, mais elle et ses descendantes
 *   sont marquées "final" et exécutées immédiatement.
 * ================================================================ */
int inorderTraverse_Task_DepthLimit_Final(struct node *r, int currentDepth)
{
  int depthLeft = 0;
  int depthRight = 0;

  if (r->right != NULL)
  {
    #pragma omp task shared(depthRight) final(currentDepth >= MAX_DEPTH)
    {
      depthRight = inorderTraverse_Task_DepthLimit_Final(r->right, currentDepth + 1);
    }
  }
  if (r->left != NULL)
  {
    #pragma omp task shared(depthLeft) final(currentDepth >= MAX_DEPTH)
    {
      depthLeft = inorderTraverse_Task_DepthLimit_Final(r->left, currentDepth + 1);
    }
  }

  #pragma omp taskwait

  return (depthLeft >= depthRight) ? depthLeft + 1 : depthRight + 1;
}

/* ================================================================
 * Q.5 : Version avec taskgroup au lieu de taskwait
 *
 * Différence taskwait vs taskgroup :
 * - taskwait : attend uniquement les tâches enfants DIRECTES du
 *   thread courant (1 seul niveau de profondeur).
 * - taskgroup : attend TOUTES les tâches créées dans le bloc
 *   taskgroup, y compris les tâches descendants (petits-enfants,
 *   arrière-petits-enfants, etc.).
 *
 * Ici taskgroup garantit que tout le sous-arbre est terminé
 * avant de calculer le max. C'est plus robuste car on n'a
 * pas besoin de s'assurer que chaque niveau fait son propre
 * taskwait.
 * ================================================================ */
int inorderTraverse_Taskgroup(struct node *r)
{
  int depthLeft = 0;
  int depthRight = 0;

  #pragma omp taskgroup
  {
    if (r->right != NULL)
    {
      #pragma omp task shared(depthRight)
      {
        depthRight = inorderTraverse_Taskgroup(r->right);
      }
    }
    if (r->left != NULL)
    {
      #pragma omp task shared(depthLeft)
      {
        depthLeft = inorderTraverse_Taskgroup(r->left);
      }
    }
  } /* Fin du taskgroup : toutes les tâches descendantes sont terminées */

  return (depthLeft >= depthRight) ? depthLeft + 1 : depthRight + 1;
}
/* Description : Function to delete the Binary Search tree
*/
void deleteTree(struct node ** r)
{
  struct node * cur=*r, *temp=NULL, * parent=*r, * succ=NULL;

  if(cur==NULL)
  {
    return;
  }

  temp=*r;

  if(cur->left && cur->right)
  {
    succ=cur->right;

    while(succ->left)
    {
      parent=succ;
      succ=succ->left;
    }
    (*r)->info=succ->info;

    if(parent!=*r)
    {
      parent->left=succ->right;
    }
    else
    {
      parent->right=succ->right;
    }

    temp=succ;
  }
  else
  {
    if(!cur->left)
    {
      *r=cur->right;
    }
    else if(!cur->right)
    {
      *r=cur->left;
    }
  }

  free(temp);
}
