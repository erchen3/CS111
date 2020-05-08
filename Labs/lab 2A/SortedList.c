/*                                                                                                                                             
NAME: Eric Chen                                                                                                                                
EMAIL: erchen3pro@gmail.com                                                                                                                    
ID:                                                                                                                                
*/

#include "SortedList.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>
/**                                                                                              
 * SortedList_insert ... insert an element into a sorted list                                    
 *                                                                                               
 *      The specified element will be inserted in to                                             
 *      the specified list, which will be kept sorted                                            
 *      in ascending order based on associated keys                                              
 *                                                                                               
 * @param SortedList_t *list ... header for the list                                             
 * @param SortedListElement_t *element ... element to be added to the list                       
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
  if(list ==NULL || element ==NULL){
    return;
  }
  SortedListElement_t *current = list->next;
  while(current != list){//traverse
    if(strcmp(element->key, current->key) <=0){ //found a spot
	break;
    }
    current= current->next;
  }
  if(opt_yield & INSERT_YIELD){
    sched_yield();
  }
  //update the pointers

  element->next = current;
  element->prev =current->prev;
  current->prev->next =element;
  current->prev = element;
}
/**                                                                                              
 * SortedList_delete ... remove an element from a sorted list                                    
 *                                                                                               
 *      The specified element will be removed from whatever                                      
 *      list it is currently in.                                                                 
 *                                                                                               
 *      Before doing the deletion, we check to make sure that                                    
 *      next->prev and prev->next both point to this node                                        
 *                                                                                               
 * @param SortedListElement_t *element ... element to be removed                                 
 *                                                                                               
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers                      
 *                                                                                               
 */


int SortedList_delete( SortedListElement_t *element){
  if(element == NULL)//not even valid
    return 1;
  
  
  if( (element->next->prev == element) && (element->prev->next == element)){
    if(opt_yield & DELETE_YIELD){
      sched_yield();
    }
    element->prev->next = element->next;
    element->next->prev = element->prev;
    return 0;
  }
  // fprintf(stderr,"seoncd");
  return 1;
}



/**                                                                                              
 * SortedList_lookup ... search sorted list for a key                                            
 *                                                                                               
 *      The specified list will be searched for an                                               
 *      element with the specified key.                                                          
 *                                                                                               
 * @param SortedList_t *list ... header for the list                                             
 * @param const char * key ... the desired key                                                   
 *                                                                                               
 * @return pointer to matching element, or NULL if none is found                                 
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
  if(list ==NULL ||key==NULL){
    return NULL;
  }
  SortedListElement_t *curr = list->next;
  while(curr != NULL){//traverse to the end
    if(strcmp(curr->key,key) ==0){// we found it
      return curr;
    }
    if(opt_yield & LOOKUP_YIELD){
      sched_yield();
    }
    curr = curr->next;
  }
  return NULL;

}
/**                                                                                              
 * SortedList_length ... count elements in a sorted list                                         
 *      While enumeratign list, it checks all prev/next pointers                                 
 *                                                                                               
 * @param SortedList_t *list ... header for the list                                             
 *                                                                                               
 * @return int number of elements in list (excluding head)                                       
 *         -1 if the list is corrupted                                                           
 */
int SortedList_length(SortedList_t *list){
  int count = 0;
  if(list == NULL)
    return -1;

  SortedListElement_t *curr = list->next;
  while(curr != list){
    if(curr == NULL){
      return -1;
    }
    if(opt_yield & LOOKUP_YIELD){
      sched_yield();
    }
    curr =curr->next;
    count++;
  }
  return count;

}
  
  
