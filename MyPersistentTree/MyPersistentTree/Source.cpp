#include "PersistentTree.h"
#include <iostream>
#include <iterator>
#include <algorithm>
#include <cassert>


using namespace Persistent;
using namespace std;



struct verify;
class tester {
	friend verify;
	static int livecount;
	const tester* self;
	int d;
public:
	tester(int i) : self(this), d(i) { ++livecount; }
	tester(const tester&) :self(this) { ++livecount; }
	~tester() { assert(self == this);--livecount; }
	tester& operator=(const tester& b) {
		assert(self == this && b.self == &b);
		return *this;
	}
	bool operator <(const tester& rhs) { return this->d < rhs.d; }
	bool operator == (const tester& rhs) { return this->d == rhs.d; }
	bool operator !=(const tester& rhs) { return !(*this == rhs); }
	void cfunction() const { assert(self == this); }
	void mfunction() { assert(self == this); }
};

int tester::livecount = 0;

struct verify {
	~verify() { assert(tester::livecount == 0); }
}verifier;

int main()
{
	// basic testing
	Tree<int> t;
	t.insert(1);
	t.insert(2);
	t.insert(3);
	t.insert(4);
	t.insert(5);
	t.insert(6);
	auto it = t.find(5);
	cout << *it << endl;
	t.remove(5);
	it = t.find(5);
	if (it != t.end())
		cout << *it << endl;
	else
		cout << "5 not found" << endl;
	it = t.begin();
	cout << *it << endl;
	++it;
	cout << *it << endl;
	it = t.begin();
	advance(it, 1);
	cout << *(it) << endl;

	// verifier approach
	Tree<tester> t2;
	t2.insert(tester(1));
	t2.insert(tester(2));
	t2.clear();
	system("pause");
}