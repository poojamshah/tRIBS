/***************************************************************************
**
**  		     tRIBS Distributed Hydrologic Model 
**
**              TIN-based Real-time Integrated Basin Simulator
**		         Ralph M. Parsons Laboratory
**  		    Massachusetts Institute of Technology
**  
**
**  tMeshList.cpp: Functions for derived classes tMeshList and tMeshListIter. 
**                 (see tMeshList.h)
**
**************************************************************************/

#include "tMeshList/tMeshList.h"

#ifdef PARALLEL_TRIBS
#include "tParallel/tParallel.h"
#endif

//=========================================================================
//
//
//                  Section 1: tMeshList Constructors/Destructors
//
//
//=========================================================================

template< class NodeType >                    
tMeshList< NodeType >::
tMeshList() :
nActiveNodes(0),
lastactive(0) 
{}

template< class NodeType >                    
tMeshList< NodeType >::
tMeshList( const tMeshList< NodeType > *original )
: tList< NodeType >( original )
{
	nActiveNodes = original->nActiveNodes;
	if( original->lastactive != 0 ) lastactive = original->lastactive;
	else lastactive = 0;
}

template< class NodeType >                    
tMeshList< NodeType >::
~tMeshList(){}

//=========================================================================
//
//
//                  Section 2: tMeshList Functions
//
//
//=========================================================================

/**************************************************************************
**
**  tMeshList overloaded operators
**
**  Assignment: creates a copy of right-hand list
**  Equality and inequality: adds test of # of active items and lastactive
**                           to basic tList operations
**
**************************************************************************/

//overloaded assignment operator
template< class NodeType >                     
const tMeshList< NodeType > &tMeshList< NodeType >::
operator=( const tMeshList< NodeType > &right )
{
	if( this != &right ){
		tList< NodeType >::operator=( right );
		lastactive = right.lastactive;
		nActiveNodes = right.nActiveNodes;
	}
	return *this;
}

//overloaded equality operator:
template< class NodeType >                    
int tMeshList< NodeType >::
operator==( const tMeshList< NodeType > &right ) const
{
	if( tList< NodeType >::operator!=( right ) ) return 0;
	if( nActiveNodes != right.nActiveNodes ) return 0;
	if( lastactive != right.lastactive ) return 0;
	return 1;
}

//overloaded inequality operator:
template< class NodeType >                    
int tMeshList< NodeType >::
operator!=( const tMeshList< NodeType > &right ) const
{
	if( tList< NodeType >::operator!=( right ) ) return 1;
	if( nActiveNodes != right.nActiveNodes ) return 1;
	if( lastactive != right.lastactive ) return 1;
	return 0;
}

/**************************************************************************
**
**  tMeshList "get" functions
**
**************************************************************************/

#ifdef PARALLEL_TRIBS
// Global sum of all active nodes
template< class NodeType >
int tMeshList< NodeType >::getGlobalActiveSize() {
   return tParallel::sumBroadcast(nActiveNodes);
}


// Collect number of active nodes in each processor
template< class NodeType >
int* tMeshList< NodeType >::collectActiveSize() {
   return tParallel::collect(nActiveNodes);
}

#endif

template< class NodeType >                    
tListNode< NodeType > *
tMeshList< NodeType >::
getLastActive() const {return lastactive;}

template< class NodeType >                     
void tMeshList< NodeType >::
setNActiveNodes( int val ) {nActiveNodes = ( val >= 0 ) ? val : 0;}

template< class NodeType >                    
int tMeshList< NodeType >::
isActiveEmpty() const{     
	if( lastactive == 0 )    
		return 1;
	else
		return 0;
}

template< class NodeType >                    
int tMeshList< NodeType >::
isBoundEmpty() const{
	if( lastactive == this->last ) return 1;
	else return 0;
}


/**************************************************************************
**
**  tMeshList insertion and removal functions
**
**  Adds and removes items to/from the list. Supplements tList
**  functionality by adding capability to add items to front of
**  "boundary" section or rear of "active" section. Updates
**  nActiveNodes as appropriate.
**
**************************************************************************/

template< class NodeType >                        
void tMeshList< NodeType >::
insertAtFront( const NodeType &value ){
	tList< NodeType >::insertAtFront( value );
	if( isActiveEmpty() ) lastactive = this->first;
	nActiveNodes++;
}

