#include"persistent_set.h"

//#define DEBUG

persistent_set::persistent_set() {
	root = NULL;
	iters = NULL;
	cursz = 0;
}

persistent_set::persistent_set(const persistent_set & o) {
	root = o.root;
	if (root != NULL) (root -> ptr_cnt)++;
	iters = NULL;
	cursz = o.cursz;
}

persistent_set& persistent_set::operator=(persistent_set const &o) {
	root = o.root;
	if (root != NULL) (root -> ptr_cnt)++;
	iters = NULL;
	cursz = o.cursz;
	return *this;
}


void persistent_set::washN(node * x) {
	if (x == NULL) return;
	if ((x -> ptr_cnt) > 0) return;

	for (int i = 0; i < 2; i++) if ((x -> child)[i] != NULL) {
		((x -> child)[i] -> ptr_cnt)--;
		washN((x -> child)[i]);
	}
	#ifdef DEBUG 
		std::cerr << "delete node\n";
	#endif
	delete x;
}

persistent_set::node * persistent_set::getN(node * x, size_t id) {
	if (x == NULL) return NULL;
	size_t lsz = 0;
	if ((x -> child)[0] != NULL) lsz = (x -> child)[0] -> sz;

	if (lsz > id) return getN((x -> child)[0], id);
	id -= lsz;

	if (id == 0) return x;

	return getN((x -> child)[1], id - 1);
}


persistent_set::node * persistent_set::findN(node * x, value_type key) {
	if (x == NULL) return NULL;
	if ((x -> key) == key) return x;
	return findN((x -> child)[key > (x -> key)], key); 
}

size_t persistent_set::countN(node * x, value_type key) {
	if (x == NULL) return 0;
	if ((x -> key) == key) return 0;

	int go = key > (x -> key);
	size_t ret = countN((x -> child)[go], key);
	if (go == 1 && (x -> child)[0] != NULL) ret += (x -> child)[0] -> sz;
	return ret;
}


//inv: doesn't contain key yet
persistent_set::node * persistent_set::insertN(node * x, value_type key) {
	if (x == NULL) {
		#ifdef DEBUG 
			std::cerr << "create node\n";
		#endif
		return new node(key);
	}		


	node * ret = new (std::nothrow) node(*x);
	if (ret == NULL) return x;
	#ifdef DEBUG 
		std::cerr << "create node\n";
	#endif
	
	int go = key > (x -> key);
	node * new_child = insertN((x -> child)[go], key);

	if (new_child == (x -> child)[go]) {
		delete ret;
		return x;
	}

	(ret -> child)[go] = new_child;
	for (int i = 0; i < 2; i++) if ((ret -> child)[i] != NULL) {
		((ret -> child)[i] -> ptr_cnt)++;
		(ret -> sz) += ((ret -> child)[i] -> sz);
	}		
	return ret;
}                                                                                               


//inv: x contains key
persistent_set::node * persistent_set::eraseN(node * x, value_type key) {
	value_type hkey = (x -> key);
	if (hkey == key) {
		int go = -1;
		for (int i = 0; i < 2; i++) if ((x -> child)[i] != NULL) go = i;

		if (go == -1) return NULL;

		node * t = (x -> child)[go];
		while ((t -> child)[1 - go] != NULL) t = (t -> child)[1 - go];

		int newkey = (t -> key);
		
		node * ret = new (std::nothrow) node(*x);
		if (ret == NULL) return x;
		#ifdef DEBUG 
			std::cerr << "create node\n";
		#endif
		
		(ret -> key) = newkey;
		
		node * new_child = eraseN((x -> child)[go], newkey);

		if (new_child == (x -> child)[go]) {
			#ifdef DEBUG 
				std::cerr << "delete node\n";
			#endif
			delete ret;
			return x;
		}

		(ret -> child)[go] = new_child;

		for (int i = 0; i < 2; i++) if ((ret -> child)[i] != NULL) {
			((ret -> child)[i] -> ptr_cnt)++;
			(ret -> sz) += ((ret -> child)[i] -> sz);
		}		
		return ret;
	}	
	node * ret = new (std::nothrow) node(*x);
	if (ret == NULL) return x;
	#ifdef DEBUG 
		std::cerr << "create node\n";
	#endif
	

	int go = key > (x -> key);
	node * new_child = eraseN((x -> child)[go], key);

	if (new_child == (x -> child)[go]) {
		#ifdef DEBUG 
			std::cerr << "delete node\n";
		#endif
		delete ret;
		return x;
	}

	(ret -> child)[go] = new_child;
	for (int i = 0; i < 2; i++) if ((ret -> child)[i] != NULL) {
		((ret -> child)[i] -> ptr_cnt)++;
		(ret -> sz) += ((ret -> child)[i] -> sz);
	}		
	return ret;
}




