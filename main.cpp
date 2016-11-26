#include"persistent_set.h"
#include<iostream>
#include<assert.h>
#include<set>
#include<cassert>


std::set<int> A;
persistent_set B;

void check() {
	persistent_set::iterator itB = B.begin();

	for (std::set<int>::iterator itA = A.begin(); itA != A.end(); itA++, itB++) {
		assert(itB != B.end());
		assert((*itA) == (*itB));
	}
}


int main() {

	int x = 1;
	for (int i = 0; i < 100; i++) {
		x = x * 107 % 239;

		bool haveA = (A.find(x) == A.end());
		bool haveB = (B.find(x) == B.end());

		assert(haveA == haveB);
		

		A.insert(x);
		B.insert(x);
	}

	check();

	x = 1;
	for (int i = 0; i < 100; i++) {
		x = x * 101 % 239;
		bool haveA = (A.find(x) == A.end());
		bool haveB = (B.find(x) == B.end());

		assert(haveA == haveB);

		if (!haveA) continue;

		A.erase(A.find(x));
		B.erase(B.find(x));
	}
	
	check();

	std::cout << "OK\n";

	return 0;
}