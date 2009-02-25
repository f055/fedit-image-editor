/*
List.h
Partially inherited from Fiew 2.0
Custom typed list with various methods enhancing it's usage.
The list is a double linked list with Nodes that encapsulate T objects.
The list has 3 cursors, main cursor 'that', and 2 helper cursors 'left' and 'right'
*/

template<class T>
class List
{
private:
	int count;

	void countUp()
	{
		this->count++;
	}
	void countDown()
	{
		this->count--;
	}

public:
	class Node
	{
	private:
		T *object;

		Node *next;
		Node *prev;

	public:
		Node(T *object)
		{
			this->object = object;

			this->next = NULL;
			this->prev = NULL;
		}
		~Node()
		{
			delete this->object;
		}

		void setNext(Node *next)
		{
			this->next = next;
		}
		void setPrev(Node *prev)
		{
			this->prev = prev;
		}

		Node *getNext()
		{
			return this->next;
		}
		Node *getPrev()
		{
			return this->prev;
		}

		T *getObj()
		{
			return this->object;
		}
		void setObj(T *object)
		{
			this->object = object;
		}

	};
private:
	Node *head, *that, *tail, *left, *right;

public:
	List()
	{
		this->count = 0;

		this->head = NULL;
		this->that = NULL;
		this->tail = NULL;

		this->left = NULL;
		this->right = NULL;
	}
	~List()
	{
		this->that = this->head;

		while( this->that != NULL ){
			this->head = this->that->getNext();
			/* causes assertion error unless vars initialized with 'new' ! */
			delete this->that;
			this->that = this->head;
		}
	}

	bool add(T *object)
	{
		if( object == NULL )
			return false;

		return this->add( new Node(object) );
	}
	bool add(Node *newton)
	{
		if( newton == NULL )
			return false;

		newton->setNext(NULL);
		newton->setPrev(NULL);

		if( this->tail == NULL ){
			this->tail = newton;
			this->that = this->tail;
			this->head = this->tail;

			this->left = this->that;
			this->right = this->that;
		}
		else {
			newton->setPrev(this->tail);
			this->tail->setNext(newton);
			this->tail = newton;
		}
		this->countUp();
		return true;
	}
	bool addToHead(T* object)
	{
		if( object == NULL )
			return false;

		return this->addToHead( new Node(object) );
	}
	bool addToHead(Node *newton)
	{
		if( newton == NULL )
			return false;

		newton->setNext(NULL);
		newton->setPrev(NULL);

		if( this->head == NULL ){
			this->head = newton;
			this->that = this->head;
			this->tail = this->head;

			this->left = this->that;
			this->right = this->that;
		}
		else {
			newton->setNext(this->head);
			this->head->setPrev(newton);
			this->head = newton;
		}
		this->countUp();
		return true;
	}
	bool addToThat(T* object)
	{
		if( object == NULL )
			return false;

		return this->addToThat( new Node(object) );
	}
	bool addToThat(Node *newton)
	{
		if( newton == NULL )
			return false;

		newton->setNext(NULL);
		newton->setPrev(NULL);

		if( this->that == this->tail ){
			return this->add(newton);
		}
		else {
			Node *next = this->that->getNext();

			newton->setNext(next);
			newton->setPrev(this->that);

			this->that->setNext(newton);
			next->setPrev(newton);

			this->countUp();

			return true;
		}
		return false;
	}

	T *remove(Node *object)
	{
		Node *tmp = object;

		if( tmp != NULL ){
			Node *tmpprev = tmp->getPrev();
			Node *tmpnext = tmp->getNext();

			if( tmpprev != NULL )
				tmpprev->setNext(tmpnext);
			if( tmpnext != NULL )
				tmpnext->setPrev(tmpprev);
			if( tmp == this->head )
				this->head = tmpnext;
			if( tmp == this->that )
				if( tmpnext != NULL )
					this->that = tmpnext;
				else
					this->that = tmpprev;
			if( tmp == this->tail )
				this->tail = tmpprev;

			if( tmp == this->left )
				if( tmpprev != NULL )
					this->left = tmpprev;
				else
					this->left = tmpnext;
			if( tmp == this->right )
				if( tmpnext != NULL )
					this->right = tmpnext;
				else
					this->right = tmpprev;

			this->countDown();
			return tmp->getObj();
		}
		return NULL;
	}
	T *remove(T *object)
	{
		Node *tmp = this->head;

		if( tmp == NULL )
			return NULL;
		while( tmp->getObj() != object ){
			tmp = tmp->getNext();
			if( tmp == NULL )
				break;
		}
		if( tmp != NULL ){
			this->remove(tmp);
			return object;
		}
		return NULL;
	}