persistent_set::node::node(value_type key):key(key) {
	child[0] = child[1] = NULL;
	sz = 1;
	ptr_cnt = 0;
}

persistent_set::node::node(const node & o) {
	key = o.key;
	child[0] = o.child[0];
	child[1] = o.child[1];
	sz = 1;
	ptr_cnt = 0;
}

persistent_set::iter::iter(node * root, size_t id, node * cur):root(root), curid(id),curnode(cur) {
	invalid = false;	
}

persistent_set::iterator::iterator(std::shared_ptr<iter> x):x(x) {}

persistent_set::iterator_list::iterator_list(){}


void persistent_set::make_invalid() {
	while (iters != NULL) {
		iterator_list * temp = (iters -> next);

		((iters -> cur) -> invalid) = true;
		#ifdef DEBUG 
			std::cerr << "delete iter\n";
		#endif
		delete(iters);
		iters = temp;
	}
}

bool operator==(persistent_set::iterator a, persistent_set::iterator b) {
	assert(a.x != nullptr);
	assert(!(a.x -> invalid));
	assert(!(b.x -> invalid));

	return (a.x -> curnode) == (b.x -> curnode);
}

bool operator!=(persistent_set::iterator a, persistent_set::iterator b) {
	return !(a == b);
}

persistent_set::iterator persistent_set::end() {
	iterator_list * newtop = new (std::nothrow) iterator_list();
	assert(newtop != NULL);
	#ifdef DEBUG 
		std::cerr << "create iter\n";
	#endif
	(newtop -> next) = iters;
	(newtop -> cur) = std::make_shared<iter>(root, cursz, nullptr);
	iters = newtop;
	return iterator(iters -> cur);
	
}

persistent_set::iterator persistent_set::begin() {
	iterator_list * newtop = new (std::nothrow) iterator_list();
	assert(newtop != NULL);
	#ifdef DEBUG 
		std::cerr << "create iter\n";
	#endif
	(newtop -> next) = iters;
	(newtop -> cur) = std::make_shared<iter>(root, 0, getN(root, 0));
	iters = newtop;
	return iterator(iters -> cur);
}

persistent_set::iterator persistent_set::find(value_type key) {
	node * x = findN(root, key);
	size_t id = (x == NULL) ? cursz : countN(root, key);

	iterator_list * newtop = new (std::nothrow) iterator_list();
	assert(newtop != NULL);
	#ifdef DEBUG 
		std::cerr << "create iter\n";
	#endif
	
	(newtop -> next) = iters;
	(newtop -> cur) = std::make_shared<iter>(root, id, x);
	iters = newtop;
	return iterator(iters -> cur);
}

std::pair<persistent_set::iterator, bool> persistent_set::insert(value_type key) {
	node * x = findN(root, key);
	
	bool changed = false;

	if (x == NULL) {
		changed = true;
		make_invalid();

		node * newroot = insertN(root, key);
		if (root != NULL) {
			(root -> ptr_cnt)--;
			washN(root);
		}
		root = newroot;
		if (root != NULL) (root -> ptr_cnt)++;
		cursz++;
	}
	//could not insert
	assert(findN(root, key) != NULL);

	return std::make_pair(find(key), changed);
}


void persistent_set::erase(iterator it) {
	assert(!(it.x -> invalid));
	assert((it.x -> curnode) != NULL);

	make_invalid();

	node * newroot = eraseN(root, (it.x -> curnode) -> key);

	if (root != NULL) {
		(root -> ptr_cnt)--;
		washN(root);
	}

	root = newroot;
	if (root != NULL) (root -> ptr_cnt)++;
	cursz--;

	//could not erase
	assert(findN(root, (it.x -> curnode) -> key) == NULL);
}

value_type const& persistent_set::iterator::operator*() {
	assert(x != nullptr);
	assert(!(x -> invalid));
	assert((x -> curnode) != NULL);
	return (x -> curnode) -> key;
}


persistent_set::iterator& persistent_set::iterator::operator++() {
	assert(x != nullptr);
	assert(!(x -> invalid));
	assert((x -> curnode) != NULL);
	(x -> curid)++;
	(x -> curnode) = getN(x -> root, (x -> curid));
	return *this;
}

persistent_set::iterator persistent_set::iterator::operator++(int) {
    iterator tmp = *this;
    ++(*this);
    return tmp;
}



persistent_set::iterator& persistent_set::iterator::operator--() {
	assert(x != nullptr);
	assert(!(x -> invalid));
	assert((x -> curid) > 0);
	(x -> curid)--;
	(x -> curnode) = getN(x -> root, (x -> curid));
	return *this;
}

persistent_set::iterator persistent_set::iterator::operator--(int) {
    iterator tmp = *this;
    --(*this);
    return tmp;
}




persistent_set::~persistent_set() {
	make_invalid();

	if (root != NULL) {
		(root -> ptr_cnt)--;
		washN(root);
	}
}