template< class NodeType >                   
void tMeshList< NodeType >::
insertAtBoundFront( const NodeType &value )
{
  tListNode< NodeType > * newPtr = this -> getNewNode( value ); //Added this-> CL 09/05/2020
	assert( newPtr>0 );
	assert( this != 0 );
	
	if( this->isEmpty() )  
		this->first = this->last = newPtr;
	else if( lastactive==0 ) {  
		newPtr->next = this->first;
		this->first = newPtr;
	}
	else  {
		newPtr->next = lastactive->next;
		lastactive->next = newPtr;
		if( lastactive==this->last ) this->last = newPtr; // Case: new is last (only) bdy
	}
}

/* SMM - moved to tMeshList.h
template< class NodeType >                 
int tMeshList< NodeType >::
removeFromBoundFront( NodeType &value )
{
	assert( &value != 0 );
	if( this->isEmpty() ) return 0;
	else if( this->last == lastactive ) return 0;
	else{
		tListNode< NodeType > * temp = lastactive->next;
		if( this->first == this->last ) this->first = this->last = 0;
		else lastactive->next = lastactive->next->next;
		value = temp->data;
		delete temp;
		this->nNodes--;
		return 1;
	}
}
*/

template< class NodeType >                    
void tMeshList< NodeType >::
insertAtActiveBack( const NodeType &value )
{
  tListNode< NodeType > * newPtr = this -> getNewNode( value ); // Added this-> CL 09/05/2020
	assert( this != 0 );
	if( this->isEmpty() )
		this->first = this->last = lastactive = newPtr;
	
	else if( !( this->isEmpty() ) && isActiveEmpty() && !( isBoundEmpty() ) ){
		lastactive = newPtr;
		lastactive->next = this->first;
		this->first = lastactive;
	}
	else if( !( this->isEmpty() ) && isBoundEmpty() ){
		newPtr->next = lastactive->next;
		lastactive->next = newPtr;
		lastactive = newPtr;
		this->last = lastactive;
	}
	else{
		newPtr->next = lastactive->next;
		lastactive->next = newPtr;
		lastactive = newPtr;
	}
	if( isBoundEmpty() ) this->last = lastactive;
	nActiveNodes++;
}

/* SMM - moved to tMeshList.h
template< class NodeType >                    
int tMeshList< NodeType >::
removeFromActiveBack( NodeType &value )
{
	if( this->isEmpty() ) return 0;
	else{
		tListNode< NodeType > * temp = lastactive;
		if( this->first == lastactive ) lastactive = 0;
		if( this->first == this->last ) this->first = this->last = 0;
		else{
			tListNode< NodeType > * current = this->first;
			while( current->next != lastactive ) current = current->next;
			current->next = lastactive->next;
			lastactive->next = 0;
			lastactive = current;
		}
		value = temp->data;
		delete temp;
		this->nNodes--;
		nActiveNodes--;
		return 1;
	}
}
*/

template< class NodeType >                         
int tMeshList< NodeType >::
removeFromFront( NodeType &value )
{
	if( !( isActiveEmpty() ) ){
		nActiveNodes--;
		if( lastactive == this->first ) lastactive = 0;
	}
	return tList< NodeType >::removeFromFront( value );
}

//delete next node
/* SMM - moved to tMeshList.h
template< class NodeType >                         
int tMeshList< NodeType >::
removeNext( NodeType &value, tListNode< NodeType > * ptr )
{
	if( ptr->next == 0 ) return 0;
	if( ptr == 0 ) return 0;
	if( ptr->next == lastactive ) return removeFromActiveBack( value );
	if( ptr == lastactive ) return removeFromBoundFront( value );
	if( tList< NodeType >::removeNext( value, ptr ) ){
		if( !( value.getBoundaryFlag() ) ) nActiveNodes--;
		return 1;
	}
	return 0;
}
*/

//delete previous node
template< class NodeType >                       
int tMeshList< NodeType >::
removePrev( NodeType &value, tListNode< NodeType > * ptr )
{
	if( ptr == 0 ) return 0;
	if( ptr == this->first && this->last->next == 0 ) return 0;
	if( lastactive->next == ptr ) return removeFromActiveBack( value );
	if( tList< NodeType >::removePrev( value, ptr ) ){
		if( !( value.getBoundaryFlag() ) ) nActiveNodes--;
		return 1;
	}
	return 0;
}