	T *removeThat()
	{
		return this->remove(this->that);
	}
	T *removeHead()
	{
		return this->remove(this->head);
	}
	T *removeTail()
	{
		return this->remove(this->tail);
	}
	/*
	Move 'that' cursor to right
	*/
	bool next(int id = NULL)
	{
		Node *tmp = this->that;
		if( id == LEFT )
			tmp = this->left;
		else if( id == RIGHT )
			tmp = this->right;

		if( tmp != NULL ){
			if( tmp->getNext() != NULL ){
				tmp = tmp->getNext();
				if( id == LEFT )
					this->left = tmp;
				else if( id == RIGHT )
					this->right = tmp;
				else
					this->that = tmp;
				return true;
			}
		}
		return false;
	}
	/*
	Move 'that' cursor to left
	*/
	bool prev(int id = NULL)
	{
		Node *tmp = this->that;
		if( id == LEFT )
			tmp = this->left;
		else if( id == RIGHT )
			tmp = this->right;

		if( tmp != NULL ){
			if( tmp->getPrev() != NULL ){
				tmp = tmp->getPrev();
				if( id == LEFT )
					this->left = tmp;
				else if( id == RIGHT )
					this->right = tmp;
				else
					this->that = tmp;
				return true;
			}
		}
		return false;
	}

	bool gotoHead()
	{
		if( this->that != NULL ){
			this->that = this->head;
			return true;
		}
		return false;
	}
	/*
	Rewind 'left' cursor back to head
	*/
	bool lefttoHead()
	{
		if( this->left != NULL ){
			this->left = this->head;
			return true;
		}
		return false;
	}
	/*
	Rewind 'right' cursor back to head
	*/
	bool righttoHead()
	{
		if( this->right != NULL ){
			this->right = this->head;
			return true;
		}
		return false;
	}
	bool gotoTail()
	{
		if( this->that != NULL ){
			this->that = this->tail;
			return true;
		}
		return false;
	}
	/*
	Forward 'left' cursor to tail
	*/
	bool lefttoTail()
	{
		if( this->left != NULL ){
			this->left = this->tail;
			return true;
		}
		return false;
	}
	/*
	Forward 'right' cursor to tail
	*/
	bool righttoTail()
	{
		if( this->right != NULL ){
			this->right = this->tail;
			return true;
		}
		return false;
	}

	void gotoThat(T *object)
	{
		Node *tmp = this->head;

		while( tmp != NULL ){
			if( tmp->getObj() == object ){
				this->that = tmp;
				break;
			}
			tmp = tmp->getNext();
		}
	}

	T *getThat()
	{
		if( this->that != NULL )
			return this->that->getObj();
		return NULL;
	}
	T *getHead()
	{
		if( this->head != NULL )
			return this->head->getObj();
		return NULL;
	}
	T *getTail()
	{
		if( this->tail != NULL )
			return this->tail->getObj();
		return NULL;
	}
	T *getLeft()
	{
		if( this->left != NULL )
			return this->left->getObj();
		return NULL;
	}
	T *getRight()
	{
		if( this->right != NULL )
			return this->right->getObj();
		return NULL;
	}
	
	T *getLeftoThat(int count)
	{
		Node *tmp = this->that;

		while( count > 0 && tmp != NULL ){
			tmp = tmp->getPrev();
			count--;
		}
		if( tmp != NULL )
			return tmp->getObj();
		return NULL;
	}
	T *getRightoThat(int count)
	{
		Node *tmp = this->that;

		while( count > 0 && tmp != NULL ){
			tmp = tmp->getNext();
			count--;
		}
		if( tmp != NULL )
			return tmp->getObj();
		return NULL;
	}
	T *gettoThat(int count)
	{
		if( count < 0 )
			return this->getLeftoThat(abs(count));
		if( count > 0 )
			return this->getRightoThat(abs(count));
		return this->getThat();
	}

