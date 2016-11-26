#ifndef PER_SET_H
#define PER_SET_H
#include <string>
#include <iostream>
#include <memory>
#include <assert.h>
#include <memory.h>

typedef int value_type;

struct persistent_set
{

	struct node {
		value_type key;
		node * child[2];

		time_t ptr_cnt;
		time_t sz;

		node(value_type);
		node(node *);
	};

	struct iter {
		node * root;
		time_t curid;
		node * curnode;
		bool invalid;	

		iter(node *, time_t, node *);
	};

	struct iterator {
		value_type const& operator*();
		
		iterator& operator++();
		iterator operator++(int);
		
		iterator& operator--();
		iterator operator--(int);

		iterator(std::shared_ptr<iter>);

		friend class persistent_set;


		friend bool operator==(persistent_set::iterator, persistent_set::iterator);
		friend bool operator!=(persistent_set::iterator, persistent_set::iterator);
		
		private:
			std::shared_ptr<iter> x;

	};
	

	struct iterator_list {
		iterator_list * next;
		std::shared_ptr<iter> cur;

		iterator_list();

	};

	persistent_set();
    
	persistent_set(persistent_set const&);

	persistent_set& operator=(persistent_set const& rhs);

	~persistent_set();

	iterator find(value_type);

	std::pair<iterator, bool> insert(value_type);

	void erase(iterator);

	iterator begin();
	iterator end();

	
	private:
		node * root;
		iterator_list * iters;
		time_t cursz;

		static node * getN(node *, time_t);
		static time_t countN(node *, value_type);
		static node * findN(node *, value_type);
		static node * insertN(node *, value_type);
		static node * eraseN(node *, value_type);

		static void washN(node *);

		void make_invalid();


};




#endif