/**************************************************************************
**
**  tMeshList::moveToBack ( tListNode * )
**
**  Moves mvnode to the back of the list (the boundary portion).
**  Handles case of moved node being the last active node, in which case
**  _lastactive_ needs to be updated.
**
**************************************************************************/

template< class NodeType >                        
void tMeshList< NodeType >::
moveToBack( tListNode< NodeType > * mvnode ) 
{
	assert( mvnode>0 );
	tListNode< NodeType > * prev;
	if( mvnode != this->last ){
		if( InActiveList( mvnode ) )
			nActiveNodes--;
		if( mvnode == lastactive ){
			if( mvnode != this->first ){
				for( prev = this->first; prev->next != mvnode; prev = prev->next );
				lastactive = prev;
			}
			else
				lastactive = 0;
		}
		tList< NodeType >::moveToBack( mvnode );
	}
}


/**************************************************************************
**
**  tMeshList::moveToBack ( NodeType * )
**
**  Finds the ListNode whose data are identical to mvnodedata and calls
**  moveToBack( tListNode ) to move it to the back of the list.
**
**************************************************************************/

template< class NodeType >                        
void tMeshList< NodeType >::
moveToBack( NodeType * mvnodedata ) {
  assert( this -> getListNode( mvnodedata )!=0 ); //Added this -> CL 09/05/2020 
  moveToBack( this -> getListNode( mvnodedata ) ); // idem
}

/**************************************************************************
**
**  tMeshList::moveToFront()
**
**  Moves mvnode to the front of the list, taking care to handle the case
**  in which the node being moved is the last on the active section
**
**************************************************************************/

template< class NodeType >                         
void tMeshList< NodeType >::
moveToFront( tListNode< NodeType > * mvnode ) 
{
	tListNode< NodeType > *prev;
	if( mvnode != this->first ){
		if( mvnode == lastactive ){
			for( prev = this->first; prev->next != mvnode; prev = prev->next );
			lastactive = prev;
		}
		tList< NodeType >::moveToFront( mvnode );
	}
}


/**************************************************************************
**
**  tMeshList::moveToActiveBack()
**
**  Moves mvnode to the back of the "active" portion of the list
**  (does not update nActiveNodes if the node happens to be inactive!)
**
**************************************************************************/

template< class NodeType >                      
void tMeshList< NodeType >::
moveToActiveBack( tListNode< NodeType > * mvnode ) 
{
	tListNode< NodeType > * prev;
	if( mvnode != lastactive ){
		// Detach mvnode from its position on the list
		if( mvnode == this->first ) this->first = this->first->next;
		else{
			prev = this->first;
			while( prev->next != mvnode ) prev = prev->next;
			prev->next = mvnode->next;
		}
		
		// Insert it at the end of the active part of the list
		mvnode->next = lastactive->next;
		lastactive->next = mvnode;
		if( lastactive == this->last ){
			this->last = mvnode;
			// If it's a circular list, make sure to preserve circularity
			if( this->last->next != 0 ) this->last->next = this->first;
		}
		lastactive = mvnode;
	}
}

/**************************************************************************
**
**  tMeshList::moveToBoundFront()
**
**  Moves mvnode to the front of the "boundary" portion of the list,
**  making sure to update nActiveNodes is the node was previously on
**  the active portion of the list.
**
**************************************************************************/

template< class NodeType >                       
void tMeshList< NodeType >::
moveToBoundFront( tListNode< NodeType > * mvnode ) 
{
	tListNode< NodeType > * prev;
	if( mvnode != lastactive->next ){
		// if node was in active part of list, decrement nActiveNodes
		if( InActiveList( mvnode ) ) --nActiveNodes;
		// Detach mvnode from its position on the list
		if( mvnode == this->first ) this->first = this->first->next;
		else{
			prev = this->first;
			while( prev->next != mvnode ) prev = prev->next;
			prev->next = mvnode->next;
		}
		
		// Insert it after the end of the active part of the list
		mvnode->next = lastactive->next;
		lastactive->next = mvnode;
		if( lastactive == this->last ){
			this->last = mvnode;
			// If it's a circular list, make sure to preserve circularity
			if( this->last->next != 0 ) this->last->next = this->first;
		}
	}
}


/**************************************************************************
**
**  tMeshList::Flush()
**
**  Also reinitializes lastactive and nActiveNodes
**
**************************************************************************/