	Node *getThatNode()
	{
		return this->that;
	}
	Node *getThatHead()
	{
		return this->head;
	}
	Node *getThatTail()
	{
		return this->tail;
	}

	bool isThatHead()
	{
		if( this->that == this->head )
			return true;
		return false;
	}
	bool isThatTail()
	{
		if( this->that == this->tail )
			return true;
		return false;
	}

	int getCount()
	{
		return this->count;
	}
	/*
	Count elements before 'that'
	*/
	int countLeftoThat()
	{
		Node *tmp = this->that;
		int count = 0;

		if( tmp == NULL )
			return count;

		while( tmp->getPrev() != NULL ){
			tmp = tmp->getPrev();
			count++;
		}
		
		return count;
	}
	/*
	Count elements after 'that'
	*/
	int countRightoThat()
	{
		Node *tmp = this->that;
		int count = 0;

		if( tmp == NULL )
			return count;

		while( tmp->getNext() != NULL ){
			tmp = tmp->getNext();
			count++;
		}
		return count;
	}
	/*
	Get index of object
	*/
	int getIndex(T *object)
	{
		Node *tmp = this->head;
		int count = 0;

		if( tmp == NULL )
			return count;

		while( tmp->getNext() != NULL && tmp->getObj() != object ){
			tmp = tmp->getNext();
			count++;
		}
		return count;
	}
	/*
	Set 'that' to object of index idx
	*/
	T *setIndex(int idx)
	{
		Node *tmp = this->head;
		int count = 0;

		if( tmp == NULL )
			return NULL;

		idx = max( min(idx,this->count), 0 );

		while( tmp->getNext() != NULL && count != idx ){
			tmp = tmp->getNext();
			count++;
		}
		return tmp->getObj();
	}

	void setThat(Node *object)
	{
		this->that = object;
		this->left = this->that;
		this->right = this->that;
	}

	void setThat(T *object)
	{
		Node *tmp = this->head;

		while( tmp != NULL ){
			if( tmp->getObj() == object ){
				this->setThat(tmp);
				break;
			}
			tmp = tmp->getNext();
		}
	}
	/*
	Swap T objects of 'that'
	*/
	T *swapThat(T *object)
	{
		if( this->that != NULL ){
			T *old = this->that->getObj();

			this->that->setObj(object);
			return old;
		}
		return NULL;
	}

	void moveThatLeft()
	{
		if( this->that->getPrev() != NULL ){
			Node *prev = this->that->getPrev();
			Node *pprev = prev->getPrev();

			Node *next = this->that->getNext();

			this->that->setPrev(pprev);
			this->that->setNext(prev);

			prev->setPrev(this->that);
			prev->setNext(next);

			if( pprev != NULL )
				pprev->setNext(this->that);
			if( next != NULL )
				next->setPrev(prev);

			if( this->that == this->tail )
				this->tail = prev;
			if( prev == this->head )
				this->head = this->that;
		}
	}
	void moveThatRight()
	{
		if( this->that->getNext() != NULL ){
			Node *next = this->that->getNext();
			Node *nnext = next->getNext();

			Node *prev = this->that->getPrev();

			this->that->setNext(nnext);
			this->that->setPrev(next);

			next->setNext(this->that);
			next->setPrev(prev);

			if( nnext != NULL )
				nnext->setPrev(this->that);
			if( prev != NULL )
				prev->setNext(next);

			if( this->that == this->head )
				this->head = next;
			if( next == this->tail )
				this->tail = this->that;
		}
	}

	T *toArray()
	{
		if( this->getCount() == 0 )
			return NULL;

		T *tab = new T[this->getCount()];
		int i = 0;

		this->gotoHead();
		do {
			tab[i++] = *this->getThat();
		} while( this->next() == true );

		return tab;
	}

	T *toArray(int &last)
	{
		if( this->getCount() == 0 )
			return NULL;

		last = min(last,this->getCount());

		T *tab = new T[last];
		int i = last - 1;

		this->gotoTail();
		do {
			tab[i--] = *this->getThat();
		} while( this->prev() == true && i >= 0);

		return tab;
	}
};