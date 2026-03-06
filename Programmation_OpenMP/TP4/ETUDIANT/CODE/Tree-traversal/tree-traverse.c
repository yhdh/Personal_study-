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
int inorderTraverse_Task(struct node *);


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

  start_time=omp_get_wtime();

  depth = inorderTraverse_Task(root); // Function call to perform the tree 

  end_time=omp_get_wtime();

  printf("\n\t\t Total Nodes: %ld ",totalNodes);
  printf("\n\t\t Number of Threads: %ld ",numThreads);
  printf("\n\t\t Time Taken ( Task Construct : OpenMp-3.0 ): %lf sec",(end_time-start_time));
  printf("\n\t\t Depth: %d ", depth);
  printf("\n\n\t Inorder tree traversal using Task Construct .................Done \n");

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

/*
Description :Uses OpenMP-3.0 Task Construct which is an efficient approah. 
Whenever a thread encounters a task construct,a new explicit task, An explicit 
task may be executed by any thread in the current team, in parallel with other 
tasks.In this approach the several task can be executed in parallel.

@param : starting pointer of the tree
*/
int inorderTraverse_Task(struct node *r)
{
  int depth = 0;
  return depth;
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