template< class NodeType >                         
void tMeshList< NodeType >::
Flush(){
	tList< NodeType >::Flush();
	lastactive = 0;
	nActiveNodes = 0;
}

/**************************************************************************
**
**  tMeshList::InActiveList
**
**  Reports whether a given list node is in the active portion of the list.
**
**  Parameters:  theNode -- list node to test
**  Returns:  1 if theNode is present in the active portion of the list,
**            0 otherwise.
**
**************************************************************************/

template< class NodeType >                       
int tMeshList< NodeType >::
InActiveList( tListNode< NodeType > * theNode ){
	tListNode< NodeType > * listnode = this->first;
	if( nActiveNodes==0 ) return 0;
	while( listnode!=lastactive && listnode!=theNode )
		listnode = listnode->next;
	if( listnode==theNode ) return 1;
	else return 0;   
}

//=========================================================================
//
//
//                  Section 3: tMeshListIter Constructor and Functions
//
//
//=========================================================================

template< class NodeType >   
tMeshListIter< NodeType >::
tMeshListIter(){ }

template< class NodeType >   
tMeshListIter< NodeType >::
tMeshListIter( tMeshList< NodeType > &list )
: tListIter< NodeType >( list )
{
	assert( &list != 0 );  
	this->curnode = this->listPtr->first;
}

/**************************************************************************
**
**  tMeshListIter::LastActive()
**
**  Moves the iterator to the last active node.
**
**************************************************************************/

template< class NodeType >   
int tMeshListIter< NodeType >::
LastActive(){
	tMeshList< NodeType > *meshlistPtr;
	meshlistPtr = ( tMeshList< NodeType > * ) this->listPtr;
	assert( meshlistPtr != 0 );
	this->curnode = meshlistPtr->lastactive;
	if( this->curnode != 0 ) return 1;
	else return 0;
}


/**************************************************************************
**
**  tMeshListIter::FirstBoundary()
**
**  Moves the iterator to the first boundary node.
**
**************************************************************************/

template< class NodeType >  
int tMeshListIter< NodeType >::
FirstBoundary(){
	tMeshList< NodeType > *meshlistPtr;
	meshlistPtr = ( tMeshList< NodeType > * ) this->listPtr;
	assert( meshlistPtr != 0 );
	if( meshlistPtr->isActiveEmpty() ) this->curnode = this->listPtr->first;
	else if( meshlistPtr->isBoundEmpty() ) this->curnode = 0;
	else this->curnode = meshlistPtr->lastactive->next;
	if( this->curnode != 0 ) return 1;
	else return 0;
}

/**************************************************************************
**
**  tMeshListIter::FirstBoundaryP()
**
**  Moves the iterator to the first boundary node and returns a pointer
**  to the data at that location.
**
**************************************************************************/

template< class NodeType >   
NodeType* tMeshListIter< NodeType >::
FirstBoundaryP(){
	tMeshList< NodeType > *meshlistPtr;
	meshlistPtr = ( tMeshList< NodeType > * ) this->listPtr;
	assert( meshlistPtr != 0 );
	if( meshlistPtr->isActiveEmpty() ) this->curnode = this->listPtr->first;
	else if( meshlistPtr->isBoundEmpty() ) this->curnode = 0;
	else this->curnode = meshlistPtr->lastactive->next;
	if( this->curnode != 0 ) return this->curnode->getDataPtrNC();
	else return 0;
}

/**************************************************************************
**
**  tMeshListIter::LastActiveP()
**
**  Moves the iterator to the last active node and returns a pointer
**  to the data at that location.
**
**************************************************************************/

template< class NodeType >   
NodeType *tMeshListIter< NodeType >::
LastActiveP(){
	tMeshList< NodeType > *meshlistPtr;
	meshlistPtr = ( tMeshList< NodeType > * ) this->listPtr;
	assert( meshlistPtr != 0 );
	this->curnode = meshlistPtr->lastactive;
	if( this->curnode != 0 ) return this->curnode->getDataPtrNC();
	else return 0;
}

/**************************************************************************
**
**  tMeshListIter::IsActive()
**
**  Indicates whether the current item is on the active portion of the
**  list, returning 1 if so, 0 if not. Assumes NodeType has a member
**  function getBoundaryFlag.
**
**************************************************************************/


//=========================================================================
//
//
//                       End of tMeshList.cpp
//
//
//=========================================================================
