#include <stdio.h>
#include <stdlib.h>



typedef struct Node{
	int data; 
	struct Node *next; 
	struct Node *prev; 
}Node; 


Node* createNode(int data){
	Node* newNode =(Node*)malloc(sizeof(Node)); 
	newNode->data = data; 
	newNode->next = NULL; 
	newNode->prev = NULL; 
	return newNode; 
}

void insertAtBeginning(Node** head, int data){
	Node* newNode = createNode(data); 
	
	if(*head == NULL){
		*head = newNode;
		return; 
	}
	newNode->next = *head; 
	(*head)->prev = newNode; 
	*head = newNode; 
}

void insertAtEnd(Node** head, int data){
	Node*  newNode = createNode(data); 
	if(*head == NULL){
		(*head) = newNode;
		return;  
	}
	Node* temp = *head; 
	while(temp->next != NULL){
		temp = (*head)->next; 
	}
	temp->next = newNode; 
	newNode->prev = temp; 

}

void insertAtPosition(Node** head,int  data, int position){
	Node* newNode = createNode(data); 
	if((*head) == NULL){
		printf("la liste est nulle\n");

	}
	if((*head)->next == NULL){
		insertAtEnd(head,data); 
	}
	Node* temp = *head; 	
	while(temp != NULL && position > 0){ 
		position = position - 1; 
		if(position == 0){
			newNode->next = temp; 
			newNode->prev = temp->prev; 
			temp->prev->next = newNode;
			temp->prev = newNode; 
			return;
		} 
		temp = temp->next;  
		
		
	}
	printf(" la liste n'atient pas le rend position\n"); 
	
}

void PrintForwardNode(Node*  head){
	if(head == NULL){
		printf("la liste à afficher est vide\n");
		
	}
	while(head->next != NULL){
		printf("%d->",head->data); 
		head = head->next; 
	}
	printf("%d",head->data); 
	printf("\n"); 		
}
int main(){
	Node* node = createNode(2);
	PrintForwardNode(node); 
	insertAtEnd(&node,3);
	PrintForwardNode(node); 
	insertAtEnd(&node,5);
	insertAtBeginning(&node,1);
	PrintForwardNode(node); 
	insertAtPosition(&node,4,4);
	PrintForwardNode(node);     
	return 0;  
}
